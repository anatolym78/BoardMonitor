#ifndef INPUTCONTROLLERSENDER_H
#define INPUTCONTROLLERSENDER_H

#include <QtCore>

class InputControllerSender : public QObject
{
    Q_OBJECT
public:
    explicit InputControllerSender(QObject* parent = nullptr);

    void sendAction();

signals:
    void action();
};

#endif // INPUTCONTROLLERSENDER_H
