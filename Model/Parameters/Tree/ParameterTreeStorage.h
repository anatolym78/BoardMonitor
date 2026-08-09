#ifndef PARAMETERTREESTORAGE_H
#define PARAMETERTREESTORAGE_H

#include "./ParameterTreeItem.h"
#include "./ParameterTreeGroupItem.h"
#include "./../../Parameters/BoardParameterSingle.h"
#include <QDateTime>
#include <QList>
#include <QMutex>

class ParameterTreeHistoryItem;

class ParameterTreeStorage : public ParameterTreeItem
{
	Q_OBJECT
public:
	explicit ParameterTreeStorage(QObject *parent = nullptr);
	ItemType type() const override { return ItemType::Root; }

	QList<ParameterTreeItem*> findPath(ParameterTreeHistoryItem* item) const;
	int topLevelItemIndex(ParameterTreeItem* item) const;
	ParameterTreeHistoryItem* findHistoryItemByFullName(const QString& fullName) const;

	ParameterTreeStorage* extractRange(const QDateTime& startTime, const QDateTime& endTime) const;
	ParameterTreeStorage* extractAfter(const QDateTime& after, const QDateTime& endTime) const;
	QDateTime latestTimestamp() const;
	void clear();

public slots:
	void appendSnapshot(ParameterTreeStorage* snapshot);
	void setSnapshot(ParameterTreeStorage* snapshot);

signals:
	void parameterAdded(ParameterTreeItem* newItem);
	void valueAdded(ParameterTreeHistoryItem* updatedItem);
	void valueChanged(ParameterTreeHistoryItem* updatedItem);

private:
	/** Сигналы нельзя слать под m_mutex: слоты часто снова берут этот же lock → deadlock. */
	struct PendingNotifications
	{
		QList<ParameterTreeItem*> parameterAdded;
		QList<ParameterTreeHistoryItem*> valueAdded;
		QList<ParameterTreeHistoryItem*> valueChanged;
	};

	void appendNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode, PendingNotifications& pending);
	void setNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode, PendingNotifications& pending);
	void flushNotifications(const PendingNotifications& pending);
	void extractNode(ParameterTreeItem* localParent, ParameterTreeItem* incomingNode, const QDateTime& startTime, const QDateTime& endTime, bool exclusiveLowerBound = false) const;
	void collectParameters(ParameterTreeItem* item, const QDateTime& startTime, const QDateTime& endTime, QList<BoardParameterSingle*>& params) const;

private:
	mutable QMutex m_mutex;
};

#endif // PARAMETERTREESTORAGE_H
