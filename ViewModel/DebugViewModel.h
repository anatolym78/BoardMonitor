#ifndef DEBUGVIEWMODEL_H
#define DEBUGVIEWMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QString>
#include <QIcon>
#include <QHash>

class DebugViewModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	enum class DebugRole : int
	{
		TimestampRole = Qt::UserRole + 1,
		TypeRole,
		MessageRole,
		FormattedMessageRole
	};

	enum class MessageType
	{
		UplinkParameters,
		DataReceived,
		Error,
		Info
	};

public:
	explicit DebugViewModel(QObject *parent = nullptr);
	~DebugViewModel() override = default;

	// QAbstractTableModel interface
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	QHash<int, QByteArray> roleNames() const override;

	// Методы для добавления сообщений
	void addUplinkParametersMessage(const QString &data);
	void addDataReceivedMessage(const QString &data);
	void addErrorMessage(const QString &message);
	void addInfoMessage(const QString &message);
	
	// Методы управления
	void clearMessages();
	int getMessageCount() const { return m_messages.size(); }
	
	// Методы для установки иконок
	void setIconForType(MessageType type, const QIcon &icon);
	QIcon getIconForType(MessageType type) const;

private:
	struct DebugMessage
	{
		QDateTime timestamp;
		QString message;
		MessageType type;
		
		DebugMessage(const QDateTime &ts, const QString &msg, MessageType t)
			: timestamp(ts), message(msg), type(t) {}
	};

	void addMessage(const QString &message, MessageType type);
	QString formatMessage(const DebugMessage &msg) const;

	QList<DebugMessage> m_messages;
	static const int MAX_MESSAGES = 1000; // Максимальное количество сообщений
	
	// Иконки для разных типов сообщений
	QHash<MessageType, QIcon> m_typeIcons;
	
	enum Column
	{
		TypeColumn = 0,
		TimeColumn = 1,
		MessageColumn = 2,
	};
};

// Функция для использования MessageType в QHash
inline uint qHash(DebugViewModel::MessageType type, uint seed = 0)
{
	return qHash(static_cast<int>(type), seed);
}

#endif // DEBUGVIEWMODEL_H
