#ifndef HMR_MESSAGE_CLIENT_INC
#define HMR_MESSAGE_CLIENT_INC 100
#
#include"data.hpp"
#include"io_interface.hpp"
namespace hmr {
	namespace message {
		class client :public io_interface{
		public:
			typedef data::wdata_array wdata_array;
			typedef data::data_array data_array;
			typedef data::data_array_rvalue_reference data_array_rvalue_reference;
		protected://message::client
			virtual void setup_talk() {}
			virtual data_array talk(void)=0;
			virtual void setup_listen() {}
			virtual void listen(const data_array& Array_)=0;
		private:
			data::id_type ID;
			bool IsListening;
			data_array Array;
			bool IsTalking;
		public:
			client(data::id_type ID_) 
				:ID(ID_)
				,IsListening(false)
				,IsTalking(false){
			}
			virtual ~client() {}
			data::id_type getID() const { return ID; }
		protected:
			virtual bool io_can_put() { return true; }
			virtual bool io_put(const data& rData_) {
				if(!IsListening) {
					IsListening=true;
					setup_listen();
				}
				listen(rData_.getData());
				return false;
			}
			virtual void io_flush() {
				IsListening=false;
			}
			virtual bool io_can_get() {
				//Arrayがからの場合、確認しに行く
				if(!Array) {
					if(!IsTalking) {
						IsTalking=true;
						setup_talk();
					}

					Array=talk();
					if(Array) {
						IsTalking=false;
					}
				}
				//Arrayが確保済みかどうか
				return static_cast<bool>(Array);
			}
			virtual bool io_get(data& rData_) {
				rData_.set(ID, hmLib::move(Array), error::null);
				return false;
			}
			virtual bool io_flowing() {
				return IsTalking;
			}
		};
	}
}
#
#endif
