#ifndef SESSION_H
#define SESSION_H

#include <QObject>
#include <QDateTime>
#include <QString>

#include "./DataPlayer.h"
#include "./BoardParametersTreeModel.h"
#include "./ChartsModel.h"

class BoardParameterHistoryStorage;
class ParameterTreeStorage;

/**
 * @brief Базовый класс для представления сессии
 */
class Session : public QObject
{
	Q_OBJECT

public:
	enum SessionType
	{
		RecordedSession,
		LiveSession
	};
	ParameterTreeStorage* storage() const;
	DataPlayer* player() const { return m_player; }
	BoardParametersTreeModel* parametersModel()  const;
	QItemSelectionModel* parametersSelectionModel() const { return m_parametersSelectionModel; }
	ChartsModel* chartsModel() const { return m_chartsModel; }

	explicit Session(QObject *parent = nullptr);
	virtual ~Session();

	// Виртуальные методы для получения информации о сессии
	virtual int getId() const = 0;
	virtual QString getName() const = 0;
	virtual QDateTime getCreatedAt() const = 0;
	virtual QString getDescription() const = 0;
	virtual int getMessageCount() const = 0;
	virtual int getParameterCount() const = 0;
	virtual SessionType getType() const = 0;
	virtual bool isRecording() const = 0;


	// Методы для сравнения сессий (для сортировки)
	virtual bool operator<(const Session& other) const;
	virtual bool operator==(const Session& other) const;


	virtual void open() { m_opened = true; }
	// Методы для работы с хранилищем
	virtual void clearStorage() = 0;

	bool isOpened() const { return m_opened; }

	void showChartFromSelectedParameter();
	void hideChartFromSelectedParameter();

signals:
	void sessionChanged();
	void messageCountChanged(int count);
	void parameterCountChanged(int count);

protected:
	// Защищенные методы для обновления данных
	void updateMessageCount(int count);
	void updateParameterCount(int count);

protected:
	BoardParametersTreeModel* m_parametersModel;
	QItemSelectionModel* m_parametersSelectionModel;
	ChartsModel* m_chartsModel;
	DataPlayer* m_player;
	ParameterTreeStorage* m_treeStorage;
	bool m_opened;

};

#endif // SESSION_H
