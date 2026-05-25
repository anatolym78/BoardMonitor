#ifndef UPLINKPARAMETERSTREEMODEL_H
#define UPLINKPARAMETERSTREEMODEL_H

#include "BoardParametersTreeModel.h"

class UplinkParametersTreeModel : public BoardParametersTreeModel
{
	Q_OBJECT

public:
	explicit UplinkParametersTreeModel(QObject* parent = nullptr);
	~UplinkParametersTreeModel() override;
};

#endif // UPLINKPARAMETERSTREEMODEL_H

