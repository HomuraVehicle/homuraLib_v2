#ifndef HMR_MACHINE_SERVICE_SAFECSTRING_INC
#define HMR_MACHINE_SERVICE_SAFECSTRING_INC 100
#
#include <hmLib/cstring.h>
#include <XC32Lib/interrupt.hpp>
namespace hmr {
	namespace machine {
		namespace service {
			//���荞�݈��S��cstring���I�����֐�
			inline void cstring_construct_safe(hmLib::cstring* str,hmLib::cstring_size_t size){
				xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);
				hmLib::cstring_construct(str,size);
			}
			//���荞�݈��S��cstring�j���֐�
			inline void cstring_destruct_safe(hmLib::cstring* str){
				xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);
				hmLib::cstring_destruct(str);
			}
		}
	}
}
#
#endif
