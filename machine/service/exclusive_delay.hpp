#ifndef HMR_MACHINE_SERVICE_EXCLUSIVEDELAY_INC
#define HMR_MACHINE_SERVICE_EXCLUSIVEDELAY_INC 100
#
namespace hmr {
	namespace machine {
		namespace service {
			// 他の割り込みを禁止する、排他的 delay 関数
			void exclusive_delay_ms(unsigned int msec);
		}
	}
}
#
#endif
