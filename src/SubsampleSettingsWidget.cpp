#include "SubsampleSettingsWidget.h"
#include "ENVILoaderPlugin.h"

#include "ui_SubsampleSettingsWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <iostream>

SubsampleSettingsWidget::SubsampleSettingsWidget(QWidget* parent) :
    _ui(new Ui::SubsampleSettingsWidget()),
    _ENVILoaderPlugin(nullptr)
{
    _ui->setupUi(this);
}

SubsampleSettingsWidget::~SubsampleSettingsWidget() = default;

void SubsampleSettingsWidget::updateOutputSizeIndicator()
{
    QString label;
    if (_ui->enabledCheckbox->isChecked())
    {
        float ratio = _ui->ratioSpinBox->value() * 0.01f;

        label = QString::number((size_t)round(_extents.first*ratio)) + " x " + QString::number((size_t)round(_extents.second*ratio));
    }
    else
    {
        label = QString::number(_extents.first) + " x " + QString::number(_extents.second);
    }
    _ui->outputSize->setText(label);
}

void SubsampleSettingsWidget::initialize(ENVILoaderPlugin* ENVILoaderPlugin, QString fileName)
{
    _ENVILoaderPlugin = ENVILoaderPlugin;

    auto& ENVILoaderModel = _ENVILoaderPlugin->getENVILoaderModel();
    _extents = ENVILoaderModel.init(_ENVILoaderPlugin, fileName);

    float ratioSub;
    int filter;

    setWindowTitle(tr("Load ENVI"));

    _ui->enabledCheckbox->setChecked(false);
//    _ui->closeCheckBox->setChecked(true);

//    _ui->filterComboBox->setDisabled(true);
    _ui->ratio25PushButton->setDisabled(true);
    _ui->ratio50PushButton->setDisabled(true);
    _ui->ratio75PushButton->setDisabled(true);
    _ui->ratioSlider->setDisabled(true);
    _ui->ratioSpinBox->setDisabled(true);

    updateOutputSizeIndicator();

    QObject::connect(_ui->enabledCheckbox, &QCheckBox::stateChanged, [&](int state) {

        if (state == 0) {
//            _ui->filterComboBox->setDisabled(true);
            _ui->ratio25PushButton->setDisabled(true);
            _ui->ratio50PushButton->setDisabled(true);
            _ui->ratio75PushButton->setDisabled(true);
            _ui->ratioSlider->setDisabled(true);
            _ui->ratioSpinBox->setDisabled(true);
        }
        else if (state == 2) {
//            _ui->filterComboBox->setDisabled(false);
            _ui->ratio25PushButton->setDisabled(false);
            _ui->ratio50PushButton->setDisabled(false);
            _ui->ratio75PushButton->setDisabled(false);
            _ui->ratioSlider->setDisabled(false);
            _ui->ratioSpinBox->setDisabled(false);
        }

        updateOutputSizeIndicator();
    });

    connect(_ui->ratio25PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(2500.0);
        _ui->ratioSpinBox->setValue(25.0);
        });

    connect(_ui->ratio50PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(5000.0);
        _ui->ratioSpinBox->setValue(50.0);
        });

    connect(_ui->ratio75PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(7500.0);
        _ui->ratioSpinBox->setValue(75.0);
        });

    connect(_ui->ratioSlider, &QSlider::valueChanged, [&](int ratio) {
        _ui->ratioSpinBox->setValue((double)ratio*0.01);
        });

    connect(_ui->ratioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double ratio) {
        _ui->ratioSlider->setValue((int)(ratio*100));
        updateOutputSizeIndicator();
        });

    QObject::connect(_ui->loadButton, &QPushButton::clicked, [&]() {
        
        if (_ui->enabledCheckbox->isChecked()) {
            filter = 1;// _ui->filterComboBox->currentIndex();
            ratioSub = _ui->ratioSpinBox->value() * 0.01f;
        }
        else {
            filter = -1;
            ratioSub = 1;
        }

        // load file with given subsampling settings
        const auto loaded = ENVILoaderModel.load(ratioSub, filter, true);

        if (loaded && _ENVILoaderPlugin->getSetting("GUI/CloseAfterLoaded", true).toBool())
            this->close();
        });

    if (_ENVILoaderPlugin->getSetting("GUI/CloseAfterLoaded", true).toBool()) {
        this->close();
    }

}