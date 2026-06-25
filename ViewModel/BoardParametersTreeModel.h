#ifndef BOARDPARAMETERSTREEMODEL_H
#define BOARDPARAMETERSTREEMODEL_H

#include <QAbstractItemModel>
#include <QColor>

#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "DataPlayer.h"

class BoardParametersTreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum ParameterRole
	{
		LabelRole = Qt::UserRole + 1,
		ValueRole,
		UnitRole,
		TimeRole,
		ChartVisibilityRole,
		ColorRole,
		FullPathRole,
		ControlRole
	};
	Q_ENUM(ParameterRole)

public:
	explicit BoardParametersTreeModel(QObject* parent = nullptr);
	~BoardParametersTreeModel() override;

	void setSnapshot(ParameterTreeStorage* storage, bool isBackPlaying);
	void attachExternalStorage(ParameterTreeStorage* storage);
	ParameterTreeStorage* storage() const { return m_rootItem; }

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role) const override;
	bool setData(const QModelIndex& index, const QVariant& value, int role) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole) override;
	QHash<int, QByteArray> roleNames() const override;



public:
	void onParameterAdded(ParameterTreeItem* newItem);
	void onValueAdded(ParameterTreeHistoryItem* updatedItem);
	void onValueChanged(ParameterTreeHistoryItem* history);

signals:
	void structureAboutToReset();

private:
	void makeRandomColors();

	bool findIndexRecursive(ParameterTreeItem* item, QModelIndex parentIndex, QModelIndex& foundedIndex);

private:
	ParameterTreeStorage* m_rootItem = nullptr;
	bool m_ownsRoot = true;
	QList<QColor> m_colors;
	// Для Qt Widgets: хранение заголовков колонок
	QVariant m_horizontalHeaders[3];

	//QMetaObject::Connection m_playConnection;
	//QMetaObject::Connection m_stopConnection;
};

#endif // BOARDPARAMETERSTREEMODEL_H
