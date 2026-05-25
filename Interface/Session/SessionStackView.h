#ifndef SESSIONSSTACKWIDGET_H
#define SESSIONSSTACKWIDGET_H

#include <QStackedWidget>

#include "SessionWorkspace.h"

/**
 * @brief Виджет для управления отображением нескольких активных сессий.
 * 
 * Наследуется от QStackedWidget и позволяет переключаться между различными
 * рабочими пространствами (SessionWorkspace), каждое из которых соответствует
 * открытой сессии (живой или записанной).
 */
class SessionStackView : public QStackedWidget
{
	Q_OBJECT
public:
	explicit SessionStackView(QWidget *parent = nullptr);
	void setCurrentIndex(int index);
	SessionWorkspace* getSessionFrame(int index) const;

signals:
};

#endif // SESSIONSSTACKWIDGET_H
