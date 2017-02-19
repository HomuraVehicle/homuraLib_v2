#ifndef HMR_MACHINE_SERVICE_SAFECSTRING_INC
#define HMR_MACHINE_SERVICE_SAFECSTRING_INC 100
#
#include <hmLib/cstring.h>
#include <XC32/interrupt.hpp>
namespace hmr {
	namespace machine {
		namespace service {
			//割り込み安全なcstring動的生成関数
			inline void cstring_construct_safe(hmLib::cstring* str,hmLib::cstring_size_t size){
				xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);
				hmLib::cstring_construct(str,size);
			}
			//割り込み安全なcstring破棄関数
			inline void cstring_destruct_safe(hmLib::cstring* str){
				xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);
				hmLib::cstring_destruct(str);
			}
		}
	}
}
#
#endif
