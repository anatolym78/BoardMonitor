#include "TelemetryIngestWorker.h"
#include "Model/Parameters/Tree/ParameterTreeJsonParser.h"
#include "Model/Parameters/Tree/ParameterTreeStorage.h"

#include <QDebug>

TelemetryIngestWorker::TelemetryIngestWorker(QObject* parent)
    : QObject(parent)
{
}

TelemetryIngestWorker::~TelemetryIngestWorker()
{
    delete m_parser;
    m_parser = nullptr;
}

void TelemetryIngestWorker::initialize()
{
    m_parser = new ParameterTreeJsonParser();
}

void TelemetryIngestWorker::acceptPacket(const QString& json, const QDateTime& arrivalTimestamp)
{
    if (!m_parser)
    {
        emit parseError(QStringLiteral("Telemetry ingest worker is not initialized"));
        return;
    }

    ParameterTreeStorage* snapshot = m_parser->parseJson(json, arrivalTimestamp);
    const QString error = m_parser->getLastError();
    if (!snapshot || !error.isEmpty())
    {
        emit parseError(error.isEmpty()
            ? QStringLiteral("Failed to parse telemetry JSON packet")
            : error);
        delete snapshot;
        return;
    }

    emit snapshotReady(snapshot);
}
