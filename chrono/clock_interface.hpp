#ifndef HMR_CHRONO_CLOCKINTERFACE_INC
#define HMR_CHRONO_CLOCKINTERFACE_INC 100
#
#include"time_point.hpp"
namespace hmr {
	namespace chrono {
		class clock_interface{
		public://clock_interface
			//���݂̎������擾
			virtual time_point now(void) const=0;
			//���݂�unix�^�C�����擾
			virtual unix_time_point now_unix(void) const=0;
			//���݂̎����b���擾
			virtual hds_time_point now_hds(void) const{return now_unix()%hmr_chrono_HALFDAYSEC;}
			//���݂̕b���擾
			virtual sec_time_point now_sec(void) const{return now_unix()%hmr_chrono_HOURSEC;}
		};

	}
}
#
#endif
