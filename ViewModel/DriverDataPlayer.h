#ifndef DRIVERDATAPLAYER_H
#define DRIVERDATAPLAYER_H

#include "DataPlayer.h"

class DriverDataPlayer : public DataPlayer
{
	Q_OBJECT

public:
	explicit DriverDataPlayer(QObject *parent = nullptr);
	~DriverDataPlayer();

	// Методы управления
	Q_INVOKABLE void play() override;
	Q_INVOKABLE void stop() override;
	Q_INVOKABLE void pause() override;
	Q_INVOKABLE void setPosition(QDateTime position) override;
	
	// Методы для работы с хранилищем
	void setStorage(ParameterTreeStorage* storage) override;
		
	// Метод для сброса состояния плеера
	void resetState() override;
	void initialPlay() override {}

	Q_INVOKABLE void moveToBegin() override;
    Q_INVOKABLE void reset() override;

private:
	void updatePlaybackPosition() override;
	void extendTimeRange();
	void initializeTimeRange();
	void checkAndPlayParameters();
	void emitTimeRangeSignals();

private:
	void onStorageValueAdded(ParameterTreeHistoryItem* historyItem);

private:
	bool m_isInitialized;
	static const int TIME_RANGE_MINUTES = 1; // Расширение диапазона на 10 минут
	double TIME_RANGE = 600.0;
};

#endif // DRIVERDATAPLAYER_H
