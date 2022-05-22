#pragma once

#include <QDialog>

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
    SubsampleSettingsWidget(QWidget* parent = nullptr);

    ~SubsampleSettingsWidget() override;

    /**
     * Initializes the widget
     * @param ENVILoaderPlugin Pointer to ENVI loader plugin
     */
    void initialize(ENVILoaderPlugin* ENVILoaderPlugin, QString fileName);

    void updateOutputSizeIndicator();

private slots:
    void on_loadButton_clicked();

private:
    QSharedPointer<Ui::SubsampleSettingsWidget>     _ui;                    /** Externally loaded UI */
    ENVILoaderPlugin*                              _ENVILoaderPlugin;     /** Pointer to ENVI loader plugin (for interfacing with data models) */
    std::pair<size_t, size_t> _extents;
};