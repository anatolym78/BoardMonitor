#ifndef CONTROLDELEGATE_H
#define CONTROLDELEGATE_H

#include <QStyledItemDelegate>

class DriverAdapter;
class UplinkParametersTreeModel;

/**
 * @brief Делегат для редактора Uplink параметров.
 * 
 * Отвечает за создание и отображение виджетов редактирования (QSpinBox, QSlider, QCheckBox и т.д.)
 * прямо в ячейках дерева UplinkEditorView.
 */
class UplinkEditorDelegates : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit UplinkEditorDelegates(QObject *parent = nullptr);
	
	void setDriverAdapter(DriverAdapter* adapter) { m_driverAdapter = adapter; }
	void setParametersModel(UplinkParametersTreeModel* model) { m_parametersModel = model; }

	// QStyledItemDelegate interface
	QWidget* createEditor(QWidget *parent, 
	                      const QStyleOptionViewItem &option, 
	                      const QModelIndex &index) const override;
	
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;
	void setModelData(QWidget *editor, QAbstractItemModel *model, 
	                  const QModelIndex &index) const override;
	
	void updateEditorGeometry(QWidget *editor, 
	                         const QStyleOptionViewItem &option, 
	                         const QModelIndex &index) const override;

private:
	bool isHistoryItem(const QModelIndex &index) const;
	
private:
	DriverAdapter* m_driverAdapter = nullptr;
	UplinkParametersTreeModel* m_parametersModel = nullptr;
};

#endif // CONTROLDELEGATE_H

