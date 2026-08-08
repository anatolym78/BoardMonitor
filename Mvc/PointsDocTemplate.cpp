#include "PointsDocTemplate.h"

PointsDocTemplate::PointsDocTemplate(QObject *parent)
	: BaseDocTemplate(parent)
{
}

void PointsDocTemplate::createDocument()
{
	if (m_pDoc) delete m_pDoc;
	m_pDoc = 0;
	m_pDoc = new PointsDoc(this);
}

void PointsDocTemplate::setPresenters()
{
	for (size_t i = 0; i < m_pViews.size(); i++)
	{
		m_pViews[i]->setPresenter(new PointsPresenter(faceDocument()));
	}
}

void PointsDocTemplate::processDocument()
{
}

void PointsDocTemplate::stopProcess()
{
}

bool PointsDocTemplate::isValid()
{
	return true;
}

PointsDocTemplate::~PointsDocTemplate()
{
}
