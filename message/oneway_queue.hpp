#ifndef HMR_MESSAGE_ONEWAYQUEUE_INC
#define HMR_MESSAGE_ONEWAYQUEUE_INC 100
#
#include<deque>
#include<memory>
#include<homuraLib_config/allocator.hpp>
#include"data.hpp"
#include"io_interface.hpp"
namespace hmr {
	namespace message {
		class oneway_queue {
			typedef std::deque<data, allocator::rebind<data>::other> containor;
		public:
			class input :public input_interface {
				containor& Que;
				unsigned int MaxSize;
				bool Flush;
			public:
				input(containor& Que_, unsigned int MaxSize_) :Que(Que_), MaxSize(MaxSize_), Flush(true){}
			protected:
				virtual bool io_can_put() { return Que.size()<MaxSize; }
				virtual bool io_put(const data& rData_) {
					if(can_put())return true;
					Que.push_back(rData_);
					Flush=false;
					return false;
				}
				virtual void io_flush() {
					if(Flush)return;
					Que.push_back(data::make_eof()); 
					Flush=true;
				}
			};
			class output :public output_interface {
				containor& Que;
				bool Flowing;
			public:
				output(containor& Que_) :Que(Que_), Flowing(false){}
			protected:
				virtual bool io_can_get() {
					if(Que.empty())return false;
					if(!Que.front().eof())return true;

					Que.pop_front();
					Flowing=true;
					
					return can_get();
				}
				virtual bool io_get(data& rData_) {
					if(!can_get())return true;

					rData_=Que.front();
					Que.pop_front();
					Flowing=false;

					return false;
				}
				virtual bool io_flowing() {
					can_get();
					return Flowing; 
				}
			};
		private:
			containor Que;
		public:
			input Input;
			output Output;
		public:
			oneway_queue(unsigned int MaxSize_)
				: Input(Que,MaxSize_)
				, Output(Que)
				, Que(MaxSize_,data()){
				Que.clear();
			}
			void clear() { Que.clear(); }
			bool empty()const { return Que.empty(); }
			unsigned int size()const { return Que.size(); }
			const data& front()const { return Que.front(); }
			const data& back()const { return Que.back(); }
		};
	}
}
#
#endif
