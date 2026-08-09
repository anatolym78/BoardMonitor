#ifndef DRIVERDATAPLAYER_H
#define DRIVERDATAPLAYER_H

#include "DataPlayer.h"

#include <QDateTime>
#include <QTimer>

class DriverDataPlayer : public DataPlayer
{
	Q_OBJECT

public:
	explicit DriverDataPlayer(QObject *parent = nullptr);
	~DriverDataPlayer();

	void setRefreshPaused(bool paused);

	/** Последнее время данных с борта (playhead на паузе); не путать с currentPosition. */
	QDateTime dataHeadTime() const { return m_dataHeadTime; }
	double dataHeadElapsed() const;

	Q_INVOKABLE void play() override;
	Q_INVOKABLE void stop() override;
	Q_INVOKABLE void pause() override;
	Q_INVOKABLE void setPosition(QDateTime position) override;

	void setStorage(ParameterTreeStorage* storage) override;

	void resetState() override;
	void initialPlay() override {}

	Q_INVOKABLE void moveToBegin() override;
	Q_INVOKABLE void reset() override;

signals:
	/** Playhead live-сессии сдвинулся (в т.ч. на паузе, когда currentPosition заморожен). */
	void dataHeadChanged();

protected:
	void startPlayback() override;

private:
	void scheduleRefresh();
	void flushRefresh();
	void extendTimeRangeTo(const QDateTime& latestTimestamp);
	void emitTimeRangeSignals();
	void updateDataHead(const QDateTime& timestamp);

	void onStorageValueAdded(ParameterTreeHistoryItem* historyItem);

private:
	bool m_isInitialized = false;
	bool m_refreshPaused = false;
	QTimer* m_refreshTimer = nullptr;
	QDateTime m_lastConsumedTimestamp;
	QDateTime m_dataHeadTime;

	static constexpr int REFRESH_INTERVAL_MS = 50;
	static constexpr double TIME_RANGE = 600.0;
};

#endif // DRIVERDATAPLAYER_H
