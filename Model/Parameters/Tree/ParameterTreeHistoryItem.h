#ifndef PARAMETERTREEHISTORYITEM_H
#define PARAMETERTREEHISTORYITEM_H

#include "ParameterTreeItem.h"
#include <QVariant>
#include <QDateTime>
#include <QList>

class ParameterTreeHistoryItem : public ParameterTreeItem
{
    Q_OBJECT

public:
    explicit ParameterTreeHistoryItem(const QString &label, ParameterTreeItem *parent = nullptr);
    ItemType type() const override { return ItemType::History; }

    void addValue(const QVariant &value, const QDateTime &timestamp);
    void setValues(const QList<QVariant>& values, const QList<QDateTime>& timestamps);
    QVariant lastValue() const;

    const QList<QVariant>& values() const;
    const QList<QDateTime>& timestamps() const;

    const QDateTime& lastTimestamp() const;

    QString control() const;
    void setControl(const QString &control);

    QVariant min() const;
    void setMin(const QVariant &min);

    QVariant max() const;
    void setMax(const QVariant &max);

private:
    QList<QVariant> m_values;
    QList<QDateTime> m_timestamps;
    QString m_control;
    QVariant m_min;
    QVariant m_max;
};

#endif // PARAMETERTREEHISTORYITEM_H
