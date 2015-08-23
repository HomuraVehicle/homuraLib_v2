#ifndef HMR_MESSAGE_INTERCEPTOR_INC
#define HMR_MESSAGE_INTERCEPTOR_INC 100
#
#include"io_interface.hpp"
namespace hmr {
	namespace message {
		class interceptor_interface : public io_interface {
		private:
			io_interface* pIO;
		public:
			interceptor_interface() :pIO(0) {}
		private://interceptor
			virtual void interceptor_put(const data& rData_)=0;
			virtual void interceptpr_get(const data& rData_)=0;
		public:
			void open(io_interface& rIO_) {
				if(is_open())return;

				pIO=&rIO_;
			}
			void close() {
				pIO=0;
			}
			bool is_open() const {
				return pIO!=0;
			}
		protected:
			virtual bool io_can_put() {
				return pIO->can_put();
			}
			virtual bool io_can_put(const data& rData_) {
				return pIO->can_put(rData_);
			}
			virtual bool io_put(const data& rData_) {
				interceptor_put(rData_);
				return pIO->put(rData_);
			}
			virtual void io_flush() {
				pIO->flush();
			}
		protected:
			virtual bool io_can_get() {
				return pIO->can_get();
			}
			virtual bool io_get(data& rData_) {
				interceptpr_get(rData_);
				return pIO->get(rData_);
			}
			virtual bool io_flowing() {
				return pIO->flowing();
			}
		};
	}
}
#
#endif
