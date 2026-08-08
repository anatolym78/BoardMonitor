#include "PlayerTemplate.h"

#include "PlayerDocument.h"
#include "PlayerPresenter.h"
#include "PlayerView.h"

PlayerTemplate::PlayerTemplate(QObject* parent)
	: BaseDocTemplate(parent)
{
}

PlayerTemplate::~PlayerTemplate()
{
}

PlayerDocument* PlayerTemplate::playerDocument() const
{
	return qobject_cast<PlayerDocument*>(m_pDoc);
}

PlayerView* PlayerTemplate::playerView() const
{
	if (m_pViews.empty())
	{
		return nullptr;
	}
	return qobject_cast<PlayerView*>(m_pViews.front().data());
}

void PlayerTemplate::createDocument()
{
	if (m_pDoc)
	{
		delete m_pDoc;
	}
	m_pDoc = new PlayerDocument(this);
}

void PlayerTemplate::createViews(QWidget* pwin)
{
	clearViews();
	m_pViews.resize(1);
	m_pViews[0] = new PlayerView(pwin);
}

void PlayerTemplate::setPresenters()
{
	for (int i = 0; i < countViews(); ++i)
	{
		m_pViews[i]->setPresenter(new PlayerPresenter(playerDocument()));
	}
}

void PlayerTemplate::processDocument()
{
}

void PlayerTemplate::stopProcess()
{
}

bool PlayerTemplate::isValid()
{
	return BaseDocTemplate::isValid() && playerDocument() != nullptr;
}
