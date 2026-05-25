#pragma once

#include <memory>

#include "../builder.hh"

namespace radio {
namespace driver {

/**
 * @brief The driver builder implementation
 */
class DriverBuilder : public IDriverBuilder {
public:
    using Driver_Ptr = std::shared_ptr<IDriver>;

    DriverBuilder();

    /**
     * @brief Set the delay between data updates
     * @param delay The delay in ms*10
     * @return The driver object reference
     */
    IDriverBuilder& setDelay(int delay) override;
    /**
     * @brief Enable/disable ACKs
     * @param enable True to enable ACKs, false to disable
     * @return The driver object reference
     */
    IDriverBuilder& enableAck(bool enable) override;
    /**
     * @brief Enable/disable short-period shutdowns
     * @param enable True to enable shutdown, false to disable
     * @return The driver object reference
     */
    IDriverBuilder& enableShutdown(bool enable) override;

    /**
     * @brief Get the constructed driver object
     * @return Driver_Ptr
     */
    Driver_Ptr get() override;

private:
    Driver_Ptr driver_;
};

} // namespace driver
} // namespace radio
