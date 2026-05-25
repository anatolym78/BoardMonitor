#pragma once

#include <cstddef>
#include <variant>

#include "data.hh"
#include "parameters.hh"

namespace radio {
namespace data {

/**
 * @brief Parameter properties
 */
struct Parameter {
    /**
     * @brief Parameter data type
     */
    using Data = std::variant<
        Protocol,
        Timestamp,
        ArmingFlag,
        FlightMode,
        DataAddress,
        UsageFlag,
        ObstacleFlag,
        AngleRates,
        Acceleration,
        GroundSpeed,
        Attitude,
        Altitude,
        GpsCoordinates,
        BatteryVoltage,
        Adjustments,
        TotalThrottle
    >;

    ParameterType type;
    Data data;

    Parameter() = delete;
    Parameter(ParameterType _type) : type(_type), data(create()) {}

    /**
     * @brief Returns the size of the parameter data
     */
    std::size_t size() const noexcept;

    /**
     * @brief Packs the parameter data into a byte array
     * @param dst The destination byte array
     * @param len The length of the destination byte array
     * @return True if the parameter data was packed successfully, false otherwise
     */
    bool pack(std::byte* dst, std::size_t len) const;
    /**
     * @brief Unpacks the parameter data from a byte array
     * @param src The source byte array
     * @param len The length of the source byte array
     * @return True if the parameter data was unpacked successfully, false otherwise
     */
    bool unpack(const std::byte* src, std::size_t len);

    auto operator<=>(const Parameter& other) const = default;

private:
    Data create();
};

} // namespace data
} // namespace radio
