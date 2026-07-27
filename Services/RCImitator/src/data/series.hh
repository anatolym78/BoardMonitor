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

			float operator()(int64_t us)
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

		class EllipseFunction
		{
		public:
			EllipseFunction(float a, float b, float period, float offset_y = 0, float offset_x = 0) :
				amplitude_a_(a), amplitude_b_(b), period_in_sec_(period), offset_y_(offset_y), offset_x_(offset_x)
			{

			}
			float amplitude_a_ = 10;
			float amplitude_b_ = 5;
			float period_in_sec_ = 10;
			float phase_ = 0;
			float offset_y_ = 0;
			float offset_x_ = 0;
		};

		class Series
		{
		private:
			int64_t _us_timestamp = 0;
			SinusFunction angle_roll_func = SinusFunction(10, 5, 0);
			SinusFunction angle_pitch_func = SinusFunction(20, 10, 1.57079632679f); 
			SinusFunction angle_yaw_func = SinusFunction(30, 20, 3.14159265359f);
			SinusFunction ground_speed_func =  SinusFunction(10, 18, 0);
			SinusFunction voltage_func =  SinusFunction(2, 5, 0, 12);
			//SinusFunction throttle_func = SinusFunction(100, 20, 0); 
			SinusFunction throttle_func = SinusFunction(100, 20, 0, 100);
			SinusFunction adjustments_one_func = SinusFunction(25, 5, 0, 300);
			SinusFunction adjustments_two_func = SinusFunction(30, 5, 1.57079632679f, 300);
			SinusFunction adjustments_three_func = SinusFunction(20, 5, 3.0f, 300);
			SinusFunction adjustments_four_func = SinusFunction(25, 5, 4.71238898038f, 300);
			SinusFunction acceleration_x_func = SinusFunction(5, 10, 0);
			SinusFunction acceleration_y_func = SinusFunction(5, 10, 1);
			SinusFunction acceleration_z_func = SinusFunction(5, 10, 2);
			SinusFunction attitude_roll_func = SinusFunction(100, 30, 0);
			SinusFunction attitude_pitch_func = SinusFunction(150, 45, 1.57079632679f);
			SinusFunction attitude_yaw_func = SinusFunction(200, 60, 3.14159265359f);
			SinusFunction altitude_func = SinusFunction(500, 40, 0, 2000);
			SinusFunction gps_lat_func = SinusFunction(269583, 120, 0);
			SinusFunction gps_lon_func = SinusFunction(479928, 120, 1.57079632679f);

			static constexpr int32_t kGpsCenterLat = 557558000; // 55.7558°
			static constexpr int32_t kGpsCenterLon = 376173000; // 37.6173°

		public:
			Series() = default;

			/**
			 * @brief Set the board clock to the real elapsed time since start
			 * @param us Elapsed time in us
			 */
			void setElapsedUs(int64_t us) { _us_timestamp = us; }

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
