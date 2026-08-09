#include "BoardParametersTreeModel.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "./../Model/Parameters/Tree/ParameterTreeGroupItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeArrayItem.h"
#include "DataPlayer.h"

#include <functional>
#include <random>
#include <QDateTime>
#include <QIcon>
#include <QDebug>
#include <QTimer>
#include <QtGlobal>

BoardParametersTreeModel::BoardParametersTreeModel(QObject* parent)
	: QAbstractItemModel(parent)
	, m_ownsRoot(true)
{
	makeRandomColors();

	m_rootItem = new ParameterTreeStorage(this);

	connect(m_rootItem, &ParameterTreeStorage::parameterAdded, this, &BoardParametersTreeModel::onParameterAdded);
	connect(m_rootItem, &ParameterTreeStorage::valueAdded, this, &BoardParametersTreeModel::onValueAdded);
	connect(m_rootItem, &ParameterTreeStorage::valueChanged, this, &BoardParametersTreeModel::onValueChanged);

	m_valueRefreshTimer = new QTimer(this);
	m_valueRefreshTimer->setSingleShot(true);
	m_valueRefreshTimer->setInterval(33);
	connect(m_valueRefreshTimer, &QTimer::timeout, this, &BoardParametersTreeModel::refreshValueColumn);
}

void BoardParametersTreeModel::setPlayer(DataPlayer* player)
{
	if (m_playingConnection)
	{
		disconnect(m_playingConnection);
	}
	if (m_positionConnection)
	{
		disconnect(m_positionConnection);
	}

	m_player = player;
	if (!m_player)
	{
		return;
	}

	m_playingConnection = connect(m_player, &DataPlayer::isPlayingChanged,
		this, &BoardParametersTreeModel::scheduleValueColumnRefresh);
	m_positionConnection = connect(m_player, &DataPlayer::currentPositionChanged,
		this, &BoardParametersTreeModel::scheduleValueColumnRefresh, Qt::QueuedConnection);
	scheduleValueColumnRefresh();
}

bool BoardParametersTreeModel::showScrubValue() const
{
	// Только live на паузе: recorded идёт через snapshot
	return m_player && !m_player->isPlayable() && !m_player->isPlaying()
		&& m_player->currentPosition().isValid();
}

QString BoardParametersTreeModel::formatDisplayValue(const QVariant& value)
{
	if (!value.isValid() || value.isNull())
	{
		return QString();
	}
	// Некорректные/пустые варианты (часто «мусор» в toString)
	if (value.userType() == QMetaType::UnknownType)
	{
		return QString();
	}
	if (value.type() == QVariant::Double)
	{
		const double d = value.toDouble();
		if (!qIsFinite(d))
		{
			return QString();
		}
		return QString::number(d, 'g', 8);
	}
	if (value.type() == QVariant::String && value.toString().trimmed().isEmpty())
	{
		return QString();
	}
	const QString text = value.toString();
	if (text.isEmpty() || text.startsWith(QLatin1String("QVariant(")))
	{
		return QString();
	}
	return text;
}

void BoardParametersTreeModel::scheduleValueColumnRefresh()
{
	if (!m_valueRefreshTimer)
	{
		return;
	}
	if (!m_valueRefreshTimer->isActive())
	{
		m_valueRefreshTimer->start();
	}
}

void BoardParametersTreeModel::refreshValueColumn()
{
	std::function<void(const QModelIndex&)> walk = [&](const QModelIndex& parent)
	{
		const int rows = rowCount(parent);
		for (int row = 0; row < rows; ++row)
		{
			const QModelIndex child = index(row, 0, parent);
			auto* item = static_cast<ParameterTreeItem*>(child.internalPointer());
			if (item && item->type() == ParameterTreeItem::ItemType::History)
			{
				const QModelIndex valueIndex = index(row, 1, parent);
				emit dataChanged(valueIndex, valueIndex, { Qt::DisplayRole, ValueRole });
			}
			walk(child);
		}
	};
	walk(QModelIndex());
}

void BoardParametersTreeModel::attachExternalStorage(ParameterTreeStorage* storage)
{
	if (!storage || m_rootItem == storage)
	{
		return;
	}

	if (m_rootItem)
	{
		disconnect(m_rootItem, nullptr, this, nullptr);
	}

	if (m_ownsRoot && m_rootItem)
	{
		delete m_rootItem;
	}

	m_rootItem = storage;
	m_ownsRoot = false;

	connect(m_rootItem, &ParameterTreeStorage::parameterAdded, this, &BoardParametersTreeModel::onParameterAdded);
	connect(m_rootItem, &ParameterTreeStorage::valueAdded, this, &BoardParametersTreeModel::onValueAdded);
	connect(m_rootItem, &ParameterTreeStorage::valueChanged, this, &BoardParametersTreeModel::onValueChanged);

	beginResetModel();
	endResetModel();
}

void BoardParametersTreeModel::setSnapshot(ParameterTreeStorage* storage, bool isBackPlaying)
{
	// Если storage удален или null, или это тот же самый объект, который мы уже используем
	if (!storage || storage == m_rootItem)
	{
		return;
	}
	
	// Если rootItem управляется извне (как в RecordedSession, где он часть сессии),
	// то мы не должны его удалять или заменять указатель на него, если мы не владеем им.
	// Но текущая реализация модели предполагает, что m_rootItem создан в конструкторе и модель им владеет.
	// Метод setSnapshot вызывает m_rootItem->setSnapshot(storage), который копирует структуру.
	// Это безопасно, так как мы копируем данные, а не подменяем указатель.
	
	m_rootItem->setSnapshot(storage);
}

void BoardParametersTreeModel::onParameterAdded(ParameterTreeItem* newItem)
{
	Q_UNUSED(newItem);
	emit structureAboutToReset();
	this->beginResetModel();
	this->endResetModel();
}

void BoardParametersTreeModel::onValueAdded(ParameterTreeHistoryItem* updatedItem)
{
	onValueChanged(updatedItem);
}

void BoardParametersTreeModel::onValueChanged(ParameterTreeHistoryItem* history)
{
	QModelIndex foundedIndex;
	if (findIndexRecursive(history, QModelIndex(), foundedIndex))
	{
		// foundedIndex - это индекс для колонки 0. 
		// Нам нужно создать индекс для колонки 1, где отображается значение.
		QModelIndex valueIndex = index(foundedIndex.row(), 1, foundedIndex.parent());

		// Испускаем сигнал только для ячейки со значением и только для роли ValueRole.
		// Это эффективнее, чем обновлять всю строку.
		emit dataChanged(valueIndex, valueIndex, { ValueRole });
	}
}
bool BoardParametersTreeModel::findIndexRecursive(ParameterTreeItem* item, QModelIndex parentIndex, QModelIndex& foundedIndex)
{
	if (static_cast<ParameterTreeItem*>(parentIndex.internalPointer()) == item)
	{
		foundedIndex = QModelIndex(parentIndex);

		return true;
	}
	for (auto i = 0; i < this->rowCount(parentIndex); i++)
	{
		auto nestedParentIndex = this->index(i, 0, parentIndex);

		if (findIndexRecursive(item, nestedParentIndex, foundedIndex))
		{
			return true;
		}
	}

	return false;
}

int BoardParametersTreeModel::columnCount(const QModelIndex& parent) const
{
	return 3;
}

int BoardParametersTreeModel::rowCount(const QModelIndex& parent) const
{
	ParameterTreeItem* parentItem;
	if (!parent.isValid())
	{
		parentItem = m_rootItem;
	}
	else
	{
		parentItem = static_cast<ParameterTreeItem*>(parent.internalPointer());
	}

	if (parentItem == nullptr)
		return 0;

	return parentItem->childCount();
}

QModelIndex BoardParametersTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	ParameterTreeItem* parentItem;
	if (!parent.isValid())
	{
		parentItem = m_rootItem;
	}
	else
	{
		parentItem = static_cast<ParameterTreeItem*>(parent.internalPointer());
	}

	if (parentItem == nullptr)
	{
		return QModelIndex();
	}

	auto childItem = parentItem->child(row);

	if (childItem)
	{
		return createIndex(row, column, childItem);
	}

	return QModelIndex();
}

QModelIndex BoardParametersTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return QModelIndex();

	auto childItem = static_cast<ParameterTreeItem*>(index.internalPointer());

	if (!childItem)
		return QModelIndex();

	auto parentItem = childItem->parentItem();

	if (!parentItem)
		return QModelIndex();
	if (!parentItem->parentItem())
		return QModelIndex();

	return createIndex(parentItem->row(), 0, parentItem);
}

QVariant BoardParametersTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	auto treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
	if (treeItem)
	{
		// ========== КОД ДЛЯ QT WIDGETS (QTreeView) ==========
		// Для работы с Qt Widgets: оставить этот блок активным
		// Для работы только с QML: закомментировать этот блок
		if (role == ParameterRole::ColorRole)
		{
			return treeItem->color();
		}
		if (role == ParameterRole::ChartVisibilityRole)
		{
			return treeItem->isChartVisible();
		}

		if (role == Qt::DecorationRole && index.column() == 0)
		{
			// Возвращаем иконку в зависимости от типа узла
			switch (treeItem->type())
			{
			case ParameterTreeItem::ItemType::Root:
				return QIcon(":/Resources/parameters_root_16.png");
			case ParameterTreeItem::ItemType::Group:
				return QIcon(":/Resources/parameters_root_16.png");
			case ParameterTreeItem::ItemType::Array:
				return QIcon(":/Resources/parameter_group_16.png");
			case ParameterTreeItem::ItemType::History:
				return QIcon(":/Resources/parameter_value_16_2.png");
			default:
				return QVariant();
			}
		}
		if (role == Qt::DisplayRole)
		{
			if (index.column() == 0)
			{
				return treeItem->label();
			}
			if (index.column() == 1)
			{
				if (treeItem->type() == ParameterTreeItem::ItemType::History)
				{
					auto* leafItem = static_cast<ParameterTreeHistoryItem*>(index.internalPointer());
					if (!leafItem || leafItem->values().isEmpty())
					{
						return QVariant();
					}

					const QString liveText = formatDisplayValue(leafItem->lastValue());
					if (!showScrubValue())
					{
						return leafItem->lastValue();
					}

					const QVariant scrubValue = leafItem->valueAtOrBefore(m_player->currentPosition());
					const QString scrubText = formatDisplayValue(scrubValue);
					if (scrubText.isEmpty())
					{
						// ASCII '-', не '—': иначе на MSVC без /utf-8 в UI кракозябры
						return QStringLiteral("%1 (-)").arg(liveText);
					}
					return QStringLiteral("%1 (%2)").arg(liveText, scrubText);
				}
				return QVariant();
			}
			if (index.column() == 2)
			{
				// Третья колонка - control, только для History элементов
				// Не возвращаем текст, так как там отображается виджет-контрол
				return QVariant();
			}
		}
		if (role == Qt::EditRole && index.column() == 2)
		{
			// Для редактирования возвращаем данные о контроле
			if (treeItem->type() == ParameterTreeItem::ItemType::History)
			{
				auto leafItem = static_cast<ParameterTreeHistoryItem*>(index.internalPointer());
				QVariantMap controlData;
				QString control = leafItem->control();
				controlData["control"] = control;
				controlData["min"] = leafItem->min();
				controlData["max"] = leafItem->max();
				QVariant lastValue = leafItem->values().last();
				controlData["value"] = lastValue;
				
				// Определяем тип значения для валидации
				QString valueType;
				if (lastValue.type() == QVariant::Int || lastValue.type() == QVariant::LongLong)
				{
					valueType = "int";
				}
				else if (lastValue.type() == QVariant::Double)
				{
					valueType = "double";
				}
				else if (lastValue.type() == QVariant::String)
				{
					valueType = "string";
				}
				else if (lastValue.type() == QVariant::Bool)
				{
					valueType = "bool";
				}
				controlData["valueType"] = valueType;
				
				qDebug() << "BoardParametersTreeModel::data - EditRole, column 2, control:" << control
						 << "min:" << leafItem->min() << "max:" << leafItem->max() << "valueType:" << valueType;
				
				return controlData;
			}
		}
		// =====================================================
		
		//// ========== КОД ДЛЯ QML ==========
		//// Для работы с QML: оставить этот блок активным
		//// Для работы только с Qt Widgets: закомментировать этот блок
		//switch (static_cast<ParameterRole>(role))
		//{
		//case ParameterRole::ColorRole:
		//	return treeItem->color();
		//case ParameterRole::ChartVisibilityRole:
		//	return m_chartVisibilities.value(index.row(), false);
		//case ParameterRole::LabelRole:
		//	return treeItem->label();
		//case ParameterRole::ValueRole:
		//	if (treeItem->type() == ParameterTreeItem::ItemType::History)
		//	{
		//		auto leafItem = static_cast<ParameterTreeHistoryItem*>(index.internalPointer());
		//		
		//		return leafItem->values().last();
		//	}
		//	else
		//	{
		//		return QVariant();
		//	}
		//default:
		//	break;
		//}
		//// ==================================
	}

	return QVariant();
}

bool BoardParametersTreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid()) return false;

	// Обновление значения параметра через контрол
	if (role == Qt::EditRole && index.column() == 2)
	{
		auto treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (treeItem && treeItem->type() == ParameterTreeItem::ItemType::History)
		{
			auto leafItem = static_cast<ParameterTreeHistoryItem*>(index.internalPointer());
			// Добавляем новое значение в историю
			leafItem->addValue(value, QDateTime::currentDateTime());
			
			// Обновляем отображение значения во второй колонке
			QModelIndex valueIndex = this->index(index.row(), 1, index.parent());
			emit dataChanged(valueIndex, valueIndex, { Qt::DisplayRole, ValueRole });
			
			return true;
		}
	}

	if (role == (int)ParameterRole::ChartVisibilityRole)
	{
		auto treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (treeItem)
		{
			treeItem->setIsChartVisible(value.toBool());
			// Маркер рисуется в колонке значений — обновляем обе ячейки строки
			const QModelIndex labelIndex = this->index(index.row(), 0, index.parent());
			const QModelIndex valueIndex = this->index(index.row(), 1, index.parent());
			emit dataChanged(labelIndex, valueIndex, { role });
			return true;
		}
	}

	return false;
}

QVariant BoardParametersTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	// Для Qt Widgets: возвращаем заголовки колонок
	// Для QML: этот метод не используется
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < 3)
		return m_horizontalHeaders[section];

	return QAbstractItemModel::headerData(section, orientation, role);
}

bool BoardParametersTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
	// Для Qt Widgets: устанавливаем заголовки колонок
	// Для QML: этот метод не используется
	if (orientation == Qt::Horizontal && role == Qt::EditRole && section < 3)
	{
		m_horizontalHeaders[section] = value;
		emit headerDataChanged(orientation, section, section);
		return true;
	}
	return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

QHash<int, QByteArray> BoardParametersTreeModel::roleNames() const
{
	QHash<int, QByteArray> rolesHash;
	rolesHash[(int)ParameterRole::LabelRole] = "label";
	rolesHash[(int)ParameterRole::ValueRole] = "value";
	return rolesHash;
}

void BoardParametersTreeModel::makeRandomColors()
{
	int hue = 0;
	int hueStep = 60 + 5;

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib_int(192, 255);
	for (auto i = 0; i < 200; i++)
	{
		auto sat = distrib_int(gen);
		auto val = distrib_int(gen);
		m_colors.append(QColor::fromHsv(hue, sat, val, 255));
		hue = (hue + hueStep) % 360;
	}
}

BoardParametersTreeModel::~BoardParametersTreeModel()
{
}
