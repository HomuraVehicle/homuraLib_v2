#ifndef HMR_MESSAGE_BUFFER_INC
#define HMR_MESSAGE_BUFFER_INC 100
#
#include"io_interface.hpp"
#include"twoway_queue.hpp"
namespace hmr {
	namespace message {
		class buffer : public io_interface {
		private:
			io_interface* pIO;
			twoway_queue Buffer;
			bool DownFlush;
			bool UpFlush;
		public:
			buffer(unsigned int UpMaxSize_, unsigned int DownMaxSize_)
				: pIO(0)
				, Buffer(UpMaxSize_, DownMaxSize_)
				, DownFlush(true)
				, UpFlush(true) {
			}
		protected:
			virtual bool io_can_put() {
				return Buffer.UpIO.can_put(); 
			}
			virtual bool io_can_put(const data& rData_) {
				return Buffer.UpIO.can_put(rData_);
			}
			virtual bool io_put(const data& rData_) {
				return Buffer.UpIO.put(rData_);
			}
			virtual void io_flush() {
				Buffer.UpIO.flush();
			}
		protected:
			virtual bool io_can_get() {
				return Buffer.UpIO.can_get();
			}
			virtual bool io_get(data& rData_) {
				return Buffer.UpIO.get(rData_);
			}
			virtual bool io_flowing() {
				return Buffer.UpIO.flowing();
			}
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
		public:
			void operator()(void) {
				if(!is_open())return;

				//受信(下り)処理
				while(Buffer.DownIO.can_get() && pIO->can_put()) {
					if(!DownFlush && !Buffer.DownIO.flowing()) {
						pIO->flush();
						DownFlush=true;
					}

					data Data;
					Buffer.DownIO.get(Data);
					pIO->put(hmLib::move(Data));

					if(!Buffer.DownIO.flowing()) {
						pIO->flush();
						DownFlush=true;
					} else {
						DownFlush=false;
					}
				}

				//送信(上り)処理
				while(Buffer.DownIO.can_put() && pIO->can_get()) {
					if(!UpFlush && !pIO->flowing()) {
						Buffer.DownIO.flush();
						UpFlush=true;
					}

					data Data;
					pIO->get(Data);
					Buffer.DownIO.put(hmLib::move(Data));

					if(!pIO->flowing()) {
						Buffer.DownIO.flush();
						UpFlush=true;
					} else {
						UpFlush=false;
					}
				}
			}
		};
	}
}
#
#endif
