#include "BaseDocTemplate.h"

#include "./Presenter.h"

BaseDocTemplate::BaseDocTemplate(QObject* parent) : DocTemplate(parent)
{
	m_pDoc = 0;
	m_bProramClosed = false;
}

BaseDocTemplate::~BaseDocTemplate()
{
	if(m_pDoc)
	{
		clearViews();
	}
}

void BaseDocTemplate::processDocument()
{
	if(m_pDoc) m_pDoc->process();
}

void BaseDocTemplate::stopProcess()
{
	if(m_pDoc) m_pDoc->stopProcess();
}

void BaseDocTemplate::create(QWidget* pwin)
{
	DocTemplate::create(pwin);
}

void BaseDocTemplate::createDocument()
{
	if(m_pDoc) delete m_pDoc;

	m_pDoc = new BaseDoc(this);
}

void BaseDocTemplate::connectDocSignals()
{
	auto res = connect(m_pDoc, &BaseDoc::imageUpdated, this, &BaseDocTemplate::onImageUpdated);
	//bool res = QObject::connect(m_pDoc, SIGNAL(imageUpdated(const BasicImage&)), SLOT(onImageUpdated(const BasicImage&)));
	res = QObject::connect(m_pDoc, SIGNAL(dataUpdated()), SLOT(onDataUpdated()));
}

void BaseDocTemplate::createViews(QWidget* pwin)
{
	clearViews();

	int nviews = 1;
	m_pViews.resize(nviews);
	for(int i=0;i<nviews;i++)
	{
		m_pViews[i] = new BaseView(pwin);
	}
}

void BaseDocTemplate::setPresenters()
{
	for (int i = 0; i < countViews(); i++)
	{
		m_pViews[i]->setPresenter(new Presenter(m_pDoc));
	}
}

void BaseDocTemplate::clearViews()
{
	//if(!m_bProramClosed)
	{
		for(int i=0;i<countViews();i++)
		{
			if (m_pViews[i])
				delete m_pViews[i];
		}
		m_pViews.clear();
	}
}

void BaseDocTemplate::updateViews()
{
	for(int i=0;i<countViews();i++)
	{
        m_pViews[i]->repaint();
	}
}

// ˜˜˜˜˜˜˜˜˜˜˜
void BaseDocTemplate::onProcess()
{
	processDocument();
}

void BaseDocTemplate::onStopProcess()
{
	stopProcess();
}

void BaseDocTemplate::onZoomFit()
{
	for(int i=0;i<countViews();i++)
	{
		m_pViews[i]->zoomFit();
	}
}

void BaseDocTemplate::onZoomNormal()
{
	for(int i=0;i<countViews();i++)
	{
		m_pViews[i]->zoomNormal();
	}
}

void BaseDocTemplate::onImageUpdated(const QImage& img)
{
	for(size_t i=0;i<m_pViews.size();i++)
	{
		m_pViews[i]->onImageUpdated(img);
	}
}

void BaseDocTemplate::onDataUpdated()
{
	for(size_t i=0;i<m_pViews.size();i++)
	{
		m_pViews[i]->update();
	}
}

BaseView* BaseDocTemplate::getView(int index)
{
	assert(index >= 0 && index < countViews());

	return m_pViews[index];
}

BaseDoc* BaseDocTemplate::getDoc()
{
	return m_pDoc;
}

bool BaseDocTemplate::isValid()
{
	bool valid = (m_pDoc != 0) && (countViews() > 0);
	for(int i=0;i<countViews();i++)
	{
		valid &= (m_pViews[i] != 0);
	}

	return valid;
}

bool BaseDocTemplate::loadImage(QString fname)
{
	if(!isValid()) return false;

	if(m_pDoc->loadImage(fname))
	{
		return true;
	}
	else
	{
		return false;
	}
}
