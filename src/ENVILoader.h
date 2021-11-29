#ifndef ENVI_LOADER
#define ENVI_LOADER

#include <string>
#include <vector>

#include <QString>

#include "CoreInterface.h"

namespace ENVI {
    class ENVILoader;
}

class ENVILoader
{
private:
	std::string _textHeader;
	QString _datasetName;

	hdps::CoreInterface* _core;

public:
	ENVILoader(hdps::CoreInterface* core, QString datasetName);
	~ENVILoader();

	bool loadFromFile(std::string file);

	std::string textHeader();

private:
	std::string trimString(std::string input, std::vector<char> delimiters);
	std::vector<QString> tokenizeString(std::string str, char delim, bool stripWhitespace);
};

#endif // ENVI_LOADER