#ifndef HMR_MACHINE_SERVICE_TASK_INC
#define HMR_MACHINE_SERVICE_TASK_INC 100
#
#include<homuraLib_v2/task.hpp>
namespace hmr {
	namespace machine {
		namespace service {
			extern hmr::task::host_interface& Task;
			namespace task{
				inline bool start(hmr::task::client_interface& Client_, hmr::task::duration Interval_, hmr::task::duration Count_ = 0) { return Task.start(Client_, Interval_, Count_); }
				inline void quick_start(hmr::task::client_interface& Client_, hmr::task::duration Interval_, hmr::task::duration Count_ = 0) { Task.quick_start(Client_, Interval_, Count_); }
				inline bool is_start(hmr::task::client_interface& Client_){ return Task.is_start(Client_); }
				inline bool restart(hmr::task::client_interface& Client_, hmr::task::duration Interval_, hmr::task::duration Count_=0) { return Task.restart(Client_, Interval_, Count_); }
				inline void stop(hmr::task::client_interface& Client_) { Task.stop(Client_); }
			}
		}
	}
}
#
#endif
