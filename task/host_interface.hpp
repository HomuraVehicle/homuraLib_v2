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
			struct handler{
			private:
				host_interface* pHost;
				client_interface* pClient;
			public:
				handler():pHost(0), pClient(0){}
				handler(host_interface& Host_, client_interface& Client_):pHost(&Host_),pClient(&Client_){}
			public:
				operator bool(){ return pHost != 0; }
				bool is_start()const{
					if(pHost == 0)return false;
					return pHost->is_start(*pClient);
				}
				bool restart(duration Interval_, duration Count_ = 0){
					if(pHost == 0)return true;
					return pHost->restart(*pClient, Interval_, Count_);
				}
				bool stop(){
					if(pHost == 0)return true;
					return pHost->stop(*pClient);
				}
			};
		public:
			virtual handler start(client_interface& Client_, duration Interval_, duration Count_)=0;
			virtual handler quick_start(client_interface& Client_, duration Interval_, duration Count_=0)=0;
			virtual bool is_start(client_interface& Client_)const=0;
			virtual bool restart(client_interface& Client_, duration Interval_, duration Count_=0)=0;
			virtual bool stop(client_interface& Client_)=0;
		};
		typedef host_interface::handler handler;
	}
}
#
#endif
