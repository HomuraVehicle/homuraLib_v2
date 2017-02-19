#ifndef HMR_LOCKCODE_INC
#define HMR_LOCKCODE_INC 100
#
#include <cstdlib>
namespace hmr {
	//ロック生成関数
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
