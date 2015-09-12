#ifndef HMR_MACHINE_MODULE_SPRITE_COMMANDUART_INC
#define HMR_MACHINE_MODULE_SPRITE_COMMANDUART_INC 100

#include<XCBase/future.hpp>
#include<XCBase/function.hpp>
#include<XCBase/lock.hpp>
#include<XC32/uart.hpp>
#include<XC32/sfr/interrupt.hpp>
#include"command.hpp"

namespace hmr{
	namespace machine{
		namespace module{
			namespace sprite{
				struct command_uart_status{
				private:
					commands::id::type ID;
					unsigned char Mode;
				public:
					typedef unsigned char mode_type;
					typedef commands::id::type id_type;
				public:
					command_uart_status() :ID(commands::id::null), Mode(0){}
					command_uart_status(commands::id::type ID_, unsigned char Mode_) :ID(ID_), Mode(Mode_){}
					id_type id()const{ return ID; }
					mode_type mode()const{ return Mode; }
				};
				//sprite::commandsを送受信するuart
				//	timer管理, led管理はしないので注意
				template<typename uart_register_>
				class command_uart{
				private:
					typedef command_uart<uart_register_> my_type;
				private:
					//カメラとの通信用Uart
					xc32::interrupt_uart<uart_register_> InterruptUart;
					//InterruptUart用Lock
					xc::unique_lock<xc32::interrupt_uart<uart_register_>> InterruptUartLock;
					//コマンド送信状況　待機:0 送信:1 受信:2
					unsigned char Mode;
					//コマンドカウンター
					unsigned int Cnt;
					//現在の担当コマンド
					command Cmd;
					//commandの返信
					xc::promise<bool> Promise;
				private:
					struct tx_interrupt :public xc32::sfr::interrupt::function{
					private:
						my_type& Ref;
					public:
						tx_interrupt(my_type& Ref_) :Ref(Ref_){}
						virtual void operator()(void){
							//送信処理
							Ref.InterruptUart.send_data(Ref.Cmd.SendStr[Ref.Cnt++]);
							//送信終了処理
							if (Ref.Cnt >= Ref.Cmd.SendSize){
								//LED点灯
								//_spriteitf_led_set(1);
								//カウンターを0に戻す
								Ref.Cnt = 0;
								//割込みモードを受信に変更
								Ref.Mode = 2;
								//送信割り込み禁止
								Ref.InterruptUart.send_disable();
								//受信割り込み許可
								Ref.InterruptUart.recv_enable();
							}
						}
					}TxInterrupt;
					struct rx_interrupt :public xc32::sfr::interrupt::function{
					private:
						my_type& Ref;
					public:
						rx_interrupt(my_type& Ref_) :Ref(Ref_){}
						virtual void operator()(void){
							//1文字受信
							unsigned char c = Ref.InterruptUart.recv_data();
							++(Ref.Cnt);

							//正しく受信コマンド先頭を読んでいなければ、読み捨て
							if (Ref.Cnt == 1 && c != 0x76){
								//カウント0にして読んだ値は無視する
								Ref.Cnt = 0;
								return;
							}

							//受信時呼出し関数
							if (Ref.Cmd.pRecvFunc){
								//終了が要求が行われた場合、Cntを直ちにMaxに変更
								if ((*Ref.Cmd.pRecvFunc)(c, Ref.Cnt - 1))Ref.Cnt = Ref.Cmd.RecvSize;
							}

							//受信終了処理
							if (Ref.Cnt >= Ref.Cmd.RecvSize){
								//割込みモードを通常に変更
								Ref.Mode = 0;
								//カウンターを0に戻す
								Ref.Cnt = 0;
								//受信割り込み禁止
								Ref.InterruptUart.recv_disable();
								//コマンド終了通知
								Ref.Promise.set_value(false);
							}
						}
					}RxInterrupt;
				public:
					command_uart()
						: InterruptUartLock(InterruptUart, true)
						, TxInterrupt(*this)
						, RxInterrupt(*this)
						, Mode(0)
						, Cnt(0){
						InterruptUart.config(38400, xc32::uart::flowcontrol::no_control, TxInterrupt, RxInterrupt);
					}
				public:
					bool lock(){
						if (is_lock())return false;
						if (InterruptUartLock.lock())return true;

						Mode = 0;
						Cnt = 0;
						Cmd = command();

						return false;
					}
					void unlock(){
						if (!Promise.can_get_future()){
							Promise.set_value(true);
						}
						InterruptUartLock.unlock();
					}
					bool is_lock()const{return InterruptUartLock;}
				public:
					//command実行 失敗したら、trueを返す
					xc::future<bool> async_command(const command& Command_){
						if (is_command() || !Promise.can_get_future())xc::future<bool>();

						Mode = 1;
						Cnt = 0;
						Cmd = Command_;

						if (!Cmd.valid()){
							xc::future<bool> ans=Promise.get_future();
							Promise.set_value(true);
							return ans;
						}

						InterruptUart.send_set_flag();
						InterruptUart.send_enable();

						return Promise.get_future();
					}
					//command実行中か
					bool is_command()const{ return Mode > 0; }
					//現在のコマンド取得
					command_uart_status status()const{
						return command_uart_status(static_cast<commands::id::type>(Cmd.ID),Mode);
					}
					//command実行をキャンセル
					void cancel_command(){
						InterruptUart.recv_disable();
						InterruptUart.send_disable();

						Mode = 0;
						Cnt = 0;

						Promise.set_value(true);
					}
				};
			}
		}
	}
}
#
#endif
