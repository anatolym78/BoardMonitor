#pragma once

#include "./BaseDocTemplate.h"

#include "./PointsDoc.h"
#include "./PointsPresenter.h"

class PointsDocTemplate : public BaseDocTemplate
{
	Q_OBJECT

public:
	PointsDocTemplate(QObject *parent);
	~PointsDocTemplate();

	virtual void processDocument() override;

	virtual void stopProcess() override;

	virtual bool isValid() override;

	PointsDoc* faceDocument() { return (PointsDoc*)m_pDoc; }

protected:
	virtual void createDocument() override;

	virtual void setPresenters() override;
};
