#include "TelemetryIngestService.h"
#include "TelemetryIngestWorker.h"

#include <QThread>
#include <QMetaObject>

TelemetryIngestService::TelemetryIngestService(QObject* parent)
    : QObject(parent)
{
}

TelemetryIngestService::~TelemetryIngestService()
{
    stop();
}

void TelemetryIngestService::start()
{
    if (m_started)
    {
        return;
    }

    m_thread = new QThread(this);
    m_worker = new TelemetryIngestWorker();
    m_worker->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_worker, &TelemetryIngestWorker::initialize);
    connect(m_worker, &TelemetryIngestWorker::snapshotReady, this, &TelemetryIngestService::snapshotReady);
    connect(m_worker, &TelemetryIngestWorker::parseError, this, &TelemetryIngestService::parseError);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_thread->start();
    m_started = true;
}

void TelemetryIngestService::stop()
{
    if (!m_started)
    {
        return;
    }

    m_thread->quit();
    m_thread->wait();
    m_worker = nullptr;
    m_started = false;
}

void TelemetryIngestService::enqueue(const QString& json)
{
    if (!m_started || !m_worker)
    {
        return;
    }

    const quint64 sequence = ++m_packetSequence;
    if (!m_anchorSet)
    {
        m_sessionAnchor = QDateTime::currentDateTime();
        m_anchorSet = true;
    }

    const QDateTime packetTimestamp = m_sessionAnchor.addMSecs(
        static_cast<qint64>(sequence - 1) * PACKET_INTERVAL_MS);

    QMetaObject::invokeMethod(
        m_worker,
        "acceptPacket",
        Qt::QueuedConnection,
        Q_ARG(QString, json),
        Q_ARG(QDateTime, packetTimestamp));

    emit packetEnqueued();
}

void TelemetryIngestService::resetSessionClock()
{
    m_packetSequence = 0;
    m_anchorSet = false;
}
