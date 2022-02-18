#pragma once

#include <QDialog>

namespace Ui {
    class SubsampleSettingsWidget;
}

class ENVILoaderPlugin;

/**
 * Subsample settings widget class
 *
 * Widget class for image subsampling settings
 *
 * @author Thomas Kroes
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
     * @param imageLoaderPlugin Pointer to image loader plugin
     */
    void initialize(ENVILoaderPlugin* ENVILoaderPlugin, QString fileName);

private slots:
    void on_loadButton_clicked();

private:
    QSharedPointer<Ui::SubsampleSettingsWidget>     _ui;                    /** Externally loaded UI */
    ENVILoaderPlugin*                              _ENVILoaderPlugin;     /** Pointer to image loader plugin (for interfacing with data models) */
};