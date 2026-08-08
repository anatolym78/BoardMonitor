#include "InputControllerSender.h"

InputControllerSender::InputControllerSender(QObject *parent)
{

}

void InputControllerSender::sendAction()
{
    emit action();
}
