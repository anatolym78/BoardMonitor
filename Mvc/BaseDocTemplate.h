#pragma once

#include "DocTemplate.h"

#include "./BaseView.h"
#include "BaseDoc.h"

#include <QImage>
#include <QPointer>

//class DocPhoto;
class BaseDocTemplate : public DocTemplate
{
	Q_OBJECT
public:
	BaseDocTemplate(QObject* parent);
	virtual ~BaseDocTemplate();

	virtual void create(QWidget* pwin);
	virtual void processDocument();
	virtual void stopProcess();
	virtual BaseView* getView(int index);
	virtual BaseDoc* getDoc();
	virtual bool isValid();
	virtual int countViews() const { return (int)m_pViews.size(); }
	virtual void updateViews() ;
	virtual bool loadImage(QString fname);
	void setProgramWasClosed() { m_bProramClosed = true; }

public slots:
	void onProcess();
	void onStopProcess();
	void onZoomFit();
	void onZoomNormal();

	void onImageUpdated(const QImage& img);
	void onDataUpdated();

protected:
	BaseDoc* m_pDoc;
	//std::vector<BaseView*> m_pViews;
	std::vector<QPointer<BaseView> > m_pViews;

protected:
	virtual void createDocument();
	virtual void createViews(QWidget* pwin);
	virtual void connectDocSignals();
	virtual void setPresenters();
protected:
	virtual void clearViews();

protected:
	bool m_bProramClosed;
};
