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
		const size_t imgSize = width * height;
		const size_t size = imgSize * bands;
		const size_t lineSize = width * bands;
		
		std::vector<InputDataType> rawData(size);
		file.read((char*)rawData.data(), size * sizeof(InputDataType));
		output.resize(size);

#pragma omp parallel for
		for (int y = 0; y < height; y++)
		{
			const size_t yOffsetOut = lineSize * (flip ? (height - y - 1) : y);
			for (size_t x = 0; x < width; x++)
			{
				const size_t offsetOut = yOffsetOut + bands * x;
				for (size_t v = 0; v < bands; v++)
				{
					if (interleave == ENVI::bsq)
					{
						output[offsetOut + v] = static_cast<float>(rawData[imgSize * v + width * y + x]);
					}
					else if (interleave == ENVI::bip)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSize * y + bands * x + v]);
					}
					else if (interleave == ENVI::bil)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSize * y + width * v + x]);
					}
				}
			}
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