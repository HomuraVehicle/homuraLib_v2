#ifndef HMR_VIC1GATE_INC
#define HMR_VIC1GATE_INC 100
#
/*---hmrVIC1Gate---
vic1のgateインターフェースでのラッピング

===hmrVIC1Gate===
v1_00/13123 hmIto
紛失したので再作成

*/
#include<memory>
#include<thread>
#include<mutex>
#include<chrono>
#include<hmLib/gate.hpp>
#include"hmrVIC1.h"
namespace hmr {
	class vic1_gate :public hmLib::gate {
	private:
		std::unique_ptr<vcom1> pVCom1;
		std::unique_ptr<unsigned char[]> Buf;
		vic1 VIC1;
	public:
		vic1_gate()=default;
		template<typename vcom1_creater,typename timeout_set>
		vic1_gate(unsigned char Ch, vcom1_creater VCom1Creater, unsigned int SeqLength, timeout_set TimeoutSet)
			:pVCom1(VCom1Creater())
			,Buf(new unsigned char[SeqLength]) {
			timeout_set::initialize(&VIC1);
			vic1_initialize(&VIC1, pVCom1.get(), Ch, Buf.get(), Buf.get()+SeqLength, timeout_set::reset, timeout_set::enable, timeout_set::disable);
		}
		//vic1_gate cannot copy
		vic1_gate(const vic1_gate& my_)=delete;
		const vic1_gate& operator=(const vic1_gate& my_)=delete;
		//vic1_gate can move
		vic1_gate(vic1_gate&& my_) :VIC1(my_.VIC1) {
			std::swap(pVCom1, my_.pVCom1);
			std::swap(Buf, my_.Buf);
		}
		const vic1_gate& operator=(vic1_gate&& my_) {
			if(this!=&my_) {
				VIC1=my_.VIC1;
				std::swap(pVCom1, my_.pVCom1);
				std::swap(Buf, my_.Buf);
			}
			return *this;
		}
		/*for VC12 CTR
		vic1_gate(vic1_gate&& my_)=default;
		const vic1_gate& operator=(vic1_gate&& my_)=default;
		*/
		~vic1_gate() { if(is_open())close(); }
	public:
		template<typename vcom1_creater, typename timeout_set>
		void open(vcom1_creater VCom1Creater, timeout_set TimeoutSet, unsigned char Ch, unsigned int SeqLength) {
			hmLib_assert(!is_open(), hmLib::gate_opened_exception, "This gate have already been opened.");

			pVCom1.reset(VCom1Creater());
			if(!is_open())return;
			Buf.reset(new unsigned char[SeqLength]);
			timeout_set::initialize(&VIC1);
			vic1_initialize(&VIC1, pVCom1.get(), Ch, Buf.get(), Buf.get()+SeqLength, timeout_set::reset, timeout_set::enable, timeout_set::disable);
		}
		void close(void) {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");

			vic1_finalize(&VIC1);
			pVCom1.reset(nullptr);
		}
		//gateが開いているかどうかの確認
		bool is_open() override { return static_cast<bool>(pVCom1); }
	public://vic1 functions
		//通信を強制的に終了させる
		void force_end() {
			vic1_force_end(&VIC1);
		}
		//エラーを取得する
		unsigned char error() {
			return vic1_error(&VIC1);
		}
		//エラーをクリアする
		void clear_error() {
			vic1_clear_error(&VIC1);
		}
	public://gate interface
		//受信可能状態かの確認
		bool can_getc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vic1_can_getc(&VIC1)!=0;
		}
		//単byte受信
		char getc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return static_cast<char>(vic1_getc(&VIC1));
		}
		//受信が継続しているかを確認する
		bool flowing() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vic1_flowing(&VIC1)!=0;
		}
		//送信可能状態かの確認
		bool can_putc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vic1_can_putc(&VIC1)!=0;
		}
		//単byte送信
		void putc(char c) {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vic1_putc(&VIC1, c);
		}
		//送信を打ち切る
		void flush() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vic1_flush(&VIC1);
		}
	};
	template<unsigned int timeout_ch_>
	class vic1_thread_timeout_set {
		class task {
		private:
			std::mutex Mx;
			typedef std::chrono::system_clock::time_point time_point;
			time_point Time;
			bool IsJoin;
			vic1* pVIC1;
		public:
			void initialize(vic1* pVIC1_) { pVIC1=pVIC1_; }
			void join() {
				std::lock_guard<std::mutex> Lock(Mx);
				IsJoin=true;
			}
			void reset() {
				std::lock_guard<std::mutex> Lock(Mx);
				Time=std::chrono::system_clock::now();
			}
			void operator()(void) {
				IsJoin=false;
				reset();

				while(true) {
					std::lock_guard<std::mutex> Lock(Mx);
					if(IsJoin)break;
					if(std::chrono::system_clock::now()-Time > std::chrono::seconds(1)) {
						vic1_timeout(pVIC1);
						std::cout<<"************ !!! TIME OUT !!! *************"<<std::endl;
						break;
					}

				}
			}
		};
	private:
		static task Task;
		static std::unique_ptr<std::thread> pThread;
	public:
		static void initialize(vic1* pVIC1_) {
			Task.initialize(pVIC1_);
		}
		static void reset() {
			Task.reset();
		}
		static void enable() {
			if(pThread)return;
			pThread.reset(new std::thread(std::ref(Task)));
//			std::cout<<"***task enable***"<<std::endl;
		}
		static void disable() {
			if(!pThread)return;
			Task.join();
			pThread->join();
			pThread.reset(nullptr);
//			std::cout<<"***task disable***"<<std::endl;
		}
	};
	template<unsigned int timeout_ch_>
	typename vic1_thread_timeout_set<timeout_ch_>::task vic1_thread_timeout_set<timeout_ch_>::Task;
	template<unsigned int timeout_ch_>
	std::unique_ptr<std::thread> vic1_thread_timeout_set<timeout_ch_>::pThread;
	template<unsigned int timeout_ch_>
	class vic1_functional_timeout_set {
		class task {
		private:
			typedef std::chrono::system_clock::time_point time_point;
			time_point Time;
			bool IsEnable;
			vic1* pVIC1;
		public:
			void initialize(vic1* pVIC1_) { pVIC1=pVIC1_; }
			void enable() {
				IsEnable=true;
			}
			void disable() {
				IsEnable=false;
			}
			void reset() {
				Time=std::chrono::system_clock::now();
			}
			void operator()(void) {
				if(!IsEnable)return;

				if(std::chrono::system_clock::now()-Time > std::chrono::seconds(3)) {
					vic1_timeout(&rVIC1);
				}
			}
		};
	private:
		static task Task;
	public:
		static void initialize(vic1* pVIC1_) {
			Task.initialize(pVIC1_);
		}
		static void reset() {
			Task.reset();
		}
		static void enable() {
			Task.reset();
			Task.enable();
		}
		static void disable() {
			Task.disable();
		}
		static void work() {
			Task(); 
		}
	};
	template<unsigned int timeout_ch_>
	typename vic1_functional_timeout_set<timeout_ch_>::task vic1_functional_timeout_set<timeout_ch_>::Task;
}
#
#endif
