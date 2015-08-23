#ifndef HMR_MACHINE_SERVICE_LOCKCODE_INC
#define HMR_MACHINE_SERVICE_LOCKCODE_INC 100
#
#include <cstdlib>
namespace hmr {
	namespace machine {
		namespace service {
			//ÉçÉbÉNê∂ê¨ä÷êî
			inline unsigned char lockcode(void){
				unsigned char Key;
				do{
					Key=(unsigned char)(rand());
				}while(Key==0);
				return Key;
			}
		}
	}
}
#
#endif
