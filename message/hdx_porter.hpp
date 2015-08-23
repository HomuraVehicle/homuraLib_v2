#ifndef HMR_MESSAGE_FDXPORTER_INC
#define HMR_MESSAGE_FDXPORTER_INC 100
#
#include"io_interface.hpp"
#include"buffer.hpp"
namespace hmr {
	namespace message {
		class fdx_porter : public io_interface {
		private:
			io_interface* pIO;
			buffer Buffer;
			bool DownFlush;
			bool UpFlush;
			bool IsSendActiveFlag;
		public:
			virtual bool can_put() {
				return Buffer.UpIO.can_put();
			}
			virtual bool can_put(const data& rData_) {
				return Buffer.UpIO.can_put(rData_);
			}
			virtual bool put(const data& rData_) {
				return Buffer.UpIO.put(rData_);
			}
			virtual bool put(data::rvalue_reference mData_) {
				return Buffer.UpIO.put(mData_);
			}
			virtual void flush() {
				Buffer.UpIO.flush();
			}
		public:
			virtual bool can_get() {
				return Buffer.UpIO.can_get();
			}
			virtual bool get(data& rData_) {
				return Buffer.UpIO.get(rData_);
			}
			virtual bool flowing() {
				return Buffer.UpIO.flowing();
			}
		public:
			void open(io_interface* pIO_) {
				if(is_open())return;

				pIO=pIO_;
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

				// 送信可能でなければ、終了
				if(!IsSendActiveFlag)return;

				//送信(上り)処理
				while(Buffer.DownIO.can_put() && pIO->can_get()) {
					if(!UpFlush && !pIO->flowing()) {
						Buffer.DownIO.flush();
						UpFlush=true;
						IsSendActiveFlag=false;
						break;
					}

					data Data;
					pIO->get(Data);
					Buffer.DownIO.put(hmLib::move(Data));

					if(!pIO->flowing()) {
						Buffer.DownIO.flush();
						Flush=true;
						IsSendActiveFlag=false;
						break;
					} else {
						Flush=false;
					}
				}
			}
		};
	}
}
#
#endif
