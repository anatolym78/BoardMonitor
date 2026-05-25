#include "SessionStackView.h"

SessionStackView::SessionStackView(QWidget *parent)
    : QStackedWidget(parent)
{
    connect(this, &QStackedWidget::currentChanged, [=](int index)
        {
            int k = 0;
        });
}

void SessionStackView::setCurrentIndex(int index)
{
    QStackedWidget::setCurrentIndex(index);
}

SessionWorkspace* SessionStackView::getSessionFrame(int index) const
{
    auto widget = this->widget(index);

    if (widget == nullptr) return nullptr;

    return (SessionWorkspace*)widget;
}
