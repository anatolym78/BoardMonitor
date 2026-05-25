#pragma once

#include <string>
#include <vector>

#include <boost/json.hpp>

#include "parameter.hh"
#include "parameters.hh"

namespace radio {
namespace data {

/**
 * @brief The parameter list JSON-serializer
 */
class JSONParameterSerializer {
    /**
     * @brief A parameter packager
     */
    class Packager {
    public:
        void operator()(boost::json::array& container, ParameterType type, const data::Protocol& protocol);
        void operator()(boost::json::array& container, ParameterType type, const data::Timestamp& timestamp);
        void operator()(boost::json::array& container, ParameterType type, const data::ArmingFlag& arming_flag);
        void operator()(boost::json::array& container, ParameterType type, const data::FlightMode& flight_mode);
        void operator()(boost::json::array& container, ParameterType type, const data::DataAddress& address);
        void operator()(boost::json::array& container, ParameterType type, const data::UsageFlag& usage_flag);
        void operator()(boost::json::array& container, ParameterType type, const data::ObstacleFlag& obstacle_flag);
        void operator()(boost::json::array& container, ParameterType type, const data::AngleRates& angle_rates);
        void operator()(boost::json::array& container, ParameterType type, const data::Acceleration& acceleration);
        void operator()(boost::json::array& container, ParameterType type, const data::GroundSpeed& ground_speed);
        void operator()(boost::json::array& container, ParameterType type, const data::Attitude& attitude);
        void operator()(boost::json::array& container, ParameterType type, const data::Altitude& altitude);
        void operator()(boost::json::array& container, ParameterType type, const data::GpsCoordinates& coordinates);
        void operator()(boost::json::array& container, ParameterType type, const data::BatteryVoltage& voltage);
        void operator()(boost::json::array& container, ParameterType type, const data::Adjustments& adjustments);
        void operator()(boost::json::array& container, ParameterType type, const data::TotalThrottle& throttle);
    };

public:
    /**
     * @brief Serialize parameters into the JSON string
     * @return JSON serialized parameters string
     */
    static std::string serialize(const std::vector<Parameter>& params);

private:
    static void serialize(boost::json::array& container, const Parameter& param);
};

} // namespace radio
} // namespace data
