#ifndef HMR_MESSAGE_TWOWAYQUE_INC
#define HMR_MESSAGE_TWOWAYQUE_INC 100
#
#include<memory>
#include<deque>
#include"oneway_queue.hpp"
#include"io_reference.hpp"
namespace hmr {
	namespace message {
		class twoway_queue {
		private:
			oneway_queue UpWay;
			oneway_queue DownWay;
		public:
			io_reference UpIO;
			io_reference DownIO;
		public:
			twoway_queue(unsigned int UpMaxSize_, unsigned int DownMaxSize_)
				: UpWay(UpMaxSize_)
				, DownWay(DownMaxSize_)
				, UpIO(DownWay.Input, UpWay.Output)
				, DownIO(UpWay.Input, DownWay.Output){
			}
			void clearUp() { UpWay.clear(); }
			bool emptyUp()const { return UpWay.empty(); }
			unsigned int sizeUp()const { return UpWay.size(); }
			const data& frontUp()const { return UpWay.front(); }
			const data& backUp()const { return UpWay.back(); }
			void clearDown() { DownWay.clear(); }
			bool emptyDown()const { return DownWay.empty(); }
			unsigned int sizeDown()const { return DownWay.size(); }
			const data& frontDown()const { return DownWay.front(); }
			const data& backDown()const { return DownWay.back(); }

		};
	}
}
#
#endif
