#pragma once

#include <QObject>
#include <QString>

#include "builder.hh"

namespace radio {

/**
 * @brief A driver interface
 */
class IDriver : public QObject {
    Q_OBJECT

public:
    /**
     * @brief The driver state
     */
    enum class State {
        kConnected,
        kDisconnected
    };

    virtual ~IDriver() = default;

    /**
     * @brief Set the delay between data updates
     * @param delay The delay in ms*10
     */
    virtual void setDelay(int delay) = 0;
    /**
     * @brief Enable/disable ACKs
     * @param enable True to enable ACKs, false to disable
     */
    virtual void enableAck(bool enable) = 0;
    /**
     * @brief Enable/disable short-period shutdowns
     * @param enable True to enable shutdown, false to disable
     */
    virtual void enableShutdown(bool enable) = 0;

    /**
     * @brief Start the driver
     */
    virtual void start() = 0;
    /**
     * @brief Stop the driver
     */
    virtual void stop() = 0;

    /**
     * @brief Write data to the driver
     * @param data The JSON-serialized data to write
     */
    virtual void write(const QString& data) = 0;

signals:
    /**
     * @brief Emitted when data is available
     * @param data The JSON-serialized data
     */
    void dataAvailable(QString data);
    /**
     * @brief Emitted when the driver state changes
     * @param state The new driver state
     */
    void stateChanged(State state);

protected:
    /**
     * @brief Emit the dataAvailable signal
     * @param data The JSON-serialized data
     */
    void emitDataAvailable(const QString& data) { emit dataAvailable(data); }
    /**
     * @brief Emit the stateChanged signal
     * @param state The new driver state
     */
    void emitStateChanged(State state) { emit stateChanged(state); }
};

} // namespace radio
