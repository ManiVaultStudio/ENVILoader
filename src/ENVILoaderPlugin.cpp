#include "ENVILoaderPlugin.h"

#include "Application.h"
#include "PointData/PointData.h"

#include "SubsampleSettingsWidget.h"

#include <QDebug>
#include <QtCore>

using namespace mv;
using namespace std;

Q_PLUGIN_METADATA(IID "nl.tudelft.ENVILoaderPlugin")

ENVILoaderPlugin::ENVILoaderPlugin(const PluginFactory* factory) :
    LoaderPlugin(factory),
    _ENVILoaderModel(this)
{
}

void ENVILoaderPlugin::init()
{
}

CoreInterface* ENVILoaderPlugin::getCore() {
    return _core;
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


    SubsampleSettingsWidget subsampleDialog(*this, fileName);
    int res = subsampleDialog.exec();

    if (res == QDialog::Accepted) {
        int filter = -1;
        float ratioSub = 1;

        if (subsampleDialog.getEnableSubsamplingToogle()) {
            filter = 1;
            ratioSub = subsampleDialog.getRatio();
        }

        // load file with given subsampling settings
        const auto loaded = _ENVILoaderModel.load(ratioSub, filter, true);

    }

}

QIcon ENVILoaderPluginFactory::getIcon(const QColor& color /*= Qt::black*/) const
{
    return Application::getIconFont("FontAwesome").getIcon("images", color);
}

LoaderPlugin* ENVILoaderPluginFactory::produce()
{
    return new ENVILoaderPlugin(this);
}

DataTypes ENVILoaderPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
