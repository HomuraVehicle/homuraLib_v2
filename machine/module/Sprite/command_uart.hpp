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
				//sprite::commands�𑗎�M����uart
				//	timer�Ǘ�, led�Ǘ��͂��Ȃ��̂Œ���
				template<typename uart_register_>
				class command_uart{
				private:
					typedef command_uart<uart_register_> my_type;
				private:
					//�J�����Ƃ̒ʐM�pUart
					xc32::interrupt_uart<uart_register_> InterruptUart;
					//InterruptUart�pLock
					xc::unique_lock<xc32::interrupt_uart<uart_register_>> InterruptUartLock;
					//�R�}���h���M�󋵁@�ҋ@:0 ���M:1 ��M:2
					unsigned char Mode;
					//�R�}���h�J�E���^�[
					unsigned int Cnt;
					//���݂̒S���R�}���h
					command Cmd;
					//command�̕ԐM
					xc::promise<bool> Promise;
				private:
					struct tx_interrupt :public xc32::sfr::interrupt::function{
					private:
						my_type& Ref;
					public:
						tx_interrupt(my_type& Ref_) :Ref(Ref_){}
						virtual void operator()(void){
							//���M����
							Ref.InterruptUart.send_data(Ref.Cmd.SendStr[Ref.Cnt++]);
							//���M�I������
							if (Ref.Cnt >= Ref.Cmd.SendSize){
								//LED�_��
								//_spriteitf_led_set(1);
								//�J�E���^�[��0�ɖ߂�
								Ref.Cnt = 0;
								//�����݃��[�h����M�ɕύX
								Ref.Mode = 2;
								//���M���荞�݋֎~
								Ref.InterruptUart.send_disable();
								//��M���荞�݋���
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
							//1������M
							unsigned char c = Ref.InterruptUart.recv_data();
							++(Ref.Cnt);

							//��������M�R�}���h�擪��ǂ�ł��Ȃ���΁A�ǂݎ̂�
							if (Ref.Cnt == 1 && c != 0x76){
								//�J�E���g0�ɂ��ēǂ񂾒l�͖�������
								Ref.Cnt = 0;
								return;
							}

							//��M���ďo���֐�
							if (Ref.Cmd.pRecvFunc){
								//�I�����v�����s��ꂽ�ꍇ�ACnt�𒼂���Max�ɕύX
								if ((*Ref.Cmd.pRecvFunc)(c, Ref.Cnt - 1))Ref.Cnt = Ref.Cmd.RecvSize;
							}

							//��M�I������
							if (Ref.Cnt >= Ref.Cmd.RecvSize){
								//�����݃��[�h��ʏ�ɕύX
								Ref.Mode = 0;
								//�J�E���^�[��0�ɖ߂�
								Ref.Cnt = 0;
								//��M���荞�݋֎~
								Ref.InterruptUart.recv_disable();
								//�R�}���h�I���ʒm
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
					//command���s ���s������Atrue��Ԃ�
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
					//command���s����
					bool is_command()const{ return Mode > 0; }
					//���݂̃R�}���h�擾
					command_uart_status status()const{
						return command_uart_status(static_cast<commands::id::type>(Cmd.ID),Mode);
					}
					//command���s���L�����Z��
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
