#ifndef HMR_CHRONO_TIMERINTERFACE_INC
#define HMR_CHRONO_TIMERINTERFACE_INC 200
#

/*=== homura chrono ===
時刻提供機能
	now関数で現在時刻を提供する
	timer_registerクラスを一つ占有する

=== chrono ===
v2_00/140712 hmIto
	cpp化
===hmrChrono===
v1_03/140226 hmIto
	時計機能の提供モジュールとして、chronoに名前変更
		timerを一つ独占
		時刻補正機能/提供機能を持つ
		毎秒実行タスクも予約を受け持つ
		タイムスタンプという概念がなくなったため、メッセージは廃止
===hmrTimeStamp===
v1_02/131202 hmIto
	タイムスタンプの意味合いを変更　詳しくは上記の使い方の通り
v1_01/131201 hmIto
	タイムスタンプから、関数登録機能を除外
		関数登録＆実行はtaskに委譲
v1_00/131123 hmito
	作成
*/
#include "homuraLib_config.hpp"
#ifndef HMR_NOXC32LIB
#include<hmLib/type.h>
#include<XCBase/lock.hpp>
#include<XC32/interrupt.hpp>
#include<XC32/timer.hpp>
#include"time_point.hpp"
#include"clock_interface.hpp"
namespace hmr {
	namespace chrono {
		template<typename timer_register_>
		class timer_clock:public clock_interface{
			typedef timer_clock<timer_register_> my_type;
		public:
			struct alarm_function {
				virtual void operator()(void)=0;
			};
		private:
			struct interrupt :public xc32::interrupt::function {
			private:
				xc32::interrupt_timer<timer_register_> Timer;
				xc::unique_lock<xc32::interrupt_timer<timer_register_>> TimerLock;
				uint32	UnixTime;
				bool RequestCorrection;
				sint32 DifSec;
				uint16 AddMSec;
				alarm_function* pFn;
			public:
				interrupt() :pFn(0), TimerLock(Timer,true) {
					Timer.config(1000,*this);
				}
			public:
				bool lock() {
					if(TimerLock.lock())return true;

					Timer.start();
					return false;
				}
				void unlock() { 
					if(TimerLock)TimerLock.unlock();
				}
				bool is_lock() const{
					return TimerLock;
				}
			public:
				time_point now()const{
					time_point Now;
					do {
						Now.Time=UnixTime;
						Now.MSec= static_cast<msec_time_point>(Timer.count()*1000/Timer.period());
					} while(Now.Time!=UnixTime);
					return Now;
				}
				unix_time_point now_unix()const{
					return UnixTime;
				}
			public://interrupt::function
				void operator()(void) {
					//時刻補正
					if(RequestCorrection) {
						//フラグを下す
						RequestCorrection = false;

						//実際に補正
						UnixTime += DifSec;

						//msの微修正
						//	誤差を最小にするために、被除数に除数の半分を足してから割っている
						Timer.set_count((AddMSec * Timer.period()*2 + 1000)/2000);

						Timer.clear_count();
						Timer.clear_flag();
						return;
					}

					//タイマーカウントアップ
					++UnixTime;

					//秒タスク実行
					if(pFn) (*pFn)();

					Timer.clear_count();
					Timer.clear_flag();
				}
				void reset_func(alarm_function* pFn_=0) { pFn=pFn_; }
				void correct(sint32 DifSec_, uint16 AddMSec_) {
					DifSec=DifSec_;
					AddMSec=AddMSec_;
					RequestCorrection=true;
				}
			};
		private:
			interrupt Interrupt;
		public:
			bool lock() {return Interrupt.lock();}
			void unlock() { Interrupt.unlock();}
			bool is_lock() const{return Interrupt.is_lock();}
		public://clock_interface
			//現在の時刻を取得
			time_point now(void) const{return Interrupt.now();}
			//現在のunixタイムを取得
			unix_time_point now_unix(void) const{return Interrupt.now_unix();}
		public:
			//クロノに毎秒タスクを登録
			void set_sec_alarm(alarm_function& rFp_) {Interrupt.reset_func(&rFp_);}
			//クロノの時刻を修正する
			void correct(sint32 DifSec_, uint16 AddMSec_) {Interrupt.correct(DifSec_, AddMSec_);}
		};
	}
}
#endif
#
#endif
