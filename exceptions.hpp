#ifndef HMR_EXCEPTION_INC
#define HMR_EXCEPTION_INC 100
#
#include<homuraLib_config.hpp>
#include<XCBase/exceptions.hpp>
namespace hmr {
	class exception :public xc::exception{
	public:
		exception() :xc::exception(){}
		exception(unsigned char Code_) :xc::exception(Code_) {}
	};
}
#ifndef  HMR_NOEXCEPT
#	define hmr_throw(except) throw except
#	define hmr_assert(condition,except) if(!(condition))hmr_throw(except)
#else
#	define hmr_throw(except)
#	define hmr_assert(condition,except)
#endif
#
#endif
