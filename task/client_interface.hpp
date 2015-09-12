#ifndef HMR_TASK_CLIENTINTERFACE_INC
#define HMR_TASK_CLIENTINTERFACE_INC 100
#
#include<hmLib/type.h>
namespace hmr{
	namespace task {
		typedef hmLib::type::sint32 duration;
		class client_interface {
		public:
			typedef hmr::task::duration duration;
			virtual ~client_interface() {}
		public://client_interface
			virtual duration operator()(duration Duration_)=0;
		};
	}
}
#
#endif
