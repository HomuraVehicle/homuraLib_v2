#ifndef HMR_MESSAGE_HOST_INC
#define HMR_MESSAGE_HOST_INC 100
#
#include<vector>
#include<memory>
#include"io_interface.hpp"
#include"io_array.hpp"
#include"client.hpp"
namespace hmr {
	namespace message {
		class host :public io_interface {
			typedef io_array containor;
			typedef containor::iterator iterator;
		private:		
			containor Clients;
			iterator TalkItr;
			bool IsTalking;
			int SendableCount;
		public:
			typedef containor::builder builder;
			host():IsTalking(false), SendableCount(0) {}
			host(builder& Builder_) :Clients(Builder_()), IsTalking(false), SendableCount(0) {
				TalkItr=Clients.begin();
			}
			const host& operator=(containor::precursor Prescursor_) {
				Clients=Prescursor_;
				TalkItr=Clients.begin();
			}
		private:
			host(const host&);
			const host& operator=(const host&);
		public:
			void setSendNum(int Val_) {
				SendableCount=Val_;
			}
			int getSendNum()const {
				return SendableCount;
			}
		protected:
			virtual bool io_can_put() { return true; }
			virtual bool io_put(const data& rData_) {
				containor::iterator Itr=Clients.find(rData_.getID());
				if(Itr==Clients.end())return true;
				if(Itr->can_put()==false)return true;
				return Itr->put(rData_);
			}
			virtual void io_flush() {
				for(containor::iterator itr=Clients.begin(); itr!=Clients.end(); ++itr) {
					itr->flush();
				}
			}
			virtual bool io_can_get() { return SendableCount!=0; }
			virtual bool io_get(data& rData_) {
				if(!IsTalking) {
					TalkItr=Clients.begin();
					IsTalking=true;
				}
				while(TalkItr!=Clients.end()) {
					if(TalkItr->can_get()) {
						++TalkItr;
						continue;
					}
					if(TalkItr->get(rData_)) {
						rData_.release();
						++TalkItr;
						continue;
					}
					break;
				}

				if(TalkItr==Clients.end()) {
					IsTalking=false;
					if(SendableCount>0)--SendableCount;
					return false;
				} else {
					return rData_;
				}
			}
			virtual bool io_flowing() { return IsTalking; }
		};
	}
}
#
#endif
