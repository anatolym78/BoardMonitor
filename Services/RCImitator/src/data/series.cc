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
		voltage.voltage = static_cast<decltype(voltage.voltage)>(voltage_func(_us_timestamp));
	}

	// =========================== flags group ====================

	void Series::operator()(ArmingFlag& arming_flag)
	{
		arming_flag.armed = 0;
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
		acceleration.x = 0;
		acceleration.y = 0;
		acceleration.z = 0;
	}

	void Series::operator()(GroundSpeed& ground_speed)
	{
		ground_speed.speed = ground_speed_func(_us_timestamp);
	}

	// =========================== attitude group ====================

	void Series::operator()(Attitude& attitude)
	{
		attitude.roll = 0;
		attitude.pitch = 0;
		attitude.yaw = 0;
	}
	void Series::operator()(Altitude& altitude)
	{
		altitude.alt = 0;
	}	

	void Series::operator()(GpsCoordinates& coordinates)
	{
		coordinates.lat = 0;
		coordinates.lon = 0;
	}	

	// =========================== controls group ====================

	void Series::operator()(TotalThrottle& throttle)
	{
		throttle.throttle = 0;
	}

	void Series::operator()(Adjustments& adjustments)
	{
		adjustments.motors[0] = 0;
		adjustments.motors[1] = 0;
		adjustments.motors[2] = 0;
		adjustments.motors[3] = 0;
	}
}

