#ifndef HMR_MACHINE_MODULE_GPS52D_INC
#define HMR_MACHINE_MODULE_GPS52D_INC 101
#
#include<XCBase/function.hpp>
#include<XC32Lib/uart.hpp>
#include<XC32Lib/interrupt.hpp>
#include<homuraLib_v2/gps.hpp>
namespace hmr {
	namespace machine {
		namespace module {
			template<typename uart_register_>
			class cGPS52D_Uart {
			private:
				typedef uart_register_ uart_register;
				typedef cGPS52D_Uart<uart_register> my_type;
				typedef xc32::interrupt_uart<uart_register> interrupt_uart;
			public:
				typedef xc::function<void(gps::data)> observer;
			private:
				//UART通信
				interrupt_uart InterruptUart;
				xc::unique_lock<interrupt_uart> InterruptUartLock;

				//取得したGPSデータ
				gps::data Data;
				void write(const gps::data& Data_) { Data = Data_; }

				//データ受信オブザーバ
				observer Observer;
			private:
				//受信用割り込み処理
				struct recv_interrupt :public xc32::interrupt::function {
				private:
					my_type& ref;
					unsigned char Buf[85];
					unsigned int Cnt;
					bool IsActive;
				public:
					recv_interrupt(my_type& ref_)
						: ref(ref_)
						, Cnt(0)
						, IsActive(false){
						Buf[0] = '$';
						Buf[1] = 'G';
						Buf[2] = 'P';
						Buf[3] = 'G';
						Buf[4] = 'G';
						Buf[5] = 'A';
						Buf[6] = ',';
					}
					void active(){
						if (is_active())return;
						ref.InterruptUart.recv_enable();
						IsActive = true;
					}
					void inactive(){
						if (!is_active())return;
						ref.InterruptUart.recv_disable();
						IsActive = false;
					}
					bool is_active()const{ return IsActive; }
				public:
					//$GPGGA開始コードを元に、必要部分のデータのみ読み出す
					virtual void operator()(void) {
						//受信
						unsigned char c = ref.InterruptUart.recv_data();

						if (Cnt<7) {
							if (Buf[Cnt] == c){
								++Cnt;
							}
							else if (Buf[0] == c){
								Cnt = 1;
							}
							else {
								Cnt = 0;
							}
						}
						else{
							Buf[Cnt] = c;
							++Cnt;

							if (Cnt == 85) {
								if (ref.Observer)ref.Observer(gps::data::fromGPGGA(Buf, Buf + 85));
								else ref.write(gps::data::fromGPGGA(Buf, Buf + 85));
								Cnt = 0;
							}
						}
					}
				}RecvInterrupt;
				//送信用割り込み処理
				struct send_interrupt : public xc32::interrupt::function {
				public:
					//送信割り込みは空（コマンドをGPSモジュールに送らないため）
					virtual void operator()(void) {}
				}SendInterrupt;
			public:
				cGPS52D_Uart()
					: InterruptUart()
					, InterruptUartLock(InterruptUart, true)
					, RecvInterrupt(*this){
					InterruptUart.config(9600, xc32::uart::flowcontrol::no_control, SendInterrupt, RecvInterrupt);
				}
			public:
				void config(const observer& Observer_){
					Observer = Observer_;
				}
				bool lock(const observer& Observer_){
					config(Observer_);
					return lock();
				}
				bool lock(){
					if (InterruptUartLock)return false;
					if(InterruptUartLock.lock())return true;

					RecvInterrupt.active();
					return false;
				}
				bool is_lock()const{ return InterruptUartLock; }
				void unlock(){
					if (!InterruptUartLock)return;
					RecvInterrupt.inactive();
					InterruptUartLock.unlock();
				}
			public:
				//有効なGPSデータが存在するか
				bool can_read()const {
					return Data.is_valid();
				}
				//取得したGPSデータを読み出す
				gps::data read(){
					gps::data tmp = Data;
					Data.invalid();
					return tmp;
				}
			};
			template<typename uart_register_, typename power_pin_>
			class cGPS52D {
			private:
				typedef uart_register_ uart_register;
				typedef power_pin_ power_pin;
				typedef cGPS52D<uart_register,power_pin> my_type;

			private:
				//GPSモジュール管理
				cGPS52D_Uart<uart_register> GPS52DUart;
				xc::unique_lock<cGPS52D_Uart<uart_register>> GPS52DUartLock;
				//電源管理
				power_pin PowerPin;
				xc::lock_guard<power_pin> PowerPinLock;
			public: 
				//有効なGPSデータが存在するか
				bool can_read()const {
					return GPS52DUart.can_read();
				}
				//取得したGPSデータを読み出す
				gps::data read(){
					return GPS52DUart.read();
				}
			public:
				cGPS52D()
					: GPS52DUart()
					, GPS52DUartLock(GPS52DUart,true)
					, PowerPinLock(PowerPin){
					PowerPin(false);
				}
			public:
				bool lock(){
					if(GPS52DUartLock)return false;
					if(GPS52DUartLock.lock())return true;

					PowerPin(true);

					return false;
				}
				bool is_lock()const{ return GPS52DUartLock; }
				void unlock(){
					if(!GPS52DUartLock)return;
					GPS52DUartLock.unlock();
					PowerPin(false);
				}
			};
		}
	}
}
#
#endif
