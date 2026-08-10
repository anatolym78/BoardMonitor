#include "Session.h"
#include "./../Model/Parameters/Tree/ParameterTreeStorage.h"
#include "./../Model/Parameters/Tree/ParameterTreeItem.h"
#include <QDebug>
#include <functional>

Session::Session(QObject *parent)
	: QObject(parent)
	, m_treeStorage(new ParameterTreeStorage(this))
	, m_parametersModel(new BoardParametersTreeModel(parent))
	, m_chartsModel(new ChartsModel(parent))
	, m_player(nullptr)
	, m_opened(false)
{
	m_parametersSelectionModel = new QItemSelectionModel(m_parametersModel, this);
	m_chartsModel->setParameterTree(m_treeStorage);
}

bool Session::operator<(const Session& other) const
{
	return getCreatedAt() < other.getCreatedAt();
}

bool Session::operator==(const Session& other) const
{
	return getId() == other.getId() && getType() == other.getType();
}

BoardParametersTreeModel* Session::parametersModel() const
{
	return m_parametersModel;
}

void Session::updateMessageCount(int count)
{
	emit messageCountChanged(count);
}

void Session::updateParameterCount(int count)
{
	emit parameterCountChanged(count);
}

ParameterTreeStorage* Session::storage() const
{
	return m_treeStorage;
}

namespace {

void updateChartVisibilityForSelection(BoardParametersTreeModel* parametersModel,
	ChartsModel* chartsModel,
	const QModelIndex& currentIndex)
{
	std::function<void(const QModelIndex&)> updateVisibility;
	updateVisibility = [&](const QModelIndex& index)
	{
		auto item = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (!item)
		{
			return;
		}

		if (item->type() == ParameterTreeItem::ItemType::History)
		{
			const bool isVisible = chartsModel->hasSeries(item->fullName());
			parametersModel->setData(index, isVisible, BoardParametersTreeModel::ChartVisibilityRole);
		}

		const int rows = parametersModel->rowCount(index);
		for (int i = 0; i < rows; ++i)
		{
			updateVisibility(parametersModel->index(i, 0, index));
		}
	};

	updateVisibility(currentIndex);
}

void updateAllChartVisibility(BoardParametersTreeModel* parametersModel, ChartsModel* chartsModel)
{
	if (!parametersModel || !chartsModel)
	{
		return;
	}

	std::function<void(const QModelIndex&)> updateVisibility;
	updateVisibility = [&](const QModelIndex& index)
	{
		auto item = static_cast<ParameterTreeItem*>(index.internalPointer());
		if (!item)
		{
			return;
		}

		if (item->type() == ParameterTreeItem::ItemType::History)
		{
			const bool isVisible = chartsModel->hasSeries(item->fullName());
			parametersModel->setData(index, isVisible, BoardParametersTreeModel::ChartVisibilityRole);
		}

		const int rows = parametersModel->rowCount(index);
		for (int i = 0; i < rows; ++i)
		{
			updateVisibility(parametersModel->index(i, 0, index));
		}
	};

	const int rows = parametersModel->rowCount();
	for (int i = 0; i < rows; ++i)
	{
		updateVisibility(parametersModel->index(i, 0));
	}
}

ParameterTreeItem* selectedParameterTreeItem(QItemSelectionModel* selectionModel)
{
	if (!selectionModel)
	{
		return nullptr;
	}

	const QModelIndex currentIndex = selectionModel->currentIndex();
	if (!currentIndex.isValid())
	{
		return nullptr;
	}

	return static_cast<ParameterTreeItem*>(currentIndex.internalPointer());
}

} // namespace

void Session::showChartFromSelectedParameter()
{
	if (!m_parametersSelectionModel || !m_chartsModel)
	{
		qWarning() << "Session::showChartFromSelectedParameter: selectionModel or chartsModel is null";
		return;
	}

	const QModelIndex currentIndex = m_parametersSelectionModel->currentIndex();
	if (!currentIndex.isValid())
	{
		qWarning() << "Session::showChartFromSelectedParameter: no parameter selected";
		return;
	}

	auto* treeItem = selectedParameterTreeItem(m_parametersSelectionModel);
	if (!treeItem)
	{
		qWarning() << "Session::showChartFromSelectedParameter: treeItem is null";
		return;
	}

	if (treeItem->fullName().isEmpty())
	{
		qWarning() << "Session::showChartFromSelectedParameter: parameter label is empty";
		return;
	}

	if (m_chartsModel->isParameterDisplayed(treeItem))
	{
		return;
	}

	m_chartsModel->showParameter(treeItem);
	updateChartVisibilityForSelection(m_parametersModel, m_chartsModel, currentIndex);

	qDebug() << "Session::showChartFromSelectedParameter: added chart for parameter" << treeItem->fullName();
}

void Session::hideChartFromSelectedParameter()
{
	if (!m_parametersSelectionModel || !m_chartsModel)
	{
		qWarning() << "Session::hideChartFromSelectedParameter: selectionModel or chartsModel is null";
		return;
	}

	const QModelIndex currentIndex = m_parametersSelectionModel->currentIndex();
	if (!currentIndex.isValid())
	{
		qWarning() << "Session::hideChartFromSelectedParameter: no parameter selected";
		return;
	}

	auto* treeItem = selectedParameterTreeItem(m_parametersSelectionModel);
	if (!treeItem)
	{
		qWarning() << "Session::hideChartFromSelectedParameter: treeItem is null";
		return;
	}

	m_chartsModel->hideParameter(treeItem);
	updateChartVisibilityForSelection(m_parametersModel, m_chartsModel, currentIndex);

	qDebug() << "Session::hideChartFromSelectedParameter: removed chart series for parameter" << treeItem->fullName();
}

void Session::hideAllCharts()
{
	if (!m_chartsModel || !m_parametersModel)
	{
		return;
	}

	if (m_chartsModel->chartCount() == 0)
	{
		return;
	}

	m_chartsModel->clearAllCharts();
	updateAllChartVisibility(m_parametersModel, m_chartsModel);
}

void Session::toggleChartAtIndex(const QModelIndex& index)
{
	if (!m_chartsModel || !m_parametersModel || !index.isValid())
	{
		return;
	}

	auto* treeItem = static_cast<ParameterTreeItem*>(index.internalPointer());
	if (!treeItem || treeItem->fullName().isEmpty())
	{
		return;
	}

	if (m_parametersSelectionModel)
	{
		m_parametersSelectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
	}

	m_chartsModel->toggleParameter(treeItem);
	updateChartVisibilityForSelection(m_parametersModel, m_chartsModel, index);
}

Session::~Session()
{
	// Принудительно останавливаем и удаляем плеер ПЕРЕД автоматическим удалением children,
	// чтобы гарантировать, что поток плеера завершен до удаления m_treeStorage
	if (m_player)
	{
		m_player->stop();
		// Удаляем плеер явно (хотя он и QObject child, это безопасно)
		// Это гарантирует, что деструктор плеера вызовется и поток остановится
		// ДО того, как начнется удаление других children (включая m_treeStorage)
		delete m_player;
		m_player = nullptr;
	}
	// m_treeStorage и другие children удалятся автоматически после завершения деструктора
}