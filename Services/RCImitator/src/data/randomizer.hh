#pragma once

#include <random>

#include "parameters.hh"

namespace radio {
namespace data {

/**
 * @brief The parameter data randomizer
 */
class ParameterRandomizer {
public:
    void operator()(data::Protocol& protocol) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<uint8_t>::min(),
            std::numeric_limits<uint8_t>::max());

        protocol.device = value_dist(gen_);
        protocol.driver = value_dist(gen_);
    }

    void operator()(data::Timestamp& timestamp) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int32_t>::min(),
            std::numeric_limits<int32_t>::max());

        timestamp.ms = value_dist(gen_);
    }

    void operator()(data::ArmingFlag& arming_flag) {
        static std::uniform_int_distribution<> value_dist(0, 1);

        arming_flag.armed = value_dist(gen_);
    }

    void operator()(data::FlightMode& flight_mode) {
        static std::uniform_int_distribution<> value_dist(0, 19);

        flight_mode.mode = static_cast<data::FlightMode::Mode>(
            1 << value_dist(gen_));
    }

    void operator()(data::DataAddress& address) {
        static std::uniform_int_distribution<> value_dist(0, 4);

        address.address = static_cast<data::DataAddress::Node>(
            value_dist(gen_));
    }

    void operator()(data::UsageFlag& usage_flag) {
        static std::uniform_int_distribution<> value_dist(0, 1);

        usage_flag.flag = value_dist(gen_);
    }

    void operator()(data::ObstacleFlag& obstacle_flag) {
        static std::uniform_int_distribution<> value_dist(0, 1);

        obstacle_flag.detected = value_dist(gen_);
    }

    void operator()(data::AngleRates& angle_rates) {
        static std::uniform_int_distribution<> value_dist(0, 255);
        static std::uniform_int_distribution<> denum_dist(1, 10000);

        angle_rates.roll = static_cast<float>(value_dist(gen_)) /
                           static_cast<float>(denum_dist(gen_));
        angle_rates.pitch = static_cast<float>(value_dist(gen_)) /
                            static_cast<float>(denum_dist(gen_));
        angle_rates.yaw = static_cast<float>(value_dist(gen_)) /
                          static_cast<float>(denum_dist(gen_));
    }

    void operator()(data::Acceleration& acceleration) {
        static std::uniform_int_distribution<> value_dist(0, 255);
        static std::uniform_int_distribution<> denum_dist(1, 10000);

        acceleration.x = static_cast<float>(value_dist(gen_)) /
                         static_cast<float>(denum_dist(gen_));
        acceleration.y = static_cast<float>(value_dist(gen_)) /
                         static_cast<float>(denum_dist(gen_));
        acceleration.z = static_cast<float>(value_dist(gen_)) /
                         static_cast<float>(denum_dist(gen_));
    }

    void operator()(data::GroundSpeed& ground_speed) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int16_t>::min(),
            std::numeric_limits<int16_t>::max());

        ground_speed.speed = value_dist(gen_);
    }

    void operator()(data::Attitude& attitude) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int16_t>::min(),
            std::numeric_limits<int16_t>::max());

        attitude.roll = value_dist(gen_);
        attitude.pitch = value_dist(gen_);
        attitude.yaw = value_dist(gen_);
    }

    void operator()(data::Altitude& altitude) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int32_t>::min(),
            std::numeric_limits<int32_t>::max());

        altitude.alt = value_dist(gen_);
    }

    void operator()(data::GpsCoordinates& coordinates) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int32_t>::min(),
            std::numeric_limits<int32_t>::max());

        coordinates.lat = value_dist(gen_);
        coordinates.lon = value_dist(gen_);
    }

    void operator()(data::BatteryVoltage& voltage) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<uint16_t>::min(),
            std::numeric_limits<uint16_t>::max());

        voltage.voltage = value_dist(gen_);
    }

    void operator()(data::Adjustments& adjustments) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int16_t>::min(),
            std::numeric_limits<int16_t>::max());

        adjustments.motors[0] = value_dist(gen_);
        adjustments.motors[1] = value_dist(gen_);
        adjustments.motors[2] = value_dist(gen_);
        adjustments.motors[3] = value_dist(gen_);
    }

    void operator()(data::TotalThrottle& throttle) {
        static std::uniform_int_distribution<> value_dist(
            std::numeric_limits<int16_t>::min(),
            std::numeric_limits<int16_t>::max());

        throttle.throttle = value_dist(gen_);
    }

private:
    std::random_device device_;
    std::mt19937 gen_ = std::mt19937(device_());
};

} // namespace radio
} // namespace data
