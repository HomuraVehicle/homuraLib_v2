#ifndef HMR_MESSAGE_EXCEPTION_INC
#define HMR_MESSAGE_EXCEPTION_INC 100
#
#include<homuraLib_v2/exception.hpp>
namespace hmr {
	namespace message{
		namespace exceptions {
			class message_exception :public exception {
				typedef exception base_type;
			public:
				message_exception() :base_type() {}
				message_exception(unsigned char Code_) :base_type(Code_) {}
			};
			class not_opend :public message_exception {
				typedef message_exception base_type;
			public:
				not_opend() :base_type() {}
				not_opend(unsigned char Code_) :base_type(Code_) {}
			};
			class opened :public message_exception {
				typedef message_exception base_type;
			public:
				opened() :base_type() {}
				opened(unsigned char Code_) :base_type(Code_) {}
			};
		}
	}
}
#
#endif
