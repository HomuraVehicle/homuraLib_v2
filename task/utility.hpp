#ifndef HMR_TASK_UTILITY_INC
#define HMR_TASK_UTILITY_INC 100
#
#include"client_interface.hpp"
#include"host_interface.hpp"
namespace hmr {
	namespace task {
		struct task_driver {
		private:
			host_interface* pHost;
			client_interface* pClient;
			bool IsStart;
		private:
			task_driver(const task_driver&);
			const task_driver& operator=(const task_driver&);
		public:
			task_driver() :pHost(0), pClient(0), IsStart(false){}
			task_driver(host_interface& rHost_,client_interface& rClient_, duration Interval_,duration Count_=0)
				:pHost(&rHost_), pClient(&rClient_), IsStart(false){
				if(Interval_>=0) {
					pHost->quick_start(*pClient, Interval_, Count_);
					IsStart=true;
				}
			}
			~task_driver(){reset();}
			bool is_start() const { return IsStart;  }
			operator bool() const{ return is_start(); }
			void reset() {
				if(is_start()) {
					pHost->stop(*pClient);
					IsStart=false;
				}
				pHost=0;
				pClient=0;
			}
			void reset(host_interface& rHost_,client_interface& rClient_, duration Interval_,duration Count_=0) {
				reset();
				pHost=&rHost_;
				pClient=&rClient_;
				if(Interval_>=0) {
					pHost->quick_start(*pClient, Interval_, Count_);
					IsStart=true;
				}
			}
			bool start(duration Interval_,duration Count_=0) {
				if(pHost==0 || IsStart)return true;
				if(Interval_<0)return true;
				pHost->quick_start(*pClient, Interval_, Count_);
				IsStart=true;
				return false;
			}
			bool restart(duration Interval_,duration Count_=0) {
				if(Interval_<0) {
					stop();
					return false;
				}
				if(!IsStart)return true;
				pHost->restart(*pClient, Interval_, Count_);
				IsStart=true;
				return false;
			}
			void stop() {
				if(!IsStart)return;
				pHost->stop(*pClient);
				IsStart=false;
			}
		};
	}
}
#
#endif
