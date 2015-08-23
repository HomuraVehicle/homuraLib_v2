#ifndef HMR_MACHINE_SERVICE_CHRONO_INC
#define HMR_MACHINE_SERVICE_CHRONO_INC 100
#
#include<homuraLib_v2/chrono.hpp>
namespace hmr {
	namespace machine {
		namespace service {
			extern hmr::chrono::clock_interface& Chrono;
			namespace chrono{
				inline hmr::chrono::time_point now(){return Chrono.now();}
				inline hmr::chrono::hds_time_point now_hds(){return Chrono.now_hds();}
				inline hmr::chrono::unix_time_point now_unix(){return Chrono.now_unix();}
				inline hmr::chrono::sec_time_point now_sec(){return Chrono.now_sec();}
			}
		}
	}
}
#
#endif
