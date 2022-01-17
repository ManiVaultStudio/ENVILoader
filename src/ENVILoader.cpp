#include "ENVILoader.h"

#include "CoreInterface.h"
#include "PointData.h"
#include "ImageData/Images.h"

#include <iostream>
#include <fstream>
#include <map>
#include <assert.h>

#include <QMessageBox>
#include <QString>

using namespace hdps;

ENVILoader::ENVILoader(CoreInterface* core, QString datasetName):
	_core(core),
	_datasetName(datasetName)
{
}

bool ENVILoader::loadFromFile(std::string file)
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

				if (objectOpenerIdx < 0)
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

		// check limited compatibility for now
		if (std::stoi(parameters["data type"]) != 4)
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

		assert(bandNames.size() == numVars);
		assert(wavelengths.size() == numVars);

		size_t imgWidth = std::stoi(parameters["samples"]);
		size_t imgHeight = std::stoi(parameters["lines"]);

		std::vector<float> data(imgWidth * imgHeight * numVars);

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

		int offset = std::stoi(parameters["header offset"]);

		// get its size:
		rawFile.seekg(offset, std::ios::end);
		size_t fileSize = rawFile.tellg();
		rawFile.seekg(offset, std::ios::beg);

		if(fileSize != imgWidth * imgHeight * numVars * 4)
		{
			throw std::runtime_error("Unable to read raw data. Fileszie does not match expected size from header.");
		}

		if (parameters["interleave"] == "bsq")
		{
			float* bsqData = new float[imgWidth * imgHeight * numVars];
			rawFile.read((char*)&bsqData[0], fileSize);

	#pragma omp parallel for
			for (int v = 0; v < numVars; v++)
				for (int y = 0; y < imgHeight; y++)
					for (int x = 0; x < imgWidth; x++)
					{
						data[imgWidth * numVars * (imgHeight - y - 1) + numVars * x + v] = bsqData[imgWidth * imgHeight * v + imgWidth * y + x];
					}

			delete[] bsqData;
		}
		else if (parameters["interleave"] == "bip")
		{
			rawFile.read(reinterpret_cast<char*>(data.data()), data.size() * sizeof(float));
		}
		else if (parameters["interleave"] == "bil")
		{
			float* bilData = new float[imgWidth * imgHeight * numVars];
			rawFile.read((char*)&bilData[0], fileSize);

	#pragma omp parallel for
			for (int y = 0; y < imgHeight; y++)
				for (int v = 0; v < numVars; v++)
					for (int x = 0; x < imgWidth; x++)
					{
						data[imgWidth * numVars * y + numVars * x + v] = bilData[imgWidth * numVars * y + imgWidth * v + x];
					}


			delete[] bilData;
		}
		else {
			throw std::runtime_error("Unable to read raw data.");
		}

		auto points = _core->addDataset<Points>("Points", _datasetName);
		//hdps::util::DatasetRef<Points> points(_core->addData("Points", _datasetName));
		_core->notifyDataAdded(points);

		points->setData(std::move(data), numVars);
		points->setDimensionNames(wavelengths);

		_core->notifyDataChanged(points);

		auto images = _core->addDataset<Images>("Images", "images", Dataset<DatasetImpl>(*points));
		//hdps::util::DatasetRef<Images> images(_core->addData("Images", "images", points->getName()));
		_core->notifyDataAdded(images);

		images->setGuiName("Images");
		images->setType(ImageData::Type::Stack);
		images->setNumberOfImages(1);
		images->setImageSize(QSize(imgWidth, imgHeight));
		images->setNumberOfComponentsPerPixel(numVars);
		images->setImageFilePaths(QStringList(QString::fromStdString(file)));

		_core->notifyDataChanged(images);

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