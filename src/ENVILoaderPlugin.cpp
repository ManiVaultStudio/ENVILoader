#pragma once

#include "ENVILoaderPlugin.h"

#include "ENVILoader.h"
#include "Set.h"
#include "PointData.h"
#include "Application.h"
#include "SubsampleSettingsWidget.h"

#include <QtCore>
#include <QDebug>
#include <iostream>

using namespace hdps;
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

    SubsampleSettingsWidget subsampleDialog;
    subsampleDialog.initialize(this, fileName);
    subsampleDialog.exec();
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
