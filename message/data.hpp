#ifndef HMR_MESSAGE_DATA_INC
#define HMR_MESSAGE_DATA_INC 101
#
#include<XCBase/bytes.hpp>
namespace hmr {
	namespace message {
		namespace data_id {
			enum special_id {
				null=0x00,
				eof=0x0a
			};
		}
		namespace error {
			enum error_code {
				null	=0x00,		//Dataにエラーがない
				stranger=0x01,		//Dataの送信元CHがおかしい
				unknown	=0x02,		//Dataの識別IDがおかしい
				failnew	=0x04,		//Dataの確保に失敗した
				sizeover=0x08,		//DataのSizeが大きすぎる
				overflow=0x10,		//DataがSizeに比べて多すぎる
				underflow=0x20,		//DataがSizeに比べて少なすぎる
				broken=0x40			//Dataが壊れている
			};
		}
		struct data {
		private:
			typedef data my_type;
		public:
			typedef unsigned char id_type;
			typedef unsigned char error_type;
		public:
			typedef my_type& reference;
			typedef const my_type& const_reference;
			typedef hmLib::rvalue_reference<my_type> rvalue_reference;
		public:
			static my_type make_eof() { return my_type(data_id::eof, data_array(), error::null); }
		private:
			id_type ID;
			error_type Error;
			bytes Data;
		public:
			data() :ID(data_id::null), Error(error::null) {}
			data(rvalue_reference mData_) 
				: ID(mData_.ref.ID)
				, Error(mData_.ref.Error)
				, Data(xc::move(mData_.ref.Data)) {
			}
			reference operator=(rvalue_reference mData_) {
				if(&(mData_.ref) !=this) {
					ID=mData_.ref.ID;
					Error=mData_.ref.Error;
					Data=xc::move(mData_.ref.Data);
				}
				return *this;
			}
			data(id_type ID_, bytes Data_, error_type Error_)
				: ID(ID_)
				, Error(Error_)
				, Data(xc::move(Data_)) {
			}
		private:
			data(const_reference Data_) 
				: ID(Data_.ID)
				, Error(Data_.Error)
				, Data(Data_.Data) {
			}
			const_reference operator=(const_reference Data_) {
				if(&Data_!=this) {
					ID=Data_.ID;
					Error=Data_.Error;
					Data=Data_.Data;
				}
				return *this;
			}
		public:
			void set(id_type ID_, bytes Data_, error_type Error_) {
				ID=ID_;
				Error=Error_;
				Data=xc::move(Data_);
			}
			void clear() {
				ID=data_id::null;
				Error=error::null;
				Data.clear();
			}
			bytes release(){
				ID=data_id::null;
				Error=error::null;
				return xc::move(Data);
			}
			operator bool() const { return ID!=data_id::null; }
			bool eof()const { return ID==data_id::eof; }
			id_type getID() const { return ID; }
			const bytes& getData()const { return Data; }
			unsigned char operator[](unsigned int Pos_) const { return Data[Pos_]; }
			unsigned int size()const { return Data.size(); }
		public:
			bool isError()const { return Error!=error::null; }
			error_type getError()const { return Error; }
			void clearError() { Error=error::null; }
		};
	}
}
#
#endif
