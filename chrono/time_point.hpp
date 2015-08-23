#ifndef HMR_CHRONO_TIMEPOINT_INC
#define HMR_CHRONO_TIMEPOINT_INC 100
#

/*=== homura chrono ===
*/
#include<homuraLib_v2/type.hpp>

#define hmr_chrono_HOURSEC			60u	
#define hmr_chrono_HALFDAYSEC	 43200u
#define hmr_chrono_DAYSEC		86400ul
namespace hmr {
	namespace chrono {
		//タイムスタンプ(ミリ秒精度)型
		typedef uint16 msec_time_point;
		//タイムスタンプ（秒精度）型
		typedef uint8 sec_time_point;
		//タイムスタンプ（時分秒）型
		typedef uint16 hds_time_point;
		//タイムスタンプ（UnixTime）型
		typedef sint32 unix_time_point;
		//タイムスタンプ（time_t+ミリ秒）
		struct time_point {
			unix_time_point Time;
			msec_time_point MSec;	//0-999：ミリ秒
			time_point() :Time(0), MSec(0) {}
			time_point(unix_time_point Time_, msec_time_point MSec_) :Time(Time_), MSec(MSec_) {}
		};
	}
}
#
#endif
