#pragma once

#include <actions/WidgetAction.h>
#include <actions/DecimalAction.h>
#include <actions/TriggersAction.h>

class ImageLoaderPlugin;

using namespace hdps::gui;

class SubsamplingRatioAction : public WidgetAction
{
public:

    /** Subsampling ratio factors */
    enum Ratio {
        Perc25,     /** Scale by 25% */
        Perc50,     /** Scale by 50% */
        Perc75      /** Scale by 75% */
    };

    static const QMap<Ratio, TriggersAction::Trigger> triggers;     /** Maps ratio enum to trigger */
    static const QMap<Ratio, float> defaultRatios;                  /** Maps ratio enum to ratio value */

protected:

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, SubsamplingRatioAction* subsamplingRatioAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    SubsamplingRatioAction(QObject* parent);

    DecimalAction& getRatioAction() { return _ratioAction; }

private:
    DecimalAction           _ratioAction;               /** Subsampling ratio action */
    TriggersAction          _defaultRatiosAction;       /** Default subsampling ratios action */

    friend class Widget;
};