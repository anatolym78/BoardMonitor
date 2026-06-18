#ifndef CHATVIEWGRIDMODEL_H
#define CHATVIEWGRIDMODEL_H

#include <QAbstractListModel>
#include <QAbstractTableModel>
#include <QString>
#include <QList>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QDateTimeAxis>
#include <QDateTime>
#include <QVariant>
#include <QColor>

#include "./../Model/Parameters/Tree/ParameterTreeItem.h"
#include "./../Model/Parameters/Tree/ParameterTreeHistoryItem.h"
#include "./DataPlayer.h"

class ChatViewGridModel : public QAbstractListModel
{
	Q_OBJECT
		
	Q_PROPERTY(bool isCanMergeCharts READ isCanMergeCharts NOTIFY isCanMergeChartsChanged)

public:
	enum ChartViewRoles
	{
		LabelsRole = Qt::UserRole + 1,
		LabelRole,
		ChartIndexRole,
		DepthRole,
		SelectionRole,
		HoverRole,
		ColorRole,
		seriesMapRole,
	};
	struct ChartInfo
	{
		QString chartName;
		QStringList series;
		QColor color = Qt::darkGray;
		bool isSelected = false;
		QMap<QString, QPointer<QtCharts::QLineSeries>> seriesMap;
		QPointer<QtCharts::QDateTimeAxis> timeAxis = nullptr;
		QPointer<QtCharts::QValueAxis> valueAxis = nullptr;
		bool isAxesInitialized = false;
	};

	void toggleParameter(ParameterTreeItem* parameter);// QString chartName);
	void showParameter(ParameterTreeItem* parameter);
	void hideParameter(ParameterTreeItem* parameter);
	bool isParameterDisplayed(ParameterTreeItem* parameter) const;

	explicit ChatViewGridModel(QObject *parent = nullptr);

	void setPlayer(DataPlayer* dataPlayer);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

	// Методы для работы с графиками
	Q_INVOKABLE void toggleParameter(const QString& label, const QColor& color = Qt::red);
	Q_INVOKABLE void addSeries(const QString &label, const QColor& color = Qt::red);
	Q_INVOKABLE void removeSeries(const QString& label);
	Q_INVOKABLE void clearCharts();
	Q_INVOKABLE QStringList getChartSeriesLabels(int chartIndex) const;

	void mergeCharts();
	/// <summary>
	/// Вызывается по кнопке объединения графиков
	/// Подготавливает названия серий, который должны быть перемещены
	/// Вызывает сигнал, обрабатываемый на стороне qml, в который передеются
	/// индекс графика, в которй нужно переместить серии и названия этих серий
	/// </summary>
	/// <returns></returns>
	Q_INVOKABLE void mergeSelectedCharts();
	QList<int> selectedIndices() const;
	int countSelectedIndices() const;
	int countEmptyCharts() const;
	int firstEmptyChart() const;
	Q_INVOKABLE void splitSeries(int chartIndex);
	void removeEmptyCharts();
	
	Q_INVOKABLE QStringList chartLabels() const;

	Q_INVOKABLE bool hasSeries(const QString &label) const;
	Q_INVOKABLE int countSeries() const { return m_charts.count(); }
	Q_INVOKABLE bool isEmpty() const { return m_charts.isEmpty(); }

	Q_INVOKABLE bool selectElement(int index, bool keepSelection);
	Q_INVOKABLE void clearSelection();

	Q_INVOKABLE bool hoverElement(int index);
	Q_INVOKABLE void clearHover();

	Q_INVOKABLE bool isCanMergeCharts() const;

	// new (move charts logic to c++ code)
	Q_INVOKABLE void addSeriesToChart(int chartIndex, const QString& label, const QColor& color, QtCharts::QLineSeries* series, QtCharts::QDateTimeAxis* timeAxis, QtCharts::QValueAxis* valueAxis);
	Q_INVOKABLE void moveSeriesToChart(int chartIndex, const QString& label, QtCharts::QLineSeries* series);
	
	Q_INVOKABLE bool isSeriesCreated(const QString& label) const;

	void updateAllCells();

	void fillSeries(const QString& label, QColor color, bool isInitialFill);
	QColor labelColor(QString label);

	int findChartIndex(const QString& label) const;
	bool hasChart(const QString& label) const { return findChartIndex(label) != -1; }

	// Вспомогательная функция для фильтрации данных за пределами диапазона
	void filterDataOutsideRange(
		const QList<QDateTime>& times, 
		const QList<QVariant>& values, 
		const QDateTime& seriesStartTime, 
		const QDateTime& seriesEndTime, 
		bool isBackPlaying,
		QList<QDateTime>& outTimes, 
		QList<QVariant>& outValues);
	
signals:
	void parameterAdded(int chartIndex, ParameterTreeItem* parameter);
	void parametersNeedToMove(int chartIndex, QStringList labels);
	void parameterNeedToRemove(int chartIndex, const QString& label);
	void isCanMergeChartsChanged();

private:
	DataPlayer* m_dataPlayer = nullptr;
	QMetaObject::Connection m_playConnection;
	//QMetaObject::Connection m_stopConnection;
	QList<ChartInfo> m_charts;
	//QList<QStringList> m_series;
	//QList<int> m_depths;
	//QList<bool> m_selectedIndices;
	int m_hoverIndex = -1;

	// Вспомогательные методы
	bool parameterExistsInHistory(const QString &label) const;
	void updateValueAxisRange(int chartIndex);
	void collectHistoryItems(ParameterTreeItem* item, QList<ParameterTreeHistoryItem*>& out) const;

private:
	DataPlayer* player() const { return m_dataPlayer; }
private:
	void onPlayed(ParameterTreeStorage* snapshot, bool isBackPlaying);
	void updateSeries(const QString& label, ParameterTreeHistoryItem* data, bool isBackPlaying);
	const qint64 minuteIntervalMsec() { return 15 * 1000; }
};

#endif // CHATVIEWGRIDMODEL_H
