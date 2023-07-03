#include "SubsamplingRatioAction.h"

#include <QHBoxLayout>

using namespace hdps::gui;

const QMap<SubsamplingRatioAction::Ratio, TriggersAction::Trigger> SubsamplingRatioAction::triggers = QMap<SubsamplingRatioAction::Ratio, TriggersAction::Trigger>({
    { SubsamplingRatioAction::Perc25, TriggersAction::Trigger("25.0%", "Scale down to 25%") },
    { SubsamplingRatioAction::Perc50, TriggersAction::Trigger("50.0%", "Scale down to 50%") },
    { SubsamplingRatioAction::Perc75, TriggersAction::Trigger("75.0%", "Scale down to 75%") }
});

const QMap<SubsamplingRatioAction::Ratio, float> SubsamplingRatioAction::defaultRatios = QMap<SubsamplingRatioAction::Ratio, float>({
    { SubsamplingRatioAction::Perc25, 25.0f },
    { SubsamplingRatioAction::Perc50, 50.0f },
    { SubsamplingRatioAction::Perc75, 75.0f }
});

SubsamplingRatioAction::SubsamplingRatioAction(QObject* parent) :
    WidgetAction(parent, "SubsamplingRatioAction"),
    _ratioAction(this, "Ratio", 1.0f, 100.0f, 50.0f, 50.0f),
    _defaultRatiosAction(this, "Default ratios", triggers.values().toVector())
{
    setText("Ratio");
    setCheckable(true);

    _ratioAction.setToolTip("Subsampling ratio");
    _ratioAction.setSuffix("%");
        
    //connect(&_ratioAction, &DecimalAction::valueChanged, this, &SubsamplingRatioAction::updateRows);

    connect(&_defaultRatiosAction, &TriggersAction::triggered, this, [this](std::int32_t triggerIndex) {
        _ratioAction.setValue(defaultRatios.values().at(triggerIndex));
    });
}

SubsamplingRatioAction::Widget::Widget(QWidget* parent, SubsamplingRatioAction* subsamplingRatioAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, subsamplingRatioAction, widgetFlags)
{
    auto layout = new QHBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(subsamplingRatioAction->_ratioAction.createWidget(this), 1);
    layout->addWidget(subsamplingRatioAction->_defaultRatiosAction.createWidget(this));

    setLayout(layout);
}
