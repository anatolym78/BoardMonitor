#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

#include "Model/Parameters/Tree/ParameterTreeStorage.h"

class ParameterTreeJsonParser;

class TelemetryIngestWorker : public QObject
{
    Q_OBJECT

public:
    explicit TelemetryIngestWorker(QObject* parent = nullptr);
    ~TelemetryIngestWorker() override;

public slots:
    void initialize();
    void acceptPacket(const QString& json, const QDateTime& arrivalTimestamp);

signals:
    void snapshotReady(ParameterTreeStorage* snapshot);
    void parseError(const QString& message);

private:
    ParameterTreeJsonParser* m_parser = nullptr;
};
