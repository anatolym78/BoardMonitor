#include "series.hh"


namespace radio::data
{
	void Series::operator()(Protocol& protocol)
	{
		protocol.device = 0;
		protocol.driver = 0;
	}

	void Series::operator()(DataAddress& address)
	{
		address.address = static_cast<DataAddress::Node>(DataAddress::Node::kUndefined);
	}

	// =========================== status group ====================

	void Series::operator()(Timestamp& timestamp)
	{
		timestamp.us = _us_timestamp;
	}

	void Series::operator()(FlightMode& flight_mode)
	{
		flight_mode.mode = static_cast<FlightMode::Mode>(FlightMode::Mode::kAngleMode);
	}

	void Series::operator()(BatteryVoltage& voltage)
	{
		using VoltageType = decltype(voltage.voltage);
		voltage.voltage = static_cast<VoltageType>(voltage_func(_us_timestamp));
	}

	// =========================== flags group ====================

	void Series::operator()(ArmingFlag& arming_flag)
	{
		arming_flag.armed = false;
	}

	void Series::operator()(UsageFlag& usage_flag)
	{
		usage_flag.flag = true;
	}

	void Series::operator()(ObstacleFlag& obstacle_flag)
	{
		obstacle_flag.detected = true;
	}

	// =========================== motion group ====================

	void Series::operator()(AngleRates& angle_rates)
	{

		angle_rates.roll = angle_roll_func(_us_timestamp);
		angle_rates.pitch = angle_pitch_func(_us_timestamp);
		angle_rates.yaw = angle_yaw_func(_us_timestamp);
	}

	void Series::operator()(Acceleration& acceleration)
	{
		acceleration.x = acceleration_x_func(_us_timestamp);
		acceleration.y = acceleration_y_func(_us_timestamp);
		acceleration.z = acceleration_z_func(_us_timestamp);
	}

	void Series::operator()(GroundSpeed& ground_speed)
	{
		ground_speed.speed = ground_speed_func(_us_timestamp);
	}

	// =========================== attitude group ====================

	void Series::operator()(Attitude& attitude)
	{
		attitude.roll = static_cast<int16_t>(attitude_roll_func(_us_timestamp));
		attitude.pitch = static_cast<int16_t>(attitude_pitch_func(_us_timestamp));
		attitude.yaw = static_cast<int16_t>(attitude_yaw_func(_us_timestamp));
	}
	void Series::operator()(Altitude& altitude)
	{
		altitude.alt = static_cast<int32_t>(altitude_func(_us_timestamp));
	}	

	void Series::operator()(GpsCoordinates& coordinates)
	{
		coordinates.lat = kGpsCenterLat + static_cast<int32_t>(gps_lat_func(_us_timestamp));
		coordinates.lon = kGpsCenterLon + static_cast<int32_t>(gps_lon_func(_us_timestamp));
	}

	// =========================== controls group ====================

	void Series::operator()(TotalThrottle& throttle)
	{
		throttle.throttle = static_cast<decltype(throttle.throttle)>(throttle_func(_us_timestamp));
	}

	void Series::operator()(Adjustments& adjustments)
	{
		using MotorValueType = decltype(adjustments.motors[0]);
		adjustments.motors[0] = std::remove_reference_t<MotorValueType>(adjustments_one_func(_us_timestamp));
		adjustments.motors[1] = std::remove_reference_t<MotorValueType>(adjustments_two_func(_us_timestamp));
		adjustments.motors[2] = std::remove_reference_t<MotorValueType>(adjustments_three_func(_us_timestamp));
		adjustments.motors[3] = std::remove_reference_t<MotorValueType>(adjustments_four_func(_us_timestamp));
	}
}

