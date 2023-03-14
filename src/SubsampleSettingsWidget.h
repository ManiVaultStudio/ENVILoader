#pragma once

#include <QDialog>
#include <QLabel>

#include "SubsamplingRatioAction.h"
#include <actions/ToggleAction.h>
#include <actions/StringAction.h>
#include <actions/TriggerAction.h>

namespace Ui {
    class SubsampleSettingsWidget;
}

class ENVILoaderPlugin;

/**
 * Subsample settings widget class
 *
 * Widget class for ENVI data subsampling settings
 *
 * Based on Image subsampling settings
 */
class SubsampleSettingsWidget : public QDialog
{
public:
    /**
     * Constructor
     * @param parent Parent widget
     */
    SubsampleSettingsWidget(ENVILoaderPlugin& ENVILoaderPlugin, QString fileName, QWidget* parent = nullptr);

    bool getEnableSubsamplingToogle() { return _enableSubsamplingToogle.isChecked(); }
    float getRatio() { return _subsamplingRatio.getRatioAction().getValue() * 0.01f; }

private:
    void updateOutputSizeIndicator();

private:
    std::pair<size_t, size_t>   _extents;

    ToggleAction                _enableSubsamplingToogle;
    SubsamplingRatioAction      _subsamplingRatio;
    StringAction                _infoText;
    TriggerAction               _loadTrigger;
    QLabel                      _outputSize;
};