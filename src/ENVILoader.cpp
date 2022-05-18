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

using namespace hdps;

ENVILoader::ENVILoader(CoreInterface* core, QString datasetName):
	_core(core),
	_datasetName(datasetName)
{
}

bool ENVILoader::loadFromFile(std::string file, float ratio, int filter, bool flip)
{
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

		int dataType = std::stoi(parameters["data type"]);

		ENVI::Interleave interleave;
		if (parameters["interleave"] == "bsq")
		{
			interleave = ENVI::bsq;
		}
		else if (parameters["interleave"] == "bil")
		{
			interleave = ENVI::bil;
		}
		else if (parameters["interleave"] == "bip")
		{
			interleave = ENVI::bip;
		}

		// check limited compatibility for now
		if (dataType != 4 && dataType != 12)
		{
			throw std::runtime_error("Unable to load. Data type not supported. Only 4 byte supported at this time.");
		}
		if (std::stoi(parameters["byte order"]) != 0)
		{
			throw std::runtime_error("Unable to load. Byte order not supported.");
		}
		if (parameters["file type"] != "ENVI Standard")
		{
			throw std::runtime_error("Unable to load. File type is not ENVI Standard.");
		}

		size_t numVars = std::stoi(parameters["bands"]);

		parameters["band names"] = trimString(parameters["band names"], { '{', '}', ' ', '\t', '\n' });
		parameters["wavelength"] = trimString(parameters["wavelength"], { '{', '}', ' ', '\t', '\n' });

		std::vector<QString> bandNames = tokenizeString(parameters["band names"], ',', true);
		std::vector<QString> wavelengths = tokenizeString(parameters["wavelength"], ',', true);

		if (bandNames.size() == 0)
		{
			bandNames = wavelengths;
		}

		assert(bandNames.size() == numVars);
		assert(wavelengths.size() == numVars);

		size_t imgWidth = std::stoi(parameters["samples"]);
		size_t imgHeight = std::stoi(parameters["lines"]);

		size_t numItems = imgWidth * imgHeight * numVars;

		std::vector<float> data(numItems);

		// check for raw file, for now, we'll test for no extension, .cube, .img, and .raw
		std::fstream rawFile;
		std::vector< std::string> possibleExtensions = {"", ".cube", ".img", ".raw"};
		std::string baseFileName = file;
		baseFileName = baseFileName.erase(baseFileName.size() - 4, 4);
		for (auto extension : possibleExtensions)
		{
			rawFile.open(baseFileName + extension, std::ios::binary | std::ios::in);
			if (rawFile.good())
			{
				std::cout << "Loading " << baseFileName + extension << std::endl;
				break;
			}
		}

		if (!rawFile.is_open())
		{
			throw std::runtime_error("Unable to open raw file ");
		}

		size_t offset = std::stoi(parameters["header offset"]);

		// get its size:
		rawFile.seekg(0, std::ios::end);
		size_t fileSize = (size_t)(rawFile.tellg()) - offset;
		rawFile.seekg(offset, std::ios::beg);

		switch (dataType) {
		case 4:
			if (fileSize < imgWidth * imgHeight * numVars * sizeof(float))
			{
				throw std::runtime_error("Unable to read raw data. Fileszie does not match expected size from header.");
			}
			read<float>(rawFile, interleave, imgWidth, imgHeight, numVars, flip, data);
			break;
		case 12:
			if (fileSize < imgWidth * imgHeight * numVars * sizeof(uint16_t))
			{
				throw std::runtime_error("Unable to read raw data. Fileszie does not match expected size from header.");
			}
			read<uint16_t>(rawFile, interleave, imgWidth, imgHeight, numVars, flip, data);
			break;
		}

		size_t targetWidth = (size_t)round(imgWidth * ratio);
		size_t targetHeight = (size_t)round(imgHeight * ratio);
		std::vector<float> subsampledData(targetWidth * targetHeight * numVars);

		if (filter != -1) {
			nearestNeighbourFiltering(ratio, imgWidth, targetWidth, imgHeight, targetHeight, numVars, data, subsampledData);
		}

		auto points = _core->addDataset<Points>("Points", _datasetName);
		//hdps::util::DatasetRef<Points> points(_core->addData("Points", _datasetName));
		_core->notifyDatasetAdded(points);

		// no downsampling
		if (filter == -1) {
			points->setData(std::move(data), numVars);
		}
		// subsample data
		else {
			points->setData(std::move(subsampledData), numVars);
		}

		points->setDimensionNames(wavelengths);

		_core->notifyDatasetChanged(points);

		auto images = _core->addDataset<Images>("Images", "images", Dataset<DatasetImpl>(*points));
		//hdps::util::DatasetRef<Images> images(_core->addData("Images", "images", points->getName()));
		_core->notifyDatasetAdded(images);

		images->setGuiName("Images");
		images->setType(ImageData::Type::Stack);
		images->setNumberOfImages(1);
		images->setImageSize(QSize(targetWidth, targetHeight));
		images->setNumberOfComponentsPerPixel(numVars);
		images->setImageFilePaths(QStringList(QString::fromStdString(file)));

		_core->notifyDatasetChanged(images);

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

// currently only nearest neighbour downsampling is supported
bool ENVILoader::nearestNeighbourFiltering(float ratio, size_t inWidth, size_t outWidth, size_t inHeight, size_t outHeight, size_t numVars, std::vector<float>& inData, std::vector<float>& outData)
{
	outData.resize(outWidth * outHeight * numVars);

#pragma omp parallel for
	for (size_t y = 0; y < outHeight; y++)
	{
		size_t oLine = outWidth * numVars * y;
		size_t scaledY = std::min(inHeight -1, (size_t)round(y / ratio));
		size_t iLine = inWidth * numVars * scaledY;

		for (size_t x = 0; x < outWidth; x++)
		{
			size_t oPix = oLine + numVars * x;
			size_t scaledX = std::min(inWidth - 1, (size_t)round(x / ratio));
			size_t iPix = iLine + numVars * scaledX;

			std::copy(inData.begin() + iPix, inData.begin() + iPix + numVars, outData.begin() + oPix);
		}
	}
	return true;
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