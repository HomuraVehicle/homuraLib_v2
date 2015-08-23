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
		//�^�C���X�^���v(�~���b���x)�^
		typedef uint16 msec_time_point;
		//�^�C���X�^���v�i�b���x�j�^
		typedef uint8 sec_time_point;
		//�^�C���X�^���v�i�����b�j�^
		typedef uint16 hds_time_point;
		//�^�C���X�^���v�iUnixTime�j�^
		typedef sint32 unix_time_point;
		//�^�C���X�^���v�itime_t+�~���b�j
		struct time_point {
			unix_time_point Time;
			msec_time_point MSec;	//0-999�F�~���b
			time_point() :Time(0), MSec(0) {}
			time_point(unix_time_point Time_, msec_time_point MSec_) :Time(Time_), MSec(MSec_) {}
		};
	}
}
#
#endif
