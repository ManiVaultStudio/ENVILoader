#include "ENVILoaderPlugin.h"

#include "ENVILoader.h"

#include "Set.h"
#include "PointData.h"
#include "Application.h"

#include <QtCore>
#include <QDebug>

using namespace hdps;

Q_PLUGIN_METADATA(IID "nl.tudelft.ENVILoaderPlugin")

ENVILoaderPlugin::ENVILoaderPlugin(const PluginFactory* factory) :
    LoaderPlugin(factory)
{
}

void ENVILoaderPlugin::init()
{
}

void ENVILoaderPlugin::loadData()
{
    const auto fileName = AskForFileName(QObject::tr("ENVI Files (*.hdr)"));

    if (fileName.isEmpty())
    {
        return;
    }

    qDebug() << "Loading ENVI file: " << fileName;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw DataLoadException(fileName, "File was not found at location.");
    }

    file.close();

    ENVILoader* loader = new ENVILoader(_core, "test");
    loader->loadFromFile(fileName.toStdString());
}

QIcon ENVILoaderPluginFactory::getIcon() const
{
    return hdps::Application::getIconFont("FontAwesome").getIcon("images");
}

LoaderPlugin* ENVILoaderPluginFactory::produce()
{
    return new ENVILoaderPlugin(this);
}

hdps::DataTypes ENVILoaderPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
