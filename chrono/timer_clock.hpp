#ifndef HMR_CHRONO_TIMERINTERFACE_INC
#define HMR_CHRONO_TIMERINTERFACE_INC 200
#

/*=== homura chrono ===
�����񋟋@�\
	now�֐��Ō��ݎ�����񋟂���
	timer_register�N���X�����L����

=== chrono ===
v2_00/140712 hmIto
	cpp��
===hmrChrono===
v1_03/140226 hmIto
	���v�@�\�̒񋟃��W���[���Ƃ��āAchrono�ɖ��O�ύX
		timer����Ɛ�
		�����␳�@�\/�񋟋@�\������
		���b���s�^�X�N���\����󂯎���
		�^�C���X�^���v�Ƃ����T�O���Ȃ��Ȃ������߁A���b�Z�[�W�͔p�~
===hmrTimeStamp===
v1_02/131202 hmIto
	�^�C���X�^���v�̈Ӗ�������ύX�@�ڂ����͏�L�̎g�����̒ʂ�
v1_01/131201 hmIto
	�^�C���X�^���v����A�֐��o�^�@�\�����O
		�֐��o�^�����s��task�ɈϏ�
v1_00/131123 hmito
	�쐬
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
					//�����␳
					if(RequestCorrection) {
						//�t���O������
						RequestCorrection = false;

						//���ۂɕ␳
						UnixTime += DifSec;

						//ms�̔��C��
						//	�덷���ŏ��ɂ��邽�߂ɁA�폜���ɏ����̔����𑫂��Ă��犄���Ă���
						Timer.set_count((AddMSec * Timer.period()*2 + 1000)/2000);

						Timer.clear_count();
						Timer.clear_flag();
						return;
					}

					//�^�C�}�[�J�E���g�A�b�v
					++UnixTime;

					//�b�^�X�N���s
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
			//���݂̎������擾
			time_point now(void) const{return Interrupt.now();}
			//���݂�unix�^�C�����擾
			unix_time_point now_unix(void) const{return Interrupt.now_unix();}
		public:
			//�N���m�ɖ��b�^�X�N��o�^
			void set_sec_alarm(alarm_function& rFp_) {Interrupt.reset_func(&rFp_);}
			//�N���m�̎������C������
			void correct(sint32 DifSec_, uint16 AddMSec_) {Interrupt.correct(DifSec_, AddMSec_);}
		};
	}
}
#endif
#
#endif
