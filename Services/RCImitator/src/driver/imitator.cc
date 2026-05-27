#include "imitator.hh"

#include <random>

#include "../data/generator.hh"
#include "../data/series_generator.hh"
#include "../data/serializer.hh"

namespace radio {
namespace driver {

DriverImitator::DriverImitator() : loop_timer_(new QTimer(this)) {
    connect(loop_timer_, &QTimer::timeout, this, &DriverImitator::onLoopTimerTimeout);
}

void DriverImitator::setDelay(int delay) {
    send_delay_ = delay;
}

void DriverImitator::enableAck(bool enable) {
    enable_ack_ = enable;
}

void DriverImitator::enableShutdown(bool enable) {
    enable_shutdown_ = enable;
}

void DriverImitator::start() {
    if (state_ == State::kConnected) {
        return;
    }

    state_ = State::kConnected;
    emitStateChanged(state_);

    loop_timer_->start(10);
}

void DriverImitator::stop() {
    auto expected = State::kConnected;
    // CAS: если было kConnected — атомарно ставим kDisconnected и возвращаем true.
    if (!state_.compare_exchange_strong(expected, State::kDisconnected)) {
        return;
    }

    emitStateChanged(State::kDisconnected);
    loop_timer_->stop();

    is_shutdown_ = false;
    counters_ = {0, 0};
}

void DriverImitator::write(const QString& data) {
    if (state_ != State::kConnected || is_shutdown_) {
        return;
    }

    if (enable_ack_ && !is_shutdown_) {
        ack_data_ = data;
    }
}

void DriverImitator::onLoopTimerTimeout() {
    enum {
        kMaxShutdownCounter = 100
    };

    if (state_ != State::kConnected) {
        return;
    }

    if (!is_shutdown_) {
        ++counters_.send;
        if (counters_.send >= send_delay_) {
            counters_.send = 0;

            //auto params = data::ParameterGenerator::generate(10);
			auto params = data::ParameterSeriesGenerator::generate(10);
            emitDataAvailable(QString::fromStdString(
                data::JSONParameterSerializer::serialize(params)));

            std::lock_guard lg(ack_data_mtx_);
            if (!ack_data_.isEmpty()) {
                emitDataAvailable(ack_data_);
                ack_data_.clear();
            }
        }
    }

    if (!enable_shutdown_) {
        return;
    }

    // 0.1% chance that the driver will shutdown
    if (!is_shutdown_) {
        std::random_device device;
        std::mt19937 gen(device());
        std::uniform_int_distribution dist(0, 1000);

        is_shutdown_ = dist(gen) == 1000;
        if (is_shutdown_) {
            emitStateChanged(State::kDisconnected);
        }

        return;
    }

    ++counters_.shutdown;
    if (counters_.shutdown >= kMaxShutdownCounter) {
        counters_.shutdown = 0;
        is_shutdown_ = false;

        emitStateChanged(State::kConnected);
    }
}

} // namespace driver
} // namespace radio
