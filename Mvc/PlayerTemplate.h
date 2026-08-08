#pragma once

#include "BaseDocTemplate.h"
#include "PlayerDocument.h"

class PlayerView;

class PlayerTemplate : public BaseDocTemplate
{
	Q_OBJECT

public:
	explicit PlayerTemplate(QObject* parent = nullptr);
	~PlayerTemplate() override;

	PlayerDocument* playerDocument() const;
	PlayerView* playerView() const;

	void processDocument() override;
	void stopProcess() override;
	bool isValid() override;

protected:
	void createDocument() override;
	void createViews(QWidget* pwin) override;
	void setPresenters() override;
};
