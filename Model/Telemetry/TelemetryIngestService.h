#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

#include "Model/Parameters/Tree/ParameterTreeStorage.h"

class QThread;
class TelemetryIngestWorker;

class TelemetryIngestService : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryIngestService(QObject* parent = nullptr);
    ~TelemetryIngestService() override;

    void start();
    void stop();

    void enqueue(const QString& json);
    void resetSessionClock();

signals:
    void snapshotReady(ParameterTreeStorage* snapshot);
    void packetEnqueued();
    void parseError(const QString& message);

private:
    QThread* m_thread = nullptr;
    TelemetryIngestWorker* m_worker = nullptr;
    bool m_started = false;
    quint64 m_packetSequence = 0;
    QDateTime m_sessionAnchor;
    bool m_anchorSet = false;

    static constexpr int PACKET_INTERVAL_MS = 33;
};
