#include "DebugViewModel.h"
#include <QDebug>
#include <QIcon>

DebugViewModel::DebugViewModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	// Инициализируем иконки для всех типов сообщений одной иконкой (как для Info)
	QIcon defaultIcon(":/Resources/info_16.png");
	m_typeIcons[MessageType::UplinkParameters] = defaultIcon;
	m_typeIcons[MessageType::DataReceived] = defaultIcon;
	m_typeIcons[MessageType::Error] = defaultIcon;
	m_typeIcons[MessageType::Info] = defaultIcon;
}

int DebugViewModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return m_messages.size();
}

int DebugViewModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	return 3; // Тип, Сообщение, Время
}

QVariant DebugViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
		return QVariant();
	
	switch (section)
	{
	case TypeColumn:
		return tr("Тип");
	case MessageColumn:
		return tr("Сообщение");
	case TimeColumn:
		return tr("Время");
	default:
		return QVariant();
	}
}


QHash<int, QByteArray> DebugViewModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[(int)DebugRole::TimestampRole] = "timestamp";
	roles[(int)DebugRole::MessageRole] = "message";
	roles[(int)DebugRole::TypeRole] = "messageType";
	roles[(int)DebugRole::FormattedMessageRole] = "formattedMessage";
	return roles;
}

QVariant DebugViewModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || index.row() >= m_messages.size() || index.column() >= 3)
		return QVariant();

	const DebugMessage &message = m_messages[index.row()];

	// Обрабатываем роль DecorationRole для отображения иконок (только в первой колонке)
	if (role == Qt::DecorationRole && index.column() == TypeColumn)
	{
		return m_typeIcons.value(message.type, QIcon());
	}

	// Обрабатываем стандартную роль DisplayRole для отображения в таблице
	if (role == Qt::DisplayRole)
	{
		switch (index.column())
		{
		case TypeColumn:
			switch (message.type)
			{
			case MessageType::UplinkParameters:
				return tr("Uplink");
			case MessageType::DataReceived:
				return tr("Received");
			case MessageType::Error:
				return tr("Error");
			case MessageType::Info:
				return tr("Info");
			}
			break;
		case MessageColumn:
			return message.message;
		case TimeColumn:
			return message.timestamp.toString("hh:mm:ss");
			return message.timestamp.toString("hh:mm:ss.zzz");
		}
	}

	// Обрабатываем кастомные роли только если role >= Qt::UserRole
	if (role >= Qt::UserRole)
	{
		auto debugRole = (DebugRole)role;

		switch (debugRole)
		{
		case DebugRole::TimestampRole:
			return message.timestamp.toString("hh:mm:ss.zzz");
		case DebugRole::MessageRole:
			return message.message;
		case DebugRole::TypeRole:
			switch (message.type)
			{
			case MessageType::UplinkParameters:
				return tr("Uplink");
			case MessageType::DataReceived:
				return tr("Received");
			case MessageType::Error:
				return tr("Error");
			case MessageType::Info:
				return tr("Info");
			}
			break;
		case DebugRole::FormattedMessageRole:
			return formatMessage(message);
		}
	}

	return QVariant();
}


void DebugViewModel::addUplinkParametersMessage(const QString &data)
{
	addMessage(data, MessageType::UplinkParameters);
}

void DebugViewModel::addDataReceivedMessage(const QString &data)
{
	addMessage(data, MessageType::DataReceived);
}

void DebugViewModel::addErrorMessage(const QString &message)
{
	addMessage(message, MessageType::Error);
}

void DebugViewModel::addInfoMessage(const QString &message)
{
	addMessage(message, MessageType::Info);
}

void DebugViewModel::clearMessages()
{
	beginResetModel();
	m_messages.clear();
	endResetModel();
}

QString DebugViewModel::formatMessage(const DebugMessage &msg) const
{
	QString typeStr;
	switch (msg.type)
	{
	case MessageType::UplinkParameters:
		typeStr = tr("[Uplink]");
		break;
	case MessageType::DataReceived:
		typeStr = tr("[Received]");
		break;
	case MessageType::Error:
		typeStr = tr("[Error]");
		break;
	case MessageType::Info:
		typeStr = tr("[Info]");
		break;
	}

	//return msg.message;
	
	return QString("%1 %2\n %3")
		.arg(msg.timestamp.toString("hh:mm:ss.zzz"))
		.arg(typeStr)
		.arg(msg.message);
}

void DebugViewModel::addMessage(const QString &message, MessageType type)
{
	// Ограничиваем количество сообщений
	if (m_messages.size() >= MAX_MESSAGES)
	{
		beginRemoveRows(QModelIndex(), 0, 0);
		m_messages.removeFirst();
		endRemoveRows();
	}

	// Добавляем новое сообщение
	int row = m_messages.size();
	beginInsertRows(QModelIndex(), row, row);
	m_messages.append(DebugMessage(QDateTime::currentDateTime(), message, type));
	endInsertRows();
}

void DebugViewModel::setIconForType(MessageType type, const QIcon &icon)
{
	m_typeIcons[type] = icon;
	// Обновляем все строки, которые имеют этот тип
	for (int i = 0; i < m_messages.size(); ++i)
	{
		if (m_messages[i].type == type)
		{
			QModelIndex index = createIndex(i, TypeColumn);
			emit dataChanged(index, index, QVector<int>() << Qt::DecorationRole);
		}
	}
}

QIcon DebugViewModel::getIconForType(MessageType type) const
{
	return m_typeIcons.value(type, QIcon());
}
