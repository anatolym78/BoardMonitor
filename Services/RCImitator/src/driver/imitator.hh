#pragma once

#include <atomic>
#include <mutex>

#include <QString>
#include <QTimer>

#include "../driver.hh"

namespace radio {
namespace driver {

/**
 * @brief The radio driver imitator
 */
class DriverImitator : public IDriver {
    Q_OBJECT

public:
    DriverImitator();
    ~DriverImitator() { stop(); }

    /**
     * @brief Set the delay between data updates
     * @param delay The delay in ms*10
     */
    void setDelay(int delay) override;
    /**
     * @brief Enable/disable ACKs
     * @param enable True to enable ACKs, false to disable
     */
    void enableAck(bool enable) override;
    /**
     * @brief Enable/disable short-period shutdowns
     * @param enable True to enable shutdown, false to disable
     */
    void enableShutdown(bool enable) override;

    /**
     * @brief Start the driver
     */
    void start() override;
    /**
     * @brief Stop the driver
     */
    void stop() override;

    /**
     * @brief Write data to the driver
     * @param data The JSON-serialized data to write
     */
    void write(const QString& data) override;

private slots:
    void onLoopTimerTimeout();

private:
    QTimer* loop_timer_ = nullptr;
    std::atomic<State> state_ = State::kDisconnected;

    int send_delay_ = 0;

    bool enable_ack_ = false;

    bool enable_shutdown_ = false;
    std::atomic<bool> is_shutdown_ = false;

    struct {
        int send = 0;
        int shutdown = 0;
    } counters_;

    QString ack_data_;
    std::mutex ack_data_mtx_;
};

} // namespace driver
} // namespace radio
