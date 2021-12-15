#pragma once

#include <LoaderPlugin.h>

#include <QStringList>

using namespace hdps::plugin;

/**
 * ENVI loader plugin class
 *
 * This loader plugin class provides functionality to load high-dimensional image data in the ENVI format into HDPS
 *
 * @author Thomas Höllt
 */
class ENVILoaderPlugin : public LoaderPlugin
{
public:
    /** Default constructor */
    ENVILoaderPlugin(const PluginFactory* factory);

public: // Inherited from LoaderPlugin

    /** Initializes the plugin */
    void init() override;

    /** Load high dimensional image data */
    void loadData() Q_DECL_OVERRIDE;
};

/**
 * Image loader plugin factory class
 * A factory for creating image loader plugin instances
 */
class ENVILoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(hdps::plugin::LoaderPluginFactory hdps::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.ENVILoader"
                      FILE  "ENVILoaderPlugin.json")

public:
    /** Default constructor */
    ENVILoaderPluginFactory(void) {}

    /** Destructor */
    ~ENVILoaderPluginFactory(void) override {}

	/** Returns the icon of this plugin */
	QIcon getIcon() const override;

    /** Creates an image loader plugin instance */
    LoaderPlugin* produce() override;

	/** Returns the supported data types */
	hdps::DataTypes supportedDataTypes() const override;
};
