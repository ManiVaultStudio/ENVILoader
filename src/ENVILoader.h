#ifndef ENVI_LOADER
#define ENVI_LOADER

#include <string>
#include <vector>
#include <tuple>
#include <fstream>

#include <QString>

#include "CoreInterface.h"

namespace ENVI {
    class ENVILoader;
	enum Interleave { bsq, bip, bil };
}

using namespace hdps;

class ENVILoader
{
private:
	std::string _textHeader;
	QString _datasetName;

	CoreInterface* _core;

public:
	ENVILoader(CoreInterface* core, QString datasetName);
	~ENVILoader();

	bool loadFromFile(std::string file, float ratio, int filter, bool flip = true);

	template<typename InputDataType>
	bool read(std::fstream& file, ENVI::Interleave interleave, size_t width, size_t height, size_t bands, bool flip, std::vector<float>& output)
	{
		size_t size = width * height * bands;
		
		std::vector<InputDataType> rawData(size);
		file.read((char*)rawData.data(), size * sizeof(InputDataType));

		output.resize(size);

		if (interleave == ENVI::bsq)
		{
#pragma omp parallel for
			for (size_t v = 0; v < bands; v++)
			{
				size_t iLine = width * height * v;
				for (size_t y = 0; y < height; y++)
				{
					size_t iPix = iLine + width * y;
					size_t oPix = width * bands * (flip ? (height - y - 1) : y) + v;
					for (size_t x = 0; x < width; x++)
					{
						output[oPix + bands * x] = static_cast<float>(rawData[iPix + x]);
					}
				}
			}
		}
		else if (interleave == ENVI::bip)
		{
#pragma omp parallel for
			for (size_t y = 0; y < height; y++)
			{
				size_t iLine = width * bands * y;
				size_t oLine = width * bands * (flip ? (height - y - 1) : y);
				for (size_t x = 0; x < width; x++)
				{
					size_t iPix = iLine + bands * x;
					size_t oPix = oLine + bands * x;
					for (size_t v = 0; v < bands; v++)
					{
						output[oPix + v] = static_cast<float>(rawData[iPix + v]);
					}
				}
			}
		}
		else if (interleave == ENVI::bil)
		{
#pragma omp parallel for
			for (size_t y = 0; y < height; y++)
			{
				size_t iLine = width * bands * y;
				size_t oLine = width * bands * (flip ? (height - y - 1) : y);
				for (size_t v = 0; v < bands; v++)
				{
					size_t iPix = iLine + width * v;
					size_t oPix = oLine + v;
						for (size_t x = 0; x < width; x++)
						{
							output[oPix + bands * x] = static_cast<float>(rawData[iPix + x]);
						}
				}
			}
		}
		else {
			throw std::runtime_error("Unable to read raw data.");
		}

		return true;
	};

	std::string textHeader();

	bool nearestNeighbourFiltering(float ratio, size_t inWidth, size_t outWidth, size_t inHeight, size_t outHeight, size_t numVars, std::vector<float>& inData, std::vector<float>& outData);

private:
	std::string trimString(std::string input, std::vector<char> delimiters);
	std::vector<QString> tokenizeString(std::string str, char delim, bool stripWhitespace);
};

#endif // ENVI_LOADER