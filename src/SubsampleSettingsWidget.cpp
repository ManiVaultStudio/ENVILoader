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

void SubsampleSettingsWidget::initialize(ENVILoaderPlugin* ENVILoaderPlugin, QString fileName)
{
    _ENVILoaderPlugin = ENVILoaderPlugin;


    float ratioSub;
    int filter;

    setWindowTitle(tr("Subsample Window"));

    _ui->enabledCheckbox->setChecked(false);
    _ui->closeCheckBox->setChecked(true);

    _ui->filterComboBox->setDisabled(true);
    _ui->ratio25PushButton->setDisabled(true);
    _ui->ratio50PushButton->setDisabled(true);
    _ui->ratio75PushButton->setDisabled(true);
    _ui->ratioSlider->setDisabled(true);
    _ui->ratioSpinBox->setDisabled(true);

    QObject::connect(_ui->enabledCheckbox, &QCheckBox::stateChanged, [&](int state) {

        if (state == 0) {
            _ui->filterComboBox->setDisabled(true);
            _ui->ratio25PushButton->setDisabled(true);
            _ui->ratio50PushButton->setDisabled(true);
            _ui->ratio75PushButton->setDisabled(true);
            _ui->ratioSlider->setDisabled(true);
            _ui->ratioSpinBox->setDisabled(true);
        }
        else if (state == 2) {
            _ui->filterComboBox->setDisabled(false);
            _ui->ratio25PushButton->setDisabled(false);
            _ui->ratio50PushButton->setDisabled(false);
            _ui->ratio75PushButton->setDisabled(false);
            _ui->ratioSlider->setDisabled(false);
            _ui->ratioSpinBox->setDisabled(false);
        }
    });

    connect(_ui->ratio25PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(25);
        _ui->ratioSpinBox->setValue(25);
        });

    connect(_ui->ratio50PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(50);
        _ui->ratioSpinBox->setValue(50);
        });

    connect(_ui->ratio75PushButton, &QPushButton::clicked, [&]() {
        _ui->ratioSlider->setValue(75);
        _ui->ratioSpinBox->setValue(75);
        });

    connect(_ui->ratioSlider, &QSlider::valueChanged, [&](int ratio) {
        _ui->ratioSpinBox->setValue(ratio);
        });

    connect(_ui->ratioSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [&](double ratio) {
        _ui->ratioSlider->setValue(ratio);
        });

    auto& ENVILoaderModel = _ENVILoaderPlugin->getENVILoaderModel();

    QObject::connect(_ui->loadButton, &QPushButton::clicked, [&]() {
        
        if (_ui->enabledCheckbox->isChecked()) {
            filter = _ui->filterComboBox->currentIndex();
            ratioSub = _ui->ratioSlider->value() * 0.01f;
        }
        else {
            filter = -1;
            ratioSub = 1;
        }

        // load file with given subsampling settings
        const auto loaded = ENVILoaderModel.load(_ENVILoaderPlugin, ratioSub, filter, fileName);

        if (loaded && _ENVILoaderPlugin->getSetting("GUI/CloseAfterLoaded", true).toBool())
            this->close();
        });

    if (_ENVILoaderPlugin->getSetting("GUI/CloseAfterLoaded", true).toBool()) {
        this->close();
    }

}