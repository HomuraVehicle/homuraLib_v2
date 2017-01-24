#ifndef HMR_LOCKCODE_INC
#define HMR_LOCKCODE_INC 100
#
#include <cstdlib>
namespace hmr {
	//ƒƒbƒN¶¬ŠÖ”
	inline unsigned char lockcode(void){
		unsigned char Key;
		do{
			Key=(unsigned char)(rand());
		}while(Key==0);
		return Key;
	}
}
#
#endif
