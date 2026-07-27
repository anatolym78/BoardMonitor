#include "serializer.hh"

#include <stdexcept>
#include <variant>

#include <boost/json/src.hpp>

#include "parameters.hh"

namespace radio {
namespace data {

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Protocol& protocol) {
    boost::json::object object;
    object["label"] = "protocol";
    object["value"] = protocol.device;
    object["value"] = protocol.driver;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Timestamp& timestamp) {
    boost::json::object object;
    object["label"] = "timeStamp_ms";
    object["value"] = timestamp.ms;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::ArmingFlag& arming_flag) {
    boost::json::object object;
    object["label"] = "armingFlag";
    object["value"] = arming_flag.armed;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::FlightMode& flight_mode) {
    boost::json::object object;
    object["label"] = "flightMode";
    object["value"] = static_cast<int>(flight_mode.mode);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType type, const data::DataAddress& address) {
    boost::json::object object;
    switch (type) {
        case ParameterType::kDataDestination:
            object["label"] = "dataDestination";
            break;

        case ParameterType::kDataSource:
            object["label"] = "dataSource";
            break;

        default: break;
    }
    object["value"] = static_cast<int>(address.address);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType type, const data::UsageFlag& usage_flag) {
    boost::json::object object;
    switch (type) {
        case ParameterType::kNeuroPIDFlag:
            object["label"] = "neuroPIDFlag";
            break;

        case ParameterType::kDetectObstaclesFlag:
            object["label"] = "detectObstaclesFlag";
            break;

        default: break;
    }
    object["value"] = static_cast<int>(usage_flag.flag);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::ObstacleFlag& obstacle_flag) {
    boost::json::object object;
    object["label"] = "obstacleFlag";
    object["value"] = obstacle_flag.detected;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType type, const data::AngleRates& angle_rates) {
    boost::json::object object;
    switch (type) {
        case ParameterType::kGyroAngleRates:
            object["label"] = "gyroAngleRates";
            break;

        case ParameterType::kDesiredAngleRates:
            object["label"] = "desiredAngleRates";
            break;

        default: break;
    }

    boost::json::array values;
    values.push_back(angle_rates.roll);
    values.push_back(angle_rates.pitch);
    values.push_back(angle_rates.yaw);
    object["value"] = std::move(values);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Acceleration& acceleration) {
    boost::json::object object;
    object["label"] = "acceleration";

    boost::json::array values;
    values.push_back(acceleration.x);
    values.push_back(acceleration.y);
    values.push_back(acceleration.z);
    object["value"] = std::move(values);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::GroundSpeed& ground_speed) {
    boost::json::object object;
    object["label"] = "groundSpeed";
    object["value"] = ground_speed.speed;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Attitude& attitude) {
    boost::json::object object;
    object["label"] = "attitude";

    boost::json::array values;
    values.push_back(attitude.roll);
    values.push_back(attitude.pitch);
    values.push_back(attitude.yaw);
    object["value"] = std::move(values);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Altitude& altitude) {
    boost::json::object object;
    object["label"] = "altitude";
    object["value"] = altitude.alt;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::GpsCoordinates& coordinates) {
    boost::json::object object;
    object["label"] = "gpsCoordinates";

    boost::json::array values;
    values.push_back(coordinates.lat);
    values.push_back(coordinates.lon);
    object["value"] = std::move(values);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::BatteryVoltage& voltage) {
    boost::json::object object;
    object["label"] = "batteryVoltage";
    object["value"] = voltage.voltage;

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::Adjustments& adjustments) {
    boost::json::object object;
    object["label"] = "adjustmentValues";

    boost::json::array values;
    for (int16_t value : adjustments.motors) {
        values.push_back(value);
    }
    object["value"] = std::move(values);

    container.push_back(std::move(object));
}

void JSONParameterSerializer::Packager::operator()(
    boost::json::array& container, ParameterType /*type*/, const data::TotalThrottle& throttle) {
    boost::json::object object;
    object["label"] = "totalThrottle";
    object["value"] = throttle.throttle;

    container.push_back(std::move(object));
}

std::string JSONParameterSerializer::serialize(const std::vector<Parameter>& params) {
    boost::json::object root;

    // Initialize containers for each group
    boost::json::array status_container;
    boost::json::array flags_container;
    boost::json::array motion_container;
    boost::json::array controls_container;
    boost::json::array attitude_container;

    // Fill group containers with parameters
    for (const auto& param : params) {
        switch (param.type) {
            case ParameterType::kTimeStamp:
            case ParameterType::kFlightMode:
            case ParameterType::kBatteryVoltage:
                serialize(status_container, param);
                break;

            case ParameterType::kArmingFlag:
            case ParameterType::kNeuroPIDFlag:
            case ParameterType::kDetectObstaclesFlag:
            case ParameterType::kObstacleFlag:
                serialize(flags_container, param);
                break;

            case ParameterType::kGyroAngleRates:
            case ParameterType::kDesiredAngleRates:
            case ParameterType::kAcceleration:
            case ParameterType::kGroundSpeed:
                serialize(motion_container, param);
                break;

            case ParameterType::kAdjustmentValues:
            case ParameterType::kTotalThrottle:
                serialize(controls_container, param);
                break;

            case ParameterType::kAttitude:
            case ParameterType::kAltitude:
            case ParameterType::kGpsCoordinates:
                serialize(attitude_container, param);
                break;

            default:
                throw std::invalid_argument(
                    "JSONParameterSerializer::serialize unexpected parameter type: " +
                    std::to_string(static_cast<int>(param.type)));
        }
    }

    // Move containers to the root object and serialize the root
    root["status"] = std::move(status_container);
    root["flags"] = std::move(flags_container);
    root["motion"] = std::move(motion_container);
    root["controls"] = std::move(controls_container);
    root["attitude"] = std::move(attitude_container);

    return boost::json::serialize(root);
}

void JSONParameterSerializer::serialize(boost::json::array& container, const Parameter& param) {
    std::visit([&container, &param](const auto& data) {
        Packager().operator()(container, param.type, data);
    }, param.data);
}

} // namespace data
} // namespace radio
