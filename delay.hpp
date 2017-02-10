#ifndef HMR_DELAY_INC
#define HMR_DELAY_INC 100
#
namespace hmr{
	struct delay_interface{
	public:
		typedef unsigned int duration;
	public:
		virtual void delay_ms(duration ms) = 0;
		virtual void exclusive_delay_ms(duration ms) = 0;
	};
}
#
#endif
