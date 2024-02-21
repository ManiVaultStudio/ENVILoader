#ifndef ENVI_LOADER
#define ENVI_LOADER

#include <fstream>
#include <string>
#include <vector>

#include <QString>

#include "CoreInterface.h"

namespace ENVI {
    class ENVILoader;
	enum Interleave { bsq, bip, bil };

	struct Header {
		Interleave interleave;

		size_t imageWidth;
		size_t imageHeight;
		size_t imageBands;

		std::vector<QString> bandNames;
		std::vector<QString> wavelengths;

		size_t rawOffset;

		int dataType;
		int typeSize;
		std::string rawFileName;
	};
}

using namespace mv;

class ENVILoader
{
private:
	std::string _textHeader;
	QString _datasetName;

	CoreInterface* _core;

public:
	ENVILoader(CoreInterface* core, QString datasetName);
	~ENVILoader();

	bool loadHeaderFromFile(std::string file);
	bool loadRaw(float ratio, int filter, bool flip = true);

	std::pair<size_t, size_t> getExtents();

	std::string textHeader();

private:

	template<typename InputDataType>
	bool read(InputDataType* rawData, bool flip, std::vector<float>& output)
	{
		const size_t imgSize = _header.imageWidth * _header.imageHeight;
		const size_t size = imgSize * _header.imageBands;
		const size_t lineSize = _header.imageWidth * _header.imageBands;

		output.resize(size);

#pragma omp parallel for
		for (int y = 0; y < _header.imageHeight; y++)
		{
			const size_t yOffsetOut = lineSize * y;
			const size_t inY = flip ? (_header.imageHeight - y - 1) : y;
			for (size_t x = 0; x < _header.imageWidth; x++)
			{
				const size_t offsetOut = yOffsetOut + _header.imageBands * x;
				for (size_t v = 0; v < _header.imageBands; v++)
				{
					if (_header.interleave == ENVI::bsq)
					{
						output[offsetOut + v] = static_cast<float>(rawData[imgSize * v + _header.imageWidth * inY + x]);
					}
					else if (_header.interleave == ENVI::bip)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSize * inY + _header.imageBands * x + v]);
					}
					else if (_header.interleave == ENVI::bil)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSize * inY + _header.imageWidth * v + x]);
					}
				}
			}
		}
		return true;
	};

	template<typename InputDataType>
	bool readScaled(InputDataType* rawData, size_t outWidth, size_t outHeight, bool flip, float scale, std::vector<float>& output)
	{
		const size_t imgSizeIn = _header.imageWidth * _header.imageHeight;
		const size_t lineSizeIn = _header.imageWidth * _header.imageBands;

		output.resize(outWidth * outHeight * _header.imageBands);

		const size_t maxX = _header.imageWidth - 1;
		const size_t maxY = _header.imageHeight - 1;

#pragma omp parallel for
		for (int y = 0; y < outHeight; y++)
		{
			// in
			size_t scaledY = (size_t)round(y * scale);
			scaledY = scaledY > maxY ? maxY : scaledY;
			scaledY = flip ? maxY - scaledY: scaledY;
			// out
			const size_t yOffsetOut = outWidth * _header.imageBands * y;
			for (size_t x = 0; x < outWidth; x++)
			{
				// in
				size_t scaledX = (size_t)round(x * scale);
				scaledX = scaledX > maxX ? maxX : scaledX;
				// out
				const size_t offsetOut = yOffsetOut + _header.imageBands * x;
				for (size_t v = 0; v < _header.imageBands; v++)
				{
					if (_header.interleave == ENVI::bsq)
					{
						output[offsetOut + v] = static_cast<float>(rawData[imgSizeIn * v + _header.imageWidth * scaledY + scaledX]);
					}
					else if (_header.interleave == ENVI::bip)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSizeIn * scaledY + _header.imageBands * scaledX + v]);
					}
					else if (_header.interleave == ENVI::bil)
					{
						output[offsetOut + v] = static_cast<float>(rawData[lineSizeIn * scaledY + _header.imageWidth * v + scaledX]);
					}
				}
			}
		}
		return true;
	};

	std::string trimString(std::string input, std::vector<char> delimiters);
	std::vector<QString> tokenizeString(std::string str, char delim, bool stripWhitespace);

	ENVI::Header _header;
	bool _headerLoaded;
};

#endif // ENVI_LOADER