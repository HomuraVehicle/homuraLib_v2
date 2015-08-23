#ifndef HMR_CHRONO_CLOCKINTERFACE_INC
#define HMR_CHRONO_CLOCKINTERFACE_INC 100
#
#include"time_point.hpp"
namespace hmr {
	namespace chrono {
		class clock_interface{
		public://clock_interface
			//Œ»İ‚Ì‚ğæ“¾
			virtual time_point now(void) const=0;
			//Œ»İ‚Ìunixƒ^ƒCƒ€‚ğæ“¾
			virtual unix_time_point now_unix(void) const=0;
			//Œ»İ‚Ì•ª•b‚ğæ“¾
			virtual hds_time_point now_hds(void) const{return now_unix()%hmr_chrono_HALFDAYSEC;}
			//Œ»İ‚Ì•b‚ğæ“¾
			virtual sec_time_point now_sec(void) const{return now_unix()%hmr_chrono_HOURSEC;}
		};

	}
}
#
#endif
