#include "DocTemplate.h"

DocTemplate::DocTemplate(QObject* parent) : QObject(parent)
{
}

DocTemplate::~DocTemplate()
{

}

void DocTemplate::create(QWidget* pwin)
{
	createDocument();
	connectDocSignals();
	createViews(pwin);
	setPresenters();
}