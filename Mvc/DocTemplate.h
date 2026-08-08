#pragma once

#include <QObject>

class BaseView;
class BaseDoc;
class DocTemplate : public QObject
{
	Q_OBJECT
public:
	DocTemplate(QObject* parent);
	virtual ~DocTemplate();
	virtual void create(QWidget* pwin);
	virtual void processDocument() = 0;
	virtual void stopProcess() = 0;
	virtual BaseView* getView(int index) = 0;
	virtual int countViews() const = 0;
	virtual BaseDoc* getDoc() = 0;
	virtual bool isValid() = 0;
	virtual void updateViews() = 0;

protected:
	virtual void createDocument() = 0;
	virtual void createViews(QWidget* pwin) = 0;
	virtual void connectDocSignals() = 0;
	virtual void setPresenters() = 0;
};