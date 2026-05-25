#include "builder.hh"

#include <memory>

#include "imitator.hh"

namespace radio {
namespace driver {

DriverBuilder::DriverBuilder() : driver_(std::make_shared<DriverImitator>()) {}

IDriverBuilder& DriverBuilder::setDelay(int delay) {
    driver_->setDelay(delay);
    return *this;
}

IDriverBuilder& DriverBuilder::enableAck(bool enable) {
    driver_->enableAck(enable);
    return *this;
}

IDriverBuilder& DriverBuilder::enableShutdown(bool enable) {
    driver_->enableShutdown(enable);
    return *this;
}

DriverBuilder::Driver_Ptr DriverBuilder::get() {
    Driver_Ptr result;
    driver_.swap(result);
    return result;
}

} // namespace driver
} // namespace radio
