#include "ParameterTreeStorage.h"
#include "ParameterTreeHistoryItem.h"
#include "ParameterTreeGroupItem.h"
#include "ParameterTreeArrayItem.h"
#include <QDebug>
#include <QMutexLocker>
#include <functional>

ParameterTreeStorage::ParameterTreeStorage(QObject *parent)
	: ParameterTreeItem("root", nullptr)
{
	setParent(parent);
}

QList<ParameterTreeItem*> ParameterTreeStorage::findPath(ParameterTreeHistoryItem* item) const
{
	QMutexLocker locker(&m_mutex);
	QList<ParameterTreeItem*> path{ item };
	ParameterTreeItem* currentItem = item;
	// перебираем узлы снизу вверх до корневого (хранилище)
	while (currentItem->parentItem() != nullptr)
	{
		currentItem = currentItem->parentItem();

		path.append(currentItem);
	}

	// узел не принадлежит хранилищу
	if (currentItem != this) return QList<ParameterTreeItem*>();

	// интвертируем порядок
	// теперь элементы расположены от верхнего (данное хранилище) к нижнему (входной параметер)
	std::reverse(path.begin(), path.end());

	return path;
}

ParameterTreeStorage* ParameterTreeStorage::extractRange(const QDateTime& startTime, const QDateTime& endTime) const
{
	QMutexLocker locker(&m_mutex);
	auto subStorage = new ParameterTreeStorage();

	for (ParameterTreeItem* child : m_childItems)
	{
		extractNode(subStorage, child, startTime, endTime, false);
	}

	return subStorage;
}

ParameterTreeStorage* ParameterTreeStorage::extractAfter(const QDateTime& after, const QDateTime& endTime) const
{
	QMutexLocker locker(&m_mutex);
	auto subStorage = new ParameterTreeStorage();

	for (ParameterTreeItem* child : m_childItems)
	{
		extractNode(subStorage, child, after, endTime, true);
	}

	return subStorage;
}

QDateTime ParameterTreeStorage::latestTimestamp() const
{
	QMutexLocker locker(&m_mutex);

	QDateTime latest;
	std::function<void(ParameterTreeItem*)> visit = [&](ParameterTreeItem* node)
	{
		if (!node)
		{
			return;
		}

		if (node->type() == ItemType::History)
		{
			const auto* history = static_cast<const ParameterTreeHistoryItem*>(node);
			const QDateTime ts = history->lastTimestamp();
			if (ts.isValid() && (latest.isNull() || ts > latest))
			{
				latest = ts;
			}
			return;
		}

		for (ParameterTreeItem* child : node->children())
		{
			visit(child);
		}
	};

	for (ParameterTreeItem* child : m_childItems)
	{
		visit(child);
	}

	return latest;
}

void ParameterTreeStorage::clear()
{
	QMutexLocker locker(&m_mutex);
	qDeleteAll(m_childItems);
	m_childItems.clear();
}

int ParameterTreeStorage::topLevelItemIndex(ParameterTreeItem* item) const
{
	QMutexLocker locker(&m_mutex);
	for (auto i = 0; i < m_childItems.count(); i++)
	{
		if (m_childItems[i] == item) return i;
	}

	return -1;
}

ParameterTreeHistoryItem* ParameterTreeStorage::findHistoryItemByFullName(const QString& fullName) const
{
	QMutexLocker locker(&m_mutex);
	
	if (fullName.isEmpty())
	{
		return nullptr;
	}
	
	QStringList pathParts = fullName.split('.');
	pathParts.removeAll(QString()); // Удаляем пустые строки
	if (pathParts.isEmpty())
	{
		return nullptr;
	}
	
	ParameterTreeItem* currentItem = const_cast<ParameterTreeStorage*>(this);
	
	// Проходим по пути от корня до конечного узла
	for (int i = 0; i < pathParts.size(); ++i)
	{
		const QString& part = pathParts[i];
		ParameterTreeItem* foundChild = currentItem->findChildByLabel(part);
		
		if (!foundChild)
		{
			return nullptr;
		}
		
		// Если это последняя часть пути, проверяем, что это History узел
		if (i == pathParts.size() - 1)
		{
			if (foundChild->type() == ItemType::History)
			{
				return static_cast<ParameterTreeHistoryItem*>(foundChild);
			}
			return nullptr;
		}
		
		// Продолжаем поиск в следующем уровне
		currentItem = foundChild;
	}
	
	return nullptr;
}

void ParameterTreeStorage::appendSnapshot(ParameterTreeStorage* snapshot)
{
	QMutexLocker locker(&m_mutex);
	if (!snapshot)
	{
		return;
	}

	for (ParameterTreeItem* incomingChild : snapshot->children())
	{
		appendNode(this, incomingChild);
	}

	// Снимок - это временный объект, который должен быть удален
	snapshot->deleteLater();
}

void ParameterTreeStorage::setSnapshot(ParameterTreeStorage* snapshot)
{
	QMutexLocker locker(&m_mutex);
	if (!snapshot)
	{
		return;
	}

	for (ParameterTreeItem* incomingChild : snapshot->children())
	{
		setNode(this, incomingChild);
	}

	// Снимок - это временный объект, который должен быть удален
	snapshot->deleteLater();
}

void ParameterTreeStorage::appendNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode)
{
	ParameterTreeItem* existingNode = localParent->findChildByLabel(incomingNode->label());

	if (!existingNode)
	{
		// Узел не существует, создаем новый и копируем данные
		if (incomingNode->type() == ItemType::History)
		{
			auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);
			auto newHistoryItem = new ParameterTreeHistoryItem(incomingHistory->label(), localParent);

			const auto& values = incomingHistory->values();
			const auto& timestamps = incomingHistory->timestamps();
			for(int i = 0; i < values.size(); ++i) {
				newHistoryItem->addValue(values[i], timestamps[i]);
			}

			// Копируем поля control, min, max
			newHistoryItem->setControl(incomingHistory->control());
			newHistoryItem->setMin(incomingHistory->min());
			newHistoryItem->setMax(incomingHistory->max());

			localParent->appendChild(newHistoryItem);
			emit parameterAdded(newHistoryItem);
			if (!values.isEmpty())
			{
				emit valueAdded(newHistoryItem);
			}
		}
		else if (incomingNode->type() == ItemType::Group)
		{
			auto newGroupItem = new ParameterTreeGroupItem(incomingNode->label(), localParent);
			localParent->appendChild(newGroupItem);
			emit parameterAdded(newGroupItem);

			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				appendNode(newGroupItem, incomingChild);
			}
		}
		else if (incomingNode->type() == ItemType::Array)
		{
			auto newArrayItem = new ParameterTreeArrayItem(incomingNode->label(), localParent);
			localParent->appendChild(newArrayItem);
			emit parameterAdded(newArrayItem);

			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				appendNode(newArrayItem, incomingChild);
			}
		}
	}
	else
	{
		// Узел существует
		if (existingNode->type() == ItemType::History && incomingNode->type() == ItemType::History)
		{
			auto existingHistory = static_cast<ParameterTreeHistoryItem*>(existingNode);
			auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);

			const auto& values = incomingHistory->values();
			const auto& timestamps = incomingHistory->timestamps();
			bool hasNewValues = false;
			for(int i = 0; i < values.size(); ++i)
			{
				existingHistory->addValue(values[i], timestamps[i]);
				hasNewValues = true;
			}

			// Обновляем поля control, min, max если они заданы во входящем узле
			if (!incomingHistory->control().isEmpty())
			{
				existingHistory->setControl(incomingHistory->control());
			}
			if (incomingHistory->min().isValid())
			{
				existingHistory->setMin(incomingHistory->min());
			}
			if (incomingHistory->max().isValid())
			{
				existingHistory->setMax(incomingHistory->max());
			}

			if (hasNewValues) 
			{
				 emit valueAdded(existingHistory);
			}
		}
		else if (existingNode->type() == ItemType::Group && incomingNode->type() == ItemType::Group)
		{
			for (ParameterTreeItem* incomingChild : incomingNode->children())
			{
				appendNode(existingNode, incomingChild);
			}
		}
		else if (existingNode->type() == ItemType::Array && incomingNode->type() == ItemType::Array)
		{
			for (ParameterTreeItem* incomingChild : incomingNode->children())
			{
				appendNode(existingNode, incomingChild);
			}
		}
		else
		{
			// Типы не совпадают - нужно пересоздать узел с правильным типом
			// Удаляем существующий узел
			localParent->removeChild(existingNode);
			existingNode->deleteLater();
			
			// Создаем новый узел с правильным типом
			if (incomingNode->type() == ItemType::History)
			{
				auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);
				auto newHistoryItem = new ParameterTreeHistoryItem(incomingHistory->label(), localParent);

				const auto& values = incomingHistory->values();
				const auto& timestamps = incomingHistory->timestamps();
				for(int i = 0; i < values.size(); ++i) {
					newHistoryItem->addValue(values[i], timestamps[i]);
				}

				localParent->appendChild(newHistoryItem);
				emit parameterAdded(newHistoryItem);
				if (!values.isEmpty())
				{
					emit valueAdded(newHistoryItem);
				}
			}
			else if (incomingNode->type() == ItemType::Group)
			{
				auto newGroupItem = new ParameterTreeGroupItem(incomingNode->label(), localParent);
				localParent->appendChild(newGroupItem);
				emit parameterAdded(newGroupItem);

				for(ParameterTreeItem* incomingChild : incomingNode->children())
				{
					appendNode(newGroupItem, incomingChild);
				}
			}
			else if (incomingNode->type() == ItemType::Array)
			{
				auto newArrayItem = new ParameterTreeArrayItem(incomingNode->label(), localParent);
				localParent->appendChild(newArrayItem);
				emit parameterAdded(newArrayItem);

				for(ParameterTreeItem* incomingChild : incomingNode->children())
				{
					appendNode(newArrayItem, incomingChild);
				}
			}
		}
	}
}

void ParameterTreeStorage::setNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode)
{
	ParameterTreeItem* existingNode = localParent->findChildByLabel(incomingNode->label());

	if (!existingNode)
	{
		// Узел не существует, создаем новый и копируем данные
		if (incomingNode->type() == ItemType::History)
		{
			auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);
			auto newHistoryItem = new ParameterTreeHistoryItem(incomingHistory->label(), localParent);

			newHistoryItem->setValues(incomingHistory->values(), incomingHistory->timestamps());

			// Копируем поля control, min, max
			newHistoryItem->setControl(incomingHistory->control());
			newHistoryItem->setMin(incomingHistory->min());
			newHistoryItem->setMax(incomingHistory->max());

			localParent->appendChild(newHistoryItem);
			emit parameterAdded(newHistoryItem);
			if (!incomingHistory->values().isEmpty())
			{
				emit valueAdded(newHistoryItem);
			}
		}
		else if (incomingNode->type() == ItemType::Group)
		{
			auto newGroupItem = new ParameterTreeGroupItem(incomingNode->label(), localParent);
			localParent->appendChild(newGroupItem);
			emit parameterAdded(newGroupItem);

			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				setNode(newGroupItem, incomingChild);
			}
		}
		else if (incomingNode->type() == ItemType::Array)
		{
			auto newArrayItem = new ParameterTreeArrayItem(incomingNode->label(), localParent);
			localParent->appendChild(newArrayItem);
			emit parameterAdded(newArrayItem);

			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				setNode(newArrayItem, incomingChild);
			}
		}
	}
	else
	{
		// Узел существует
		if (existingNode->type() == ItemType::History && incomingNode->type() == ItemType::History)
		{
			auto existingHistory = static_cast<ParameterTreeHistoryItem*>(existingNode);
			auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);

			existingHistory->setValues(incomingHistory->values(), incomingHistory->timestamps());

			// Обновляем поля control, min, max если они заданы во входящем узле
			if (!incomingHistory->control().isEmpty())
			{
				existingHistory->setControl(incomingHistory->control());
			}
			if (incomingHistory->min().isValid())
			{
				existingHistory->setMin(incomingHistory->min());
			}
			if (incomingHistory->max().isValid())
			{
				existingHistory->setMax(incomingHistory->max());
			}

			emit valueChanged(existingHistory);
		}
		else if (existingNode->type() == ItemType::Group && incomingNode->type() == ItemType::Group)
		{
			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				setNode(existingNode, incomingChild);
			}
		}
		else if (existingNode->type() == ItemType::Array && incomingNode->type() == ItemType::Array)
		{
			for(ParameterTreeItem* incomingChild : incomingNode->children())
			{
				setNode(existingNode, incomingChild);
			}
		}
		else
		{
			// Типы не совпадают - нужно пересоздать узел с правильным типом
			// Удаляем существующий узел
			localParent->removeChild(existingNode);
			existingNode->deleteLater();
			
			// Создаем новый узел с правильным типом
			if (incomingNode->type() == ItemType::History)
			{
				auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);
				auto newHistoryItem = new ParameterTreeHistoryItem(incomingHistory->label(), localParent);

				newHistoryItem->setValues(incomingHistory->values(), incomingHistory->timestamps());

				localParent->appendChild(newHistoryItem);
				emit parameterAdded(newHistoryItem);
				if (!incomingHistory->values().isEmpty())
				{
					emit valueAdded(newHistoryItem);
				}
			}
			else if (incomingNode->type() == ItemType::Group)
			{
				auto newGroupItem = new ParameterTreeGroupItem(incomingNode->label(), localParent);
				localParent->appendChild(newGroupItem);
				emit parameterAdded(newGroupItem);

				for(ParameterTreeItem* incomingChild : incomingNode->children())
				{
					setNode(newGroupItem, incomingChild);
				}
			}
			else if (incomingNode->type() == ItemType::Array)
			{
				auto newArrayItem = new ParameterTreeArrayItem(incomingNode->label(), localParent);
				localParent->appendChild(newArrayItem);
				emit parameterAdded(newArrayItem);

				for(ParameterTreeItem* incomingChild : incomingNode->children())
				{
					setNode(newArrayItem, incomingChild);
				}
			}
		}
	}
}

void ParameterTreeStorage::extractNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode, const QDateTime& startTime, const QDateTime& endTime, bool exclusiveLowerBound) const
{
	if (incomingNode->type() == ItemType::History)
	{
		auto incomingHistory = static_cast<ParameterTreeHistoryItem*>(incomingNode);
		auto newHistoryItem = new ParameterTreeHistoryItem(incomingHistory->label(), localParent);

		const auto& values = incomingHistory->values();
		const auto& timestamps = incomingHistory->timestamps();
		bool hasValuesInRange = false;
		for (int i = 0; i < values.size(); ++i)
		{
			const bool inRange = exclusiveLowerBound
				? (timestamps[i] > startTime && timestamps[i] <= endTime)
				: (timestamps[i] >= startTime && timestamps[i] <= endTime);
			if (inRange)
			{
				newHistoryItem->addValue(values[i], timestamps[i]);
				hasValuesInRange = true;
			}
		}

		if (hasValuesInRange)
		{
			localParent->appendChild(newHistoryItem);
		}
		else
		{
			delete newHistoryItem;
		}
	}
	else if (incomingNode->type() == ItemType::Group || incomingNode->type() == ItemType::Array)
	{
		// Определяем тип узла: если все дочерние элементы имеют числовые метки, это Array
		bool isArray = true;
		const auto& children = incomingNode->children();
		if (!children.isEmpty())
		{
			for (ParameterTreeItem* child : children)
			{
				bool ok;
				child->label().toInt(&ok);
				if (!ok)
				{
					isArray = false;
					break;
				}
			}
		}
		else
		{
			// Если нет дочерних элементов, используем исходный тип узла
			isArray = (incomingNode->type() == ItemType::Array);
		}

		ParameterTreeItem* newItem = nullptr;
		if (isArray)
		{
			newItem = new ParameterTreeArrayItem(incomingNode->label(), localParent);
		}
		else
		{
			newItem = new ParameterTreeGroupItem(incomingNode->label(), localParent);
		}

		for (ParameterTreeItem* incomingChild : incomingNode->children())
		{
			extractNode(newItem, incomingChild, startTime, endTime, exclusiveLowerBound);
		}

		if (newItem->childCount() > 0)
		{
			localParent->appendChild(newItem);
		}
		else
		{
			delete newItem;
		}
	}
}

