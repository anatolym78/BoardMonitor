#pragma once

#include "parameters.hh"
#include <cmath>

namespace radio 
{
	namespace data
	{
		class SinusFunction
		{
		public:
			SinusFunction(float amplitude, float period, float phase, float offset = 0) :
				amplitude_(amplitude), period_in_sec_(period), phase_(phase), offset_(offset) {}

			float operator()(uint32_t us)
			{
				auto period = period_in_sec_ * 1000000;
				return amplitude_ * std::sin(6.28318530717958647692f * (us / period) + phase_) + offset_;
			}

		private:
			float amplitude_ = 10;
			float period_in_sec_ = 10;
			float phase_ = 0;
			float offset_ = 0;
		};

		class Series
		{
		private:
			uint32_t _us_timestamp = 0;
			SinusFunction angle_roll_func = SinusFunction(10, 5, 0);
			SinusFunction angle_pitch_func = SinusFunction(20, 10, 1.57079632679f); 
			SinusFunction angle_yaw_func = SinusFunction(30, 20, 3.14159265359f);
			SinusFunction ground_speed_func =  SinusFunction(10, 18, 0);
			SinusFunction voltage_func =  SinusFunction(2, 5, 0, 12);

		public:
			Series() = default;
			uint32_t next() { return _us_timestamp += 100000; }

			void operator()(data::Protocol& protocol);

			void operator()(data::DataAddress& address);

			// =========================== status group ====================

			void operator()(data::Timestamp& timestamp);

			void operator()(data::FlightMode& flight_mode);

			void operator()(data::BatteryVoltage& voltage);

			// =========================== flags group ====================

			void operator()(data::ArmingFlag& arming_flag);

			void operator()(data::ObstacleFlag& obstacle_flag);

			void operator()(data::UsageFlag& usage_flag);

			// =========================== motion group ====================

			void operator()(data::AngleRates& angle_rates);

			void operator()(data::Acceleration& acceleration);

			void operator()(data::GroundSpeed& ground_speed);

			// =========================== attitude group ====================


			void operator()(data::Attitude& attitude);

			void operator()(data::Altitude& altitude);

			void operator()(data::GpsCoordinates& coordinates);

			// =========================== controls group ====================

			void operator()(data::Adjustments& adjustments);

			void operator()(data::TotalThrottle& throttle);
		};
	}
}
