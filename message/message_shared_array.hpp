#ifndef HMR_MACHINE_MESSAGESHAREDARRAY_INC
#define HMR_MACHINE_MESSAGESHAREDARRAY_INC
#
#include <memory>
#include <hmLib_v3_06/lab/memory/shared_array.hpp>
#include <homuraLib_v2/shared_array.hpp>
#include <homuraLib_config/message_data_allocator.hpp>
namespace hmr {
	namespace message {
		typedef shared_array_builder<data_allocator> local_shared_array_builder;
		typedef local_shared_array_builder::shared_read_array shared_read_array;
		typedef local_shared_array_builder::shared_write_array shared_write_array;
		typedef hmLib::rvalue_reference<shared_read_array> shared_read_array_rvalue_reference;
		inline shared_write_array make_shared_array(unsigned int Size_) { return local_shared_array_builder()(Size_); }
	}
}
#
#endif
