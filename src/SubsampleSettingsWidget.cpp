#include "SubsampleSettingsWidget.h"
#include "ENVILoaderPlugin.h"

#include <actions/GroupAction.h>

#include <QDebug>
#include <QFileDialog>
#include <iostream>

SubsampleSettingsWidget::SubsampleSettingsWidget(ENVILoaderPlugin& ENVILoaderPlugin, QString fileName, QWidget* parent) :
    QDialog(parent),
    _enableSubsamplingToogle(this, "Enable Subsampling"),
    _subsamplingRatio(this),
    _infoText(this, "Output Size"),
    _loadTrigger(this, "Load")
{
    setWindowTitle(tr("Load ENVI"));

    auto& ENVILoaderModel = ENVILoaderPlugin.getENVILoaderModel();
    _extents = ENVILoaderModel.init(&ENVILoaderPlugin, fileName);

    updateOutputSizeIndicator();

    _subsamplingRatio.setEnabled(false);

    auto layout = new QGridLayout();

    layout->addWidget(_enableSubsamplingToogle.createLabelWidget(this), 0, 0);
    layout->addWidget(_enableSubsamplingToogle.createWidget(this), 0, 1);

    layout->addWidget(_subsamplingRatio.createLabelWidget(this), 1, 0);
    layout->addWidget(_subsamplingRatio.createWidget(this), 1, 1);

    layout->addWidget(_infoText.createLabelWidget(this), 2, 0);
    layout->addWidget(&_outputSize, 2, 1);

    layout->addWidget(_loadTrigger.createLabelWidget(this), 3, 0);
    layout->addWidget(_loadTrigger.createWidget(this), 3, 1);

    layout->setContentsMargins(5, 5, 5, 5);

    setLayout(layout);
    //setFixedWidth(600);

    // Accept when the load action is triggered
    connect(&_loadTrigger, &TriggerAction::triggered, this, [this](bool checked) {
        accept();
        });

    connect(&_subsamplingRatio.getRatioAction(), &DecimalAction::valueChanged, this, [this](const float& value) {
        updateOutputSizeIndicator();
        });

    connect(&_enableSubsamplingToogle, &ToggleAction::toggled, this, [this](bool toggled) {
        _subsamplingRatio.setEnabled(toggled);

        updateOutputSizeIndicator();
        });

}

void SubsampleSettingsWidget::updateOutputSizeIndicator()
{
    QString label;
    if (_enableSubsamplingToogle.isChecked())
    {
        float ratio = _subsamplingRatio.getRatioAction().getValue() * 0.01f;

        label = QString::number((size_t)round(_extents.first*ratio)) + " x " + QString::number((size_t)round(_extents.second*ratio));
    }
    else
    {
        label = QString::number(_extents.first) + " x " + QString::number(_extents.second);
    }
    _outputSize.setText(label);
}

