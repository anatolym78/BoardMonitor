#include "UplinkEditorView.h"

#include "../Tools/UplinkEditorDelegates.h"
#include "../../Model/Parameters/Tree/ParameterTreeItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "../../Model/Parameters/Tree/ParameterTreeGroupItem.h"
#include "../../Model/Parameters/Tree/ParameterTreeArrayItem.h"
#include "../../Model/DriverAdapter.h"
#include "../../ViewModel/UplinkParametersTreeModel.h"

#include <QHeaderView>
#include <QFrame>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QVariantMap>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>

namespace {

ParameterTreeItem* replicatePathToSnapshot(ParameterTreeItem* targetNode, ParameterTreeStorage* snapshot)
{
	if (!targetNode || targetNode->type() == ParameterTreeItem::ItemType::Root)
	{
		return snapshot;
	}

	QList<ParameterTreeItem*> path;
	for (ParameterTreeItem* node = targetNode;
		 node && node->type() != ParameterTreeItem::ItemType::Root;
		 node = node->parentItem())
	{
		path.prepend(node);
	}

	ParameterTreeItem* currentParent = snapshot;
	for (ParameterTreeItem* node : path)
	{
		ParameterTreeItem* child = currentParent->findChildByLabel(node->label());
		if (!child)
		{
			switch (node->type())
			{
			case ParameterTreeItem::ItemType::Group:
				child = new ParameterTreeGroupItem(node->label(), currentParent);
				break;
			case ParameterTreeItem::ItemType::Array:
				child = new ParameterTreeArrayItem(node->label(), currentParent);
				break;
			default:
				return currentParent;
			}
			currentParent->appendChild(child);
		}
		currentParent = child;
	}

	return currentParent;
}

void copyHistoryItemToSnapshot(ParameterTreeHistoryItem* source,
	ParameterTreeItem* parent,
	const QVariant& value)
{
	auto* copy = new ParameterTreeHistoryItem(source->label(), parent);
	copy->addValue(value, QDateTime::currentDateTime());
	copy->setControl(source->control());
	copy->setMin(source->min());
	copy->setMax(source->max());
	parent->appendChild(copy);
}

} // namespace

UplinkEditorView::UplinkEditorView(QWidget *parent)
	: QTreeView(parent)
{
	setAlternatingRowColors(false);
	setFrameShape(QFrame::NoFrame);
	setRootIsDecorated(true);
	setItemsExpandable(true);
	setAllColumnsShowFocus(true);
	setUniformRowHeights(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setExpandsOnDoubleClick(false);

	// Настройка заголовка
	header()->setStretchLastSection(true);
}

void UplinkEditorView::setModel(QAbstractItemModel* model)
{
	// Отключаемся от старой модели
	if (this->model())
	{
		disconnect(this->model(), nullptr, this, nullptr);
	}

	QTreeView::setModel(model);

	if (!model)
		return;
	
	model->setHeaderData(0, Qt::Horizontal, tr("label"));
	model->setHeaderData(1, Qt::Horizontal, tr("value"));
	model->setHeaderData(2, Qt::Horizontal, tr("control"));

	header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	header()->setSectionResizeMode(1, QHeaderView::Stretch);
	header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	
	// Устанавливаем делегат для третьей колонки
	m_delegate = new UplinkEditorDelegates(this);
	setItemDelegateForColumn(2, m_delegate);
	
	// Подключаемся к сигналам модели для пересоздания виджетов при обновлении
	connect(model, &QAbstractItemModel::modelReset, this, &UplinkEditorView::setupControlWidgets);
	connect(model, &QAbstractItemModel::layoutChanged, this, &UplinkEditorView::setupControlWidgets);
	
	// Создаем постоянные виджеты-контролы после установки модели
	// Используем QTimer::singleShot чтобы дождаться полной инициализации модели
	QTimer::singleShot(100, this, &UplinkEditorView::setupControlWidgets);
}

void UplinkEditorView::setSendDataImmediately(bool immediately)
{
	m_sendDataImmediately = immediately;

	//QMessageBox::information(nullptr, "info", immediately ? "checked" : "unchecked");
}

void UplinkEditorView::setupControlWidgets()
{
	qDebug() << "UplinkEditorView::setupControlWidgets - called";
	
	// Очищаем все существующие виджеты
	QAbstractItemModel* model = this->model();
	if (!model)
	{
		qDebug() << "UplinkEditorView::setupControlWidgets - no model";
		return;
	}
	
	qDebug() << "UplinkEditorView::setupControlWidgets - model has" << model->rowCount() << "rows";
	
	// Очищаем виджеты рекурсивно
	clearControlWidgetsRecursive();
	
	// Рекурсивно создаем виджеты для всех History элементов
	setupControlWidgetsRecursive();
	
	qDebug() << "UplinkEditorView::setupControlWidgets - completed";
}

void UplinkEditorView::clearControlWidgetsRecursive(const QModelIndex &parent)
{
	QAbstractItemModel* model = this->model();
	if (!model)
		return;
	
	int rowCount = model->rowCount(parent);
	for (int row = 0; row < rowCount; ++row)
	{
		QModelIndex controlIndex = model->index(row, 2, parent);
		if (controlIndex.isValid())
		{
			QWidget* widget = indexWidget(controlIndex);
			if (widget)
			{
				setIndexWidget(controlIndex, nullptr);
				widget->deleteLater();
			}
		}
		
		QModelIndex index = model->index(row, 0, parent);
		if (index.isValid() && model->hasChildren(index))
		{
			clearControlWidgetsRecursive(index);
		}
	}
}

void UplinkEditorView::setupControlWidgetsRecursive(const QModelIndex &parent)
{
	QAbstractItemModel* model = this->model();
	if (!model)
		return;
	
	int rowCount = model->rowCount(parent);
	qDebug() << "UplinkEditorView::setupControlWidgetsRecursive - rowCount:" << rowCount;
	
	for (int row = 0; row < rowCount; ++row)
	{
		QModelIndex index = model->index(row, 0, parent);
		if (!index.isValid())
			continue;
		
		// Проверяем, является ли элемент History элементом
		auto treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (treeItem && treeItem->type() == ParameterTreeItem::ItemType::History)
		{
			qDebug() << "UplinkEditorView::setupControlWidgetsRecursive - found History item at row:" << row;
			
			// Создаем виджет-контрол для третьей колонки
			QModelIndex controlIndex = model->index(row, 2, parent);
			if (controlIndex.isValid())
			{
				QWidget* controlWidget = createControlWidget(controlIndex);
				if (controlWidget)
				{
					qDebug() << "UplinkEditorView::setupControlWidgetsRecursive - created widget for row:" << row;
					setIndexWidget(controlIndex, controlWidget);
				}
				else
				{
					qDebug() << "UplinkEditorView::setupControlWidgetsRecursive - failed to create widget for row:" << row;
				}
			}
			else
			{
				qDebug() << "UplinkEditorView::setupControlWidgetsRecursive - invalid controlIndex for row:" << row;
			}
		}
		
		// Рекурсивно обрабатываем дочерние элементы
		if (model->hasChildren(index))
		{
			setupControlWidgetsRecursive(index);
		}
	}
}

QWidget* UplinkEditorView::createControlWidget(const QModelIndex &index) const
{
	if (!index.isValid())
	{
		qDebug() << "UplinkEditorView::createControlWidget - invalid index";
		return nullptr;
	}
	
	// Получаем данные о контроле через EditRole
	QVariantMap controlData = index.data(Qt::EditRole).toMap();
	QString controlType = controlData["control"].toString();
	
	qDebug() << "UplinkEditorView::createControlWidget - row:" << index.row() 
			 << "column:" << index.column() 
			 << "controlType:" << controlType
			 << "controlData keys:" << controlData.keys();
	
	if (controlType.isEmpty())
	{
		qDebug() << "UplinkEditorView::createControlWidget - controlType is empty";
		return nullptr;
	}
	
	QVariant minValue = controlData["min"];
	QVariant maxValue = controlData["max"];
	QVariant currentValue = controlData["value"];
	QString valueType = controlData["valueType"].toString();
	
	QWidget* widget = nullptr;
	
	if (controlType == "QSlider")
	{
		QSlider *slider = new QSlider(Qt::Horizontal, const_cast<UplinkEditorView*>(this));
		if (minValue.isValid() && maxValue.isValid())
		{
			slider->setMinimum(minValue.toInt());
			slider->setMaximum(maxValue.toInt());
		}
		if (currentValue.isValid())
		{
			slider->setValue(currentValue.toInt());
		}
		widget = slider;
		
		// Подключаем сигнал изменения значения
		connect(slider, static_cast<void(QSlider::*)(int)>(&QSlider::valueChanged), 
				[this, index](int value) {
					if (model())
					{
						model()->setData(index, value, Qt::EditRole);
					}
					sendParameterSnapshot(index, value);
				});
	}
	
	if (controlType == "QSpinBox")
	{
		QSpinBox *spinBox = new QSpinBox(const_cast<UplinkEditorView*>(this));
		spinBox->setFrame(false);
		if (minValue.isValid() && maxValue.isValid())
		{
			spinBox->setMinimum(minValue.toInt());
			spinBox->setMaximum(maxValue.toInt());
		}
		if (currentValue.isValid())
		{
			spinBox->setValue(currentValue.toInt());
		}
		widget = spinBox;
		
		connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
				[this, index](int value) {
					if (model())
					{
						model()->setData(index, value, Qt::EditRole);
					}
					sendParameterSnapshot(index, value);
				});
	}
	
	if (controlType == "QDoubleSpinBox")
	{
		QDoubleSpinBox *doubleSpinBox = new QDoubleSpinBox(const_cast<UplinkEditorView*>(this));
		doubleSpinBox->setFrame(false);
		if (minValue.isValid() && maxValue.isValid())
		{
			doubleSpinBox->setMinimum(minValue.toDouble());
			doubleSpinBox->setMaximum(maxValue.toDouble());
		}
		if (currentValue.isValid())
		{
			doubleSpinBox->setValue(currentValue.toDouble());
		}
		widget = doubleSpinBox;
		
		connect(doubleSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				[this, index](double value) {
					if (model())
					{
						model()->setData(index, value, Qt::EditRole);
					}
					sendParameterSnapshot(index, value);
				});
	}
	
	if (controlType == "QComboBox")
	{
		QComboBox *comboBox = new QComboBox(const_cast<UplinkEditorView*>(this));
		comboBox->setFrame(false);
		// Для QComboBox можно добавить логику заполнения списка значений
		widget = comboBox;
		
		connect(comboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				[this, index, comboBox]() {
					if (model())
					{
						QVariant value = comboBox->currentData();
						if (!value.isValid())
						{
							value = comboBox->currentText();
						}
						model()->setData(index, value, Qt::EditRole);
					}
					QVariant value = comboBox->currentData();
					if (!value.isValid())
					{
						value = comboBox->currentText();
					}
					sendParameterSnapshot(index, value);
				});
	}
	
	if (controlType == "QCheckBox")
	{
		QCheckBox *checkBox = new QCheckBox(const_cast<UplinkEditorView*>(this));
		if (currentValue.isValid())
		{
			checkBox->setChecked(currentValue.toBool());
		}
		widget = checkBox;
		
		connect(checkBox, &QCheckBox::stateChanged,
				[this, index](int state) {
					if (model())
					{
						bool checked = (state == Qt::Checked);
						model()->setData(index, checked, Qt::EditRole);
					}
					bool checked = (state == Qt::Checked);
					sendParameterSnapshot(index, checked);
				});
	}
	
	if (controlType == "QLineEdit")
	{
		QLineEdit*textEdit = new QLineEdit(const_cast<UplinkEditorView*>(this));
		textEdit->setMaximumHeight(60);
		textEdit->setMaximumWidth(200);
		
		if (currentValue.isValid())
		{
			textEdit->setText(currentValue.toString());
		}
		widget = textEdit;
		
		// Подключаем сигнал изменения текста с валидацией
						
		//QVariant newValue;
		connect(textEdit, &QLineEdit::textChanged,
				[this, index, textEdit, valueType, minValue, maxValue]() 
				{
					QVariant newValue;

					if (model())
					{
						QString text = textEdit->text();
						bool isValid = true;
						
						if (valueType == "int")
						{
							bool ok;
							int intValue = text.toInt(&ok);
							if (ok)
							{
								if (minValue.isValid() && maxValue.isValid())
								{
									if (intValue >= minValue.toInt() && intValue <= maxValue.toInt())
									{
										newValue = intValue;
									}
									else
									{
										isValid = false;
									}
								}
								else
								{
									newValue = intValue;
								}
							}
							else if (!text.isEmpty())
							{
								isValid = false;
							}
						}
						else if (valueType == "double")
						{
							bool ok;
							double doubleValue = text.toDouble(&ok);
							if (ok)
							{
								if (minValue.isValid() && maxValue.isValid())
								{
									if (doubleValue >= minValue.toDouble() && doubleValue <= maxValue.toDouble())
									{
										newValue = doubleValue;
									}
									else
									{
										isValid = false;
									}
								}
								else
								{
									newValue = doubleValue;
								}
							}
							else if (!text.isEmpty())
							{
								isValid = false;
							}
						}
						else
						{
							// Для строковых значений просто сохраняем текст
							newValue = text;
						}
						
						if (isValid && newValue.isValid())
						{
							model()->setData(index, newValue, Qt::EditRole);
						}
					}
					sendParameterSnapshot(index, newValue);
				});
	}
	
	return widget;
}

void UplinkEditorView::sendParameterSnapshot(const QModelIndex &index, const QVariant &value) const
{
	if (!m_driverAdapter || !m_parametersModel)
	{
		qWarning() << "UplinkEditorView::sendParameterSnapshot - DriverAdapter or ParametersModel not set";
		return;
	}

	// Получаем элемент дерева
	auto treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
	if (!treeItem || treeItem->type() != ParameterTreeItem::ItemType::History)
	{
		return;
	}

	auto historyItem = static_cast<ParameterTreeHistoryItem*>(treeItem);

	ParameterTreeStorage* snapshot = new ParameterTreeStorage();

	QString sentLabel;
	QVariant sentValue;

	ParameterTreeItem* parentItem = historyItem->parentItem();
	if (parentItem && parentItem->type() == ParameterTreeItem::ItemType::Array)
	{
		auto* arrayItem = static_cast<ParameterTreeArrayItem*>(parentItem);
		ParameterTreeItem* arrayInSnapshot = replicatePathToSnapshot(arrayItem, snapshot);

		QVariantList arrayValues;
		for (int i = 0; i < arrayItem->childCount(); ++i)
		{
			auto* childHistory = static_cast<ParameterTreeHistoryItem*>(arrayItem->child(i));
			if (!childHistory)
			{
				continue;
			}

			const QVariant childValue = (childHistory == historyItem) ? value : childHistory->lastValue();
			copyHistoryItemToSnapshot(childHistory, arrayInSnapshot, childValue);
			arrayValues.append(childValue);
		}

		sentLabel = arrayItem->fullName();
		sentValue = arrayValues;
	}
	else
	{
		ParameterTreeItem* parentInSnapshot = replicatePathToSnapshot(parentItem, snapshot);
		copyHistoryItemToSnapshot(historyItem, parentInSnapshot, value);

		sentLabel = historyItem->fullName();
		sentValue = value;
	}

	m_driverAdapter->sendParameterTreeSnapshot(snapshot);

	qDebug() << "UplinkEditorView: Sent parameter" << sentLabel << "with value" << sentValue;

	// Удаляем временный snapshot
	snapshot->deleteLater();
}

