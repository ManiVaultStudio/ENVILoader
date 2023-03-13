#include "ENVILoader.h"

#include "CoreInterface.h"
#include "PointData.h"
#include "ImageData/Images.h"

#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>
#include <algorithm>

#include <QMessageBox>
#include <QString>

#include "external/mio/single_include/mio/mio.hpp"

using namespace hdps;

ENVILoader::ENVILoader(CoreInterface* core, QString datasetName) :
	_core(core),
	_datasetName(datasetName),
	_headerLoaded(false)
{
}

bool ENVILoader::loadHeaderFromFile(std::string file)
{
	_header = ENVI::Header();
	_headerLoaded = false;
	try
	{
		std::fstream headerFile(file, std::ios::in);

		if (!headerFile.is_open())
		{
			throw std::runtime_error("File was not found at location.");
		}

		qDebug() << "Loading " << QString::fromStdString(file);

		std::string line;
		std::getline(headerFile, line);

		// not an ENVI file
		if (line != "ENVI") {
			throw std::runtime_error("Unable to load. Not an ENVI file.");
		}

		std::map<std::string, std::string> parameters;
		while (std::getline(headerFile, line)) {

			std::string key = "";
			std::string value = "";

			int separatorIdx = line.find("=");

			if (separatorIdx >= 0)
			{
				key = trimString(line.substr(0, separatorIdx), { ' ', '\t', '\n' });

				int objectOpenerIdx = line.find("{");
				int objectCloserIdx = line.find("}");

				if (objectOpenerIdx < 0 || objectCloserIdx >= 0)
				{
					value = trimString(line.substr(separatorIdx + 1, line.size() - 1), { ' ', '\t', '\n' });
				}
				else
				{
					value = line.substr(objectOpenerIdx, line.size() - 1);
					while (std::getline(headerFile, line))
					{
						value += line;

						int found = line.find("}");
						if (found >= 0)
						{
							break;
						}
					}
				}
			}

			parameters[key] = value;
		}

		headerFile.close();

		_header.dataType = std::stoi(parameters["data type"]);

		if (parameters["interleave"] == "bsq")
		{
			_header.interleave = ENVI::bsq;
		}
		else if (parameters["interleave"] == "bil")
		{
			_header.interleave = ENVI::bil;
		}
		else if (parameters["interleave"] == "bip")
		{
			_header.interleave = ENVI::bip;
		}

		// check limited compatibility for now
		//if (dataType != 4 && dataType != 12)
		//{
		//	throw std::runtime_error("Unable to load. Data type not supported. Only 4 byte supported at this time.");
		//}
		if (std::stoi(parameters["byte order"]) != 0)
		{
			throw std::runtime_error("Unable to load. Byte order not supported.");
		}
		if (parameters["file type"] != "ENVI Standard")
		{
			throw std::runtime_error("Unable to load. File type is not ENVI Standard.");
		}

		_header.imageBands = std::stoi(parameters["bands"]);

		parameters["band names"] = trimString(parameters["band names"], { '{', '}', ' ', '\t', '\n' });
		parameters["wavelength"] = trimString(parameters["wavelength"], { '{', '}', ' ', '\t', '\n' });

		_header.bandNames = tokenizeString(parameters["band names"], ',', true);
		_header.wavelengths = tokenizeString(parameters["wavelength"], ',', true);

		if (_header.bandNames.size() == 0)
		{
			_header.bandNames = _header.wavelengths;
		}

		assert(_header.bandNames.size() == _header.imageBands);
		assert(_header.wavelengths.size() == _header.imageBands);

		_header.imageWidth = std::stoi(parameters["samples"]);
		_header.imageHeight = std::stoi(parameters["lines"]);

		// check for raw file, for now, we'll test for no extension, .cube, .img, and .raw
		std::fstream rawFile;
		std::vector< std::string> possibleExtensions = { "", ".cube", ".img", ".raw" };
		std::string baseFileName = file;
		baseFileName = baseFileName.erase(baseFileName.size() - 4, 4);
		for (auto extension : possibleExtensions)
		{
			rawFile.open(baseFileName + extension, std::ios::binary | std::ios::in);
			if (rawFile.good())
			{
				_header.rawFileName = baseFileName + extension;
				break;
			}
		}

		if (!rawFile.is_open())
		{
			throw std::runtime_error("Unable to open raw file ");
		}

		_header.rawOffset = std::stoi(parameters["header offset"]);

		// get its size:
		rawFile.seekg(0, std::ios::end);
		size_t fileSize = (size_t)(rawFile.tellg()) - _header.rawOffset;
		rawFile.close();

		switch (_header.dataType) {
		case 1:
			_header.typeSize = sizeof(uint8_t);
			break;
		case 2:
			_header.typeSize = sizeof(int16_t);
			break;
		case 3:
			_header.typeSize = sizeof(int32_t);
			break;
		case 4:
			_header.typeSize = sizeof(float);
			break;
		case 5:
			_header.typeSize = sizeof(double);
			break;
		case 12:
			_header.typeSize = sizeof(uint16_t);
			break;
		case 13:
			_header.typeSize = sizeof(uint32_t);
			break;
		case 14:
			_header.typeSize = sizeof(int64_t);
			break;
		case 15:
			_header.typeSize = sizeof(uint64_t);
			break;
		default:
			throw std::runtime_error("Raw data cannot be loaded. Data type is not supported.");
		}

		const size_t readSize = _header.imageWidth * _header.imageHeight * _header.imageBands * _header.typeSize;
		if (fileSize < readSize)
		{
			throw std::runtime_error("Raw data cannot be loaded. Fileszie does not match expected size from header.");
		}
	}
	catch (const std::runtime_error& e)
	{
		QMessageBox::critical(nullptr, QString("Unable to load %1").arg(_datasetName), e.what());
	}
	catch (std::exception e)
	{
		QMessageBox::critical(nullptr, QString("Unable to load %1").arg(_datasetName), e.what());
	}

	_headerLoaded = true;
}

std::pair<size_t, size_t> ENVILoader::getExtents()
{
	std::pair<size_t, size_t> extents(0, 0);

	if (_headerLoaded)
	{
		extents.first = _header.imageWidth;
		extents.second = _header.imageHeight;
	}

	return extents;
}

bool ENVILoader::loadRaw(float ratio, int filter, bool flip)
{
	try
	{
		size_t targetWidth = _header.imageWidth;
		size_t targetHeight = _header.imageHeight;

		size_t numItems = _header.imageWidth * _header.imageHeight * _header.imageBands;
		size_t numBytes = numItems * _header.typeSize;

		mio::mmap_source mmapSource(_header.rawFileName, _header.rawOffset, numBytes);

		std::vector<float> data;

		if (filter < 0)
		{
			switch (_header.dataType) {
			case 1:
				read((uint8_t*)mmapSource.data(), flip, data);
				break;
			case 2:
				read((int16_t*)mmapSource.data(), flip, data);
				break;
			case 3:
				read((int32_t*)mmapSource.data(), flip, data);
				break;
			case 4:
				read((float*)mmapSource.data(), flip, data);
				break;
			case 5:
				read((double*)mmapSource.data(), flip, data);
				break;
			case 12:
				read((uint16_t*)mmapSource.data(), flip, data);
				break;
			case 13:
				read((uint32_t*)mmapSource.data(), flip, data);
				break;
			case 14:
				read((int64_t*)mmapSource.data(), flip, data);
				break;
			case 15:
				read((uint64_t*)mmapSource.data(), flip, data);
				break;
			}
		}
		else
		{
			targetWidth = static_cast<size_t>(std::round(_header.imageWidth * ratio));
			targetHeight = static_cast<size_t>(std::round(_header.imageHeight * ratio));

			switch (_header.dataType) {
			case 1:
				readScaled((uint8_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f/ratio, data);
				break;
			case 2:
				readScaled((int16_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 3:
				readScaled((int32_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 4:
				readScaled((float*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 5:
				readScaled((double*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 12:
				readScaled((uint16_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 13:
				readScaled((uint32_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 14:
				readScaled((int64_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			case 15:
				readScaled((uint64_t*)mmapSource.data(), targetWidth, targetHeight, flip, 1.0f / ratio, data);
				break;
			}
		}

		auto points = _core->addDataset<Points>("Points", _datasetName);
		events().notifyDatasetAdded(points);

		points->setData(std::move(data), _header.imageBands);
		points->setDimensionNames(_header.wavelengths);
		events().notifyDatasetChanged(points);

		auto images = _core->addDataset<Images>("Images", "images", Dataset<DatasetImpl>(*points));
		events().notifyDatasetAdded(images);

		images->setGuiName("Images");
		images->setType(ImageData::Type::Stack);
		images->setNumberOfImages(1);
		images->setImageSize(QSize(static_cast<int>(targetWidth), static_cast<int>(targetHeight)));
		images->setNumberOfComponentsPerPixel(static_cast<unsigned int>(_header.imageBands));
		images->setImageFilePaths(QStringList(QString::fromStdString(_header.rawFileName)));

		events().notifyDatasetChanged(images);

		return true;
	}
	catch (const std::runtime_error& e)
	{
		QMessageBox::critical(nullptr, QString("Unable to load %1").arg(_datasetName), e.what());
	}
	catch (std::exception e)
	{
		QMessageBox::critical(nullptr, QString("Unable to load %1").arg(_datasetName), e.what());
	}

	return false;
}

std::string ENVILoader::trimString(std::string input, std::vector<char> delimiters)
{
	while (input.length() > 0 && std::find(std::begin(delimiters), std::end(delimiters), input[0]) != std::end(delimiters))
	{
		input.erase(0, 1);
	}
	while (input.length() > 0 && std::find(std::begin(delimiters), std::end(delimiters), input[input.size() - 1]) != std::end(delimiters))
	{
		input.erase(input.size() - 1, 1);
	}

	return input;
}

std::vector<QString> ENVILoader::tokenizeString(std::string str, char delim, bool stripWhitespace)
{
	size_t start;
	size_t end = 0;

	std::vector<QString> out(0);

	while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = str.find(delim, start);
		std::string stripped = str.substr(start, end - start);
		if (stripWhitespace)
		{
			stripped = trimString(stripped, { ' ', '\t', '\n' });
		}
		out.push_back(QString::fromStdString(stripped));
	}

	return out;
}

std::string ENVILoader::textHeader()
{
	return _textHeader;
}

ENVILoader::~ENVILoader()
{

}