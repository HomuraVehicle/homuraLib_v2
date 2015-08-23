#ifndef HMR_IO_IOINTERFACE_INC
#define HMR_IO_IOINTERFACE_INC 100
#
namespace hmr {
	namespace io {
		struct io_interface {
			virtual ~io_interface() {}
		public: //io_interface
			virtual bool start()=0;
			virtual bool is_start()=0;
			virtual void stop()=0;
		};
	}
}
#
#endif
