#ifndef PARAMETERTREEJSONPARSER_H
#define PARAMETERTREEJSONPARSER_H

#include <QObject>
#include <QVariant>
#include <QDateTime>

class QJsonDocument;
class QJsonObject;
class QJsonValue;
class ParameterTreeStorage;
class ParameterTreeItem;


class ParameterTreeJsonParser : public QObject
{
    Q_OBJECT

public:
    explicit ParameterTreeJsonParser(QObject *parent = nullptr);

    ParameterTreeStorage* parseJson(const QString &jsonString);
    ParameterTreeStorage* parseJson(const QString &jsonString, const QDateTime &snapshotTimestamp);
    QString toJson(ParameterTreeStorage* root);
    QString toBoardJson(ParameterTreeStorage* root);  // Группированный формат для отправки на борт
    void updateJson(const QString &jsonString, ParameterTreeStorage *root);
    void updateJsonFromArray(const QJsonArray &jsonArray, ParameterTreeStorage *root);

    QString getLastError() const;

    /** Сбрасывает привязку бортовых часов к настенному времени: вызывать при старте новой сессии. */
    void resetBoardClock();

private:
    bool extractBoardTimestampMs(const QJsonObject &obj, qint64 &boardMs) const;
    QDateTime resolveSnapshotTime(qint64 rawBoardMs, const QDateTime &arrivalTimestamp);

    void processJsonObject(const QJsonObject &jsonObject, ParameterTreeItem *parent);
    void updateJsonFromGroupedObject(const QJsonObject &obj, ParameterTreeStorage *root);
    void processValue(const QString &key, const QJsonValue &value, ParameterTreeItem *parent, 
                      const QString &control = QString(), const QVariant &min = QVariant(), const QVariant &max = QVariant());
    QVariant convertJsonValue(const QJsonValue &jsonValue);
    
    void serializeTreeItem(ParameterTreeItem* item, QJsonArray& jsonArray);
    void serializeBoardGroupChildren(ParameterTreeItem* group, QJsonArray& groupArray);
    void appendBoardParameterItem(ParameterTreeItem* item, QJsonArray& groupArray);
    QJsonValue convertVariantToJson(const QVariant& variant);

    QString m_lastError;
    QDateTime m_snapshotTimestamp;

    // Привязка бортового счётчика (int32, миллисекунды от включения борта) к настенному времени
    bool m_boardClockAnchored = false;
    QDateTime m_boardWallAnchor;
    QDateTime m_lastResolvedSnapshot;
    qint64 m_boardAnchorMs = 0;
    qint64 m_lastRawBoardMs = 0;
    qint64 m_boardWrapOffsetMs = 0;
};

#endif // PARAMETERTREEJSONPARSER_H
