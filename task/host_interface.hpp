#ifndef HMR_TASK_HOSTINTERFACE_INC
#define HMR_TASK_HOSTINTERFACE_INC 100
#
#include"client_interface.hpp"
namespace hmr{
	namespace task {
		class host_interface {
		public:
			typedef hmr::task::client_interface client_interface;
			typedef hmr::task::duration duration;
		public:
			virtual bool start(client_interface& Client_, duration Interval_, duration Count_)=0;
			virtual void quick_start(client_interface& Client_, duration Interval_, duration Count_)=0;
			virtual bool is_start(client_interface& Client_)const=0;
			virtual bool restart(client_interface& Client_, duration Interval_, duration Count_=0)=0;
			virtual bool stop(client_interface& Client_)=0;
		};
	}
}
#
#endif
