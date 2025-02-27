#pragma once

#include "ENVILoaderModel.h"

#include <LoaderPlugin.h>

using namespace mv::plugin;

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

    mv::CoreInterface* getCore();

    /** Load high dimensional image data */
    void loadData() Q_DECL_OVERRIDE;

public:

    ENVILoaderModel &getENVILoaderModel() { return _ENVILoaderModel; }

private:

    ENVILoaderModel _ENVILoaderModel;
};

/**
 * Image loader plugin factory class
 * A factory for creating image loader plugin instances
 */
class ENVILoaderPluginFactory : public LoaderPluginFactory
{
    Q_INTERFACES(mv::plugin::LoaderPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.ENVILoader"
                      FILE  "ENVILoaderPlugin.json")

public:
    /** Default constructor */
    ENVILoaderPluginFactory(void) {}

    /** Destructor */
    ~ENVILoaderPluginFactory(void) override {}

    /** Creates an image loader plugin instance */
    LoaderPlugin* produce() override;

	/** Returns the supported data types */
	mv::DataTypes supportedDataTypes() const override;
};
