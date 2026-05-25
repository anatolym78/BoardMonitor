#include "parameter.hh"

#include <cstring>
#include <stdexcept>
#include <variant>

enum {
    kParameterType_Pos = 0,
    kParameterData_Pos = 1
};

namespace radio {
namespace data {

std::size_t Parameter::size() const noexcept {
    auto data_size = std::visit<std::size_t>([](const auto& raw_data) {
        return sizeof(raw_data);
    }, data);

    return data_size + 1;
}

bool Parameter::pack(std::byte* dst, std::size_t len) const {
    if (len < size()) {
        return false;
    }

    dst[kParameterType_Pos] = static_cast<std::byte>(type);
    std::visit([&dst](const auto& raw_data) {
        std::memcpy(&dst[kParameterData_Pos], &raw_data, sizeof(raw_data));
    }, data);

    return true;
}

bool Parameter::unpack(const std::byte* src, std::size_t len) {
    if (len < size() ||
        src[kParameterType_Pos] != static_cast<std::byte>(type)) {
        return false;
    }

    std::visit([&src](auto& raw_data) {
        std::memcpy(&raw_data, &src[kParameterData_Pos], sizeof(raw_data));
    }, data);

    return true;
}

Parameter::Data Parameter::create() {
    switch (type) {
    case ParameterType::kProtocol: return Protocol();

    case ParameterType::kTimeStamp: return Timestamp();

    case ParameterType::kArmingFlag: return ArmingFlag();
    case ParameterType::kFlightMode: return FlightMode();

    case ParameterType::kDataSource: return DataAddress();
    case ParameterType::kDataDestination: return DataAddress();

    case ParameterType::kNeuroPIDFlag: return UsageFlag();
    case ParameterType::kDetectObstaclesFlag: return UsageFlag();

    case ParameterType::kObstacleFlag: return ObstacleFlag();

    case ParameterType::kGyroAngleRates: return AngleRates();
    case ParameterType::kDesiredAngleRates: return AngleRates();

    case ParameterType::kAcceleration: return Acceleration();
    case ParameterType::kGroundSpeed: return GroundSpeed();

    case ParameterType::kAttitude: return Attitude();
    case ParameterType::kAltitude: return Altitude();
    case ParameterType::kGpsCoordinates: return GpsCoordinates();

    case ParameterType::kBatteryVoltage: return BatteryVoltage();

    case ParameterType::kAdjustmentValues: return Adjustments();
    case ParameterType::kTotalThrottle: return TotalThrottle();

    default:
        throw std::invalid_argument(
            "Parameter::create() invalid parameter type");
    }
}

} // namespace data
} // namespace radio
