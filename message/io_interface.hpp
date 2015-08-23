#ifndef HMR_MESSAGE_MESSAGEINTERFACE_INC
#define HMR_MESSAGE_MESSAGEINTERFACE_INC 101
#
/*
=== message_reference ===
v1_01/140525 hmIto
	NVI‚É]‚Á‚ÄAvirtualŠÖ”‚ğ”ñŒöŠJ‰»
*/
#include<XCBase/bytes.hpp>
namespace hmr {
	namespace message {
		class message_recv_interface {
		protected:
			virtual bool message_can_put()=0;
			virtual bool message_can_put(const bytes& rData_) { return can_put(); }
			virtual bool message_put(bytes Data_)=0;
			virtual void message_flush()=0;
		public:
			bool can_put() { return message_can_put(); }
			bool can_put(const data& rData_) { return message_can_put(rData_); }
			bool put(data Data_) { return message_put(Data_); }
			void flush() { message_flush(); }
		};
		class message_send_interface {
		protected:
			virtual bool message_can_get()=0;
			virtual bool message_get(data Data_)=0;
			virtual bool message_flowing()=0;
		public:
			bool can_get() {return message_can_get();}
			bool get(data Data_) { return message_get(xc::move(Data_)); }
			bool flowing() { return message_flowing(); }
		};
		class message_interface :public message_send_interface, public message_recv_interface{
		protected:
			virtual bool message_can_put()=0;
			virtual bool message_can_put(const data& rData_) { return can_put(); }
			virtual bool message_put(const data& rData_)=0;
			virtual void message_flush()=0;
		protected:
			virtual bool message_can_get()=0;
			virtual bool message_get(data& rData_)=0;
			virtual bool message_flowing()=0;
		};
	}
}
#
#endif
