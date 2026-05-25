#ifndef SESSIONSELECTIONMODEL_H
#define SESSIONSELECTIONMODEL_H

#include <QItemSelectionModel>

class SessionSelectionModel : public QItemSelectionModel
{
	Q_OBJECT
public:
	explicit SessionSelectionModel(QAbstractItemModel* model, QObject* parent = nullptr);
	~SessionSelectionModel() override = default;
};

#endif // SESSIONSELECTIONMODEL_H


