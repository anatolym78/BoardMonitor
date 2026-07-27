#pragma once

#include <compare>
#include <cstdint>

namespace radio {
namespace data {

/**
 * @brief Transmitted parameter types
 */
enum class ParameterType : uint8_t {
    kProtocol,            // Information protocol description

    kTimeStamp,           // Hardware timer timestamp

    kArmingFlag,          // Arming flag
    kFlightMode,          // Flight mode

    kDataSource,          // Data source identification
    kDataDestination,     // Data destination identification

    kNeuroPIDFlag,        // Neuro PID usage flag
    kDetectObstaclesFlag, // Obstacles detection usage flag

    kObstacleFlag,        // Obstacle detected flag

    kGyroAngleRates,      // Gyro angle rates
    kDesiredAngleRates,   // Desired angle rates

    kAcceleration,        // Acceleration
    kGroundSpeed,         // Ground speed

    kAttitude,            // Attitude
    kAltitude,            // Altitude
    kGpsCoordinates,      // GPS coordinates

    kBatteryVoltage,      // Battery voltage

    kAdjustmentValues,    // Adjustment values
    kTotalThrottle,       // Total throttle

    kTotal
};

/**
 * @brief Information protocol description
 */
struct Protocol {
    uint8_t device = 0;
    uint8_t driver = 0;

    auto operator<=>(const Protocol& other) const = default;
};

/**
 * @brief Hardware timer timestamp in ms since board power-on
 */
struct Timestamp {
    int32_t ms = 0;

    auto operator<=>(const Timestamp& other) const = default;
};

/**
 * @brief Arming flag
 */
struct ArmingFlag {
    bool armed = false;

    auto operator<=>(const ArmingFlag& other) const = default;
};

/**
 * @brief Flight mode
 */
struct FlightMode {
    enum class Mode {
        kAngleMode         = (1 << 0),
        kHorizonMode       = (1 << 1),
        kHeadingMode       = (1 << 2),
        kNavAltholdMode    = (1 << 3),
        kNavRthMODE        = (1 << 4),
        kNavPosHoldMode    = (1 << 5),
        kHeadFreeMode      = (1 << 6),
        kNavLaunchMode     = (1 << 7),
        kManualMode        = (1 << 8),
        kFailSafeMode      = (1 << 9),
        kAutoTune          = (1 << 10),
        kNavWPMode         = (1 << 11),
        kNavCourseHoldMode = (1 << 12),
        kFlaperon          = (1 << 13),
        kTurnAssistant     = (1 << 14),
        kTurtleMode        = (1 << 15),
        kSoaringMode       = (1 << 16),
        kAngleholdMode     = (1 << 17),
        kNavFWAutoland     = (1 << 18),
        kNavSendTo         = (1 << 19),
    };

    Mode mode = Mode::kFailSafeMode;

    auto operator<=>(const FlightMode& other) const = default;
};

/**
 * @brief Data address identification
 */
struct DataAddress {
    /**
     * @brief Connected devices enumeration
     */
    enum class Node : uint8_t {
        kUndefined,

        kFlightController,
        kNeurostab,
        kDispatcher,
        kRadio
    };

    Node address = Node::kUndefined;

    auto operator<=>(const DataAddress& other) const = default;
};

/**
 * @brief Technology usage flag
 */
struct UsageFlag {
    bool flag = false;

    auto operator<=>(const UsageFlag& other) const = default;
};

/**
 * @brief Obstacle detection flag
 */
struct ObstacleFlag {
    bool detected = false;

    auto operator<=>(const ObstacleFlag& other) const = default;
};

/**
 * @brief Angle rates in °/s
 */
struct AngleRates {
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    auto operator<=>(const AngleRates& other) const = default;
};

/**
 * @brief Acceleration in g
 */
struct Acceleration {
    float x = 0;
    float y = 0;
    float z = 0;

    auto operator<=>(const Acceleration& other) const = default;
};

/**
 * @brief Ground speed in cm/s
 */
struct GroundSpeed {
    float speed = 10;
    auto operator<=>(const GroundSpeed& other) const = default;
};

/**
 * @brief Attitude in °x10
 */
struct Attitude {
    int16_t roll = 0;
    int16_t pitch = 0;
    int16_t yaw = 0;

    auto operator<=>(const Attitude& other) const = default;
};

/**
 * @brief Altitude in cm
 */
struct Altitude {
    int32_t alt = 0;
    auto operator<=>(const Altitude& other) const = default;
};

/**
 * @brief GPS coordinates
 */
struct GpsCoordinates {
    int32_t lat = 0; // Latitude x1e+7
    int32_t lon = 0; // Longitude x1e+7

    auto operator<=>(const GpsCoordinates& other) const = default;
};

/**
 * @brief Battery voltage in mV
 */
struct BatteryVoltage {
    uint16_t voltage = 0;

    auto operator<=>(const BatteryVoltage& other) const = default;
};

/**
 * @brief Motor adjustment values
 */
struct Adjustments {
    enum { kMotorsNumber = 4 };

    int16_t motors[kMotorsNumber];

    auto operator<=>(const Adjustments& other) const = default;
};

/**
 * @brief Total throttle
 */
struct TotalThrottle {
    int16_t throttle = 0;
    int16_t speed = 0;
    float amplitude = 10;
    float period = 3.1415f / 2;
    float time = 0;

    void next();
    auto operator<=>(const TotalThrottle& other) const = default;
};

} // namespace data
} // namespace radio
