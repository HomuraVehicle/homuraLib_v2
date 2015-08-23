#ifndef HMR_TASK_WATCHDOGTASK_INC
#define HMR_TASK_WATCHDOG_INC
#
#include"client_interface.hpp"
namespace hmr{
	namespace task{
		struct watchdog :public client_interface{
		private:
			//timeoutŒn
			unsigned int Count;
			unsigned int Limit;
		public:
			watchdog()
				: Count(0)
				, Limit(0) {
			}
			void enable(unsigned int Limit_){
				Limit = Limit_;
			}
			void disable(){
				Limit = 0;
				Count = 0;
			}
			void restart(){
				Count = 0;
			}
		public://client_interface
			virtual duration operator()(duration Duration_){
				if(Limit>0){
					Count += Duration_;
					if(Count >= Limit){
						timeout();
					}
				}
				return 1;
			}
		public:
			virtual void timeout() = 0;
		};
	}
}
#
#endif
