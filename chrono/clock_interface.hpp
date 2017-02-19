#ifndef HMR_CHRONO_CLOCKINTERFACE_INC
#define HMR_CHRONO_CLOCKINTERFACE_INC 100
#
#include"time_point.hpp"
namespace hmr {
	namespace chrono {
		class clock_interface{
		public://clock_interface
			//現在の時刻を取得
			virtual time_point now(void) const=0;
			//現在のunixタイムを取得
			virtual unix_time_point now_unix(void) const=0;
			//現在の時分秒を取得
			virtual hds_time_point now_hds(void) const{return now_unix()%hmr_chrono_HALFDAYSEC;}
			//現在の秒を取得
			virtual sec_time_point now_sec(void) const{return now_unix()%hmr_chrono_HOURSEC;}
		};

	}
}
#
#endif
