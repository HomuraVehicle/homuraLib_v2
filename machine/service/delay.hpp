#ifndef HMR_MACHINE_SERVICE_DELAY_INC
#define HMR_MACHINE_SERVICE_DELAY_INC 100
#
namespace hmr {
	namespace machine {
		namespace service {
			//delay 関数 割り込み中で利用すると、死ぬ
			void delay_ms(unsigned int ms_);
		}
	}
}
#
#endif
