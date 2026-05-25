#pragma once

#include <memory>

namespace radio {

class IDriver;

/**
 * @brief The driver builder interface
 */
class IDriverBuilder {
public:
    using Driver_Ptr = std::shared_ptr<IDriver>;

    virtual ~IDriverBuilder() = default;

    /**
     * @brief Set the delay between data updates
     * @param delay The delay in ms*10
     * @return The driver object reference
     */
    virtual IDriverBuilder& setDelay(int delay) = 0;
    /**
     * @brief Enable/disable ACKs
     * @param enable True to enable ACKs, false to disable
     * @return The driver object reference
     */
    virtual IDriverBuilder& enableAck(bool enable) = 0;
    /**
     * @brief Enable/disable short-period shutdowns
     * @param enable True to enable shutdown, false to disable
     * @return The driver object reference
     */
    virtual IDriverBuilder& enableShutdown(bool enable) = 0;

    /**
     * @brief Get the constructed driver object
     * @return Driver_Ptr
     */
    virtual Driver_Ptr get() = 0;
};

} // namespace radio
