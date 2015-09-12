#ifndef HMR_MACHINE_DEVICE_KK10_INC
#define HMR_MACHINE_DEVICE_KK10_INC 100
#
#include<device_include.h>
#include<XC32/sfr/port.hpp>
#include<XC32/pin.hpp>
//#include<XC32/analog_pin.hpp>
//#include<XC32/adc.hpp>
#include<XC32/clock.hpp>
#include<XC32/device.hpp>
#include<homuraLib_v2/type.hpp>
namespace hmr {
	namespace machine {
		namespace device {
			class kk10 {
				xc32::device Device;
			//===========================�f�o�C�X=========================
//			#define _device_SYSTEM_CLOCK 73700000
				static const uint64 system_clock;
			public:
				kk10(xc32::clock_div::type PeripheralBathClockDiv_=xc32::clock_div::div8):Device(){
					//�f�o�C�X���b�N
					Device.lock(system_clock,PeripheralBathClockDiv_);
					
					//AD�ϊ��ݒ�
					//ADC�̃L�����u���[�V�����p�̃r�b�g�CPIC32MZ�̃f�[�^�V�[�g��ADC�̏͂ɏ����Ă�����
/*					AD1CAL1 = 0xF8894530;
					AD1CAL2 = 0x01E4AF69;
					AD1CAL3 = 0x0FBBBBB8;
					AD1CAL4 = 0x000004AC;
					AD1CAL5 = 0x02000002;
*//*
AD1CAL1 = 0xB3341210;
AD1CAL2 = 0x01FFA769;
AD1CAL3 = 0x0BBBBBB8;
AD1CAL4 = 0x000004AC;
AD1CAL5 = 0x02028002;

    AD1CAL1 = DEVADC1;                  // Writing Calibration-Data to ADC-Registers
    AD1CAL2 = DEVADC2;
    AD1CAL3 = DEVADC3;
    AD1CAL4 = DEVADC4;
    AD1CAL5 = DEVADC5;
*/					//=== ���́AANSEL bits �ɂ��ẮAanalog_pin��lock���ɐݒ肳���̂ŁA�{����zero fill��OK ===
					//AD�ݒ�
					ANSELA = 0x0200;			//0,1,5,9,10���ݒ�\,9:AD
					__asm("nop");
					ANSELB = 0x8B0B;			//0�`15�܂Őݒ�\,0,1,3,8,9,11,15:AD
					__asm("nop");
					ANSELC = 0x0002;			//1�`4�܂Őݒ�\,1:AD
					__asm("nop");
					ANSELD = 0x0000;			//14,15���ݒ�\
					__asm("nop");
					ANSELE = 0x0050;			//4�`9�܂Őݒ�\,4,6:AD
					__asm("nop");
					ANSELF = 0x0000;			//12,13���ݒ�\
					__asm("nop");
					ANSELG = 0x0000;			//6�`9,15���ݒ�\
					__asm("nop");

					//ReMapable pin�̐ݒ�
					xc32::oscillator::lock_guard Lock(xc32::oscillator::Mutex);
					CFGCONbits.IOLOCK = 0;
					//=============UART1(RF#0)�s���ݒ�===============
					__asm("nop");
					RPC13R = 1;		//U1TX��C13pin�ɐݒ�
					__asm("nop");
					U1RXR = 7;		//U1RX��C14pin�ɐݒ�
					__asm("nop");
					U1CTSR = 0;		//U1CTS��D9pin�ɐݒ�
					__asm("nop");
					RPD0R = 1;		//U1RTS��D0pin�ɐݒ�
					__asm("nop");
					//=============UART3(RF#1)�s���ݒ�===============
					RPD2R = 1;		//U3TX��D2pin�ɐݒ�
					__asm("nop");
					U3RXR = 0;		//U3RX��D3pin�ɐݒ�
					__asm("nop");
					RPD4R = 1;		//U3RTS��D4pin�ɐݒ�
					__asm("nop");
					U3CTSR = 6;		//U3CTS��D5pin�ɐݒ�
					__asm("nop");
					//=============UART4(GPS#0)�s���ݒ�===============
					U4RXR = 12;		//U4RX��C2pin�ɐݒ�
					__asm("nop");
					RPC3R = 2;		//U4TX��C3pin�ɐݒ�
					__asm("nop");
					//=============UART2(GPS#1)�s���ݒ�===============
					RPE8R = 2;		//U2TX��E8pin�ɐݒ�
					__asm("nop");
					U2RXR = 13;		//U2RX��E9pin�ɐݒ�
					__asm("nop");
					//=============UART5(Camera#0)�s���ݒ�===============
					RPD15R = 3;		//U5TX��D15pin�ɐݒ�
					__asm("nop");
					U5RXR = 11;		//U5RX��D14pin�ɐݒ�
					__asm("nop");
					//=============UART6(Camera#1)�s���ݒ�===============
					RPF3R = 4;		//U6TX��F3pin�ɐݒ�
					__asm("nop");
					U6RXR = 11;		//U6RX��F2pin�ɐݒ�
					__asm("nop");
					//============SPI2�s���ݒ�=============
					RPG8R = 6;		//SDO2��G8pin�ɐݒ�
					__asm("nop");
					SDI2R = 1;		//SDI2��G7pin�ɐݒ�
					__asm("nop");
					CFGCONbits.IOLOCK = 1;

				}
			public://PIN
				//=========================== �f�o�C�X�J�[�l�� =========================
				typedef xc32::output_pin<xc32::sfr::portE::pin3> opinDevicePower;

				//============================5V DC/DC=============
				typedef xc32::output_pin<xc32::sfr::portC::pin4> opin5VDCDC;

				//=================================LED===============
				typedef xc32::output_pin<xc32::sfr::portC::pin12,true> opinRedLED;
				typedef xc32::output_pin<xc32::sfr::portC::pin15,true> opinYellowLED;

				//============================DIP SWITCH=============
				typedef xc32::input_pin<xc32::sfr::portE::pin0,true> ipinDip1;
				typedef xc32::input_pin<xc32::sfr::portA::pin7,true> ipinDip2;
				typedef xc32::input_pin<xc32::sfr::portA::pin6,true> ipinDip3;
				typedef xc32::input_pin<xc32::sfr::portG::pin0,true> ipinDip4;

				//===============================ADC=================
				/*
				typedef xc32::analog_pin<xc32::sfr::portB::pin3> apinADC0;		// 1/2 attenated for CO2 sensor
				typedef apinADC0 apinCO2;		//ADC#0
				typedef xc32::analog_pin<xc32::sfr::portB::pin1> apinADC1;
				typedef apinADC1 apinThermo;	//ADC#1
				typedef xc32::analog_pin<xc32::sfr::portA::pin9> apinADC2;
				typedef xc32::analog_pin<xc32::sfr::portB::pin8> apinADC3;
				typedef xc32::analog_pin<xc32::sfr::portB::pin9> apinADC4;		//attenated signal
				typedef xc32::analog_pin<xc32::sfr::portE::pin6> apinRFBattery;
				typedef xc32::analog_pin<xc32::sfr::portC::pin1> apinMainBoardBattery;
				typedef xc32::analog_pin<xc32::sfr::portE::pin4> apinMotorBattery;
				*/
				typedef xc32::sfr::portB::pin3 apinADC0;
				typedef apinADC0 apinCO2;
				typedef xc32::sfr::portB::pin1 apinADC1;
				typedef apinADC1 apinThermo;
				typedef xc32::sfr::portA::pin9 apinADC2;
				typedef xc32::sfr::portB::pin8 apinADC3;
				typedef xc32::sfr::portB::pin9 apinADC4;

				typedef xc32::sfr::portE::pin6 apinRightMotorBattery;
				typedef xc32::sfr::portC::pin1 apinMainBoardBattery;
				typedef xc32::sfr::portE::pin4 apinLeftMotorBattery;


				typedef xc32::output_pin<xc32::sfr::portB::pin4> opinADC0Power;
				typedef xc32::output_pin<xc32::sfr::portB::pin2> opinADC1to4Power;
				
				//==============================CAMERA======================
				/*�J������4��Ȃ���D2�r�b�g�ŃJ������I��
				S1 S0 CAMERA ch
				0	0	#0
				0	1	#1
				1	0	#2	�R�l�N�^�܂��Ȃ�(2013/12/15)
				1	1	#3	�R�l�N�^�܂��Ȃ�(2013/12/15)
				�d��OFF�ɂ���ɂ�S0,S1��input�ݒ�ɂ��邩�Ȃ��ł��Ȃ�ch�ɂ���(���Ƃ��΁CS1=S0=1)
				*/
				//#define _device_PIN_CAM_PW			PORTAbits.RA1		//PowerSwitch for Camera	OUT
				//#define _device_PIN_CAM_PW_set(val)			_device_PIN_CAM_PW=itob(val!=0);__asm__("nop")
//				typedef xc32::output_pin<xc32::sfr::portF::pin1,true> opinSelect0;
//				typedef xc32::output_pin<xc32::sfr::portF::pin0,true> opinSelect1;
				typedef xc32::output_pin<xc32::sfr::portF::pin12> opinCamera0Power;
				typedef xc32::input_pin<xc32::sfr::portD::pin14> ipinCamera0_RX;
				typedef xc32::output_pin<xc32::sfr::portD::pin15> opinCamera0_TX;
				typedef xc32::output_pin<xc32::sfr::portF::pin13> opinCamera1Power;
				typedef xc32::input_pin<xc32::sfr::portF::pin2> ipinCamera1_RX;
				typedef xc32::output_pin<xc32::sfr::portF::pin3> opinCamera1_TX;

				//===============================Inertial==========================
				typedef xc32::output_pin<xc32::sfr::portF::pin4> opinInertialI2C_SDA;
				typedef xc32::output_pin<xc32::sfr::portF::pin5> opinInertialI2C_SCL;
				typedef xc32::output_pin<xc32::sfr::portD::pin11> opinInertialPower;

				typedef xc32::input_pin<xc32::sfr::portD::pin1> ipinGyroDataReady;

				//===============================CO2========================
				typedef xc32::output_pin<xc32::sfr::portB::pin14> opinCO2PumpsPw;
				typedef xc32::output_pin<xc32::sfr::portB::pin14> opinCO2SensorPw;

				//===============================microphone========================
				typedef xc32::output_pin<xc32::sfr::portF::pin8> opinMicrophonePw;

				//===============================External SPI Device=============================
				typedef xc32::output_pin<xc32::sfr::portB::pin10> opinExtSPIDevicePower;
				typedef xc32::output_pin<xc32::sfr::portB::pin12> opinExtSPIDevice_SPISelect;

				//============================24bit ADC module===========
				typedef xc32::output_pin<xc32::sfr::portA::pin1> opin24bitADC_Pw;
				typedef xc32::output_pin<xc32::sfr::portB::pin13> opin24bitADC_SPISelect;

				//===============================I2C=========================
				typedef xc32::output_pin<xc32::sfr::portA::pin4> opinExternalI2C_Pw;

				typedef xc32::output_pin<xc32::sfr::portA::pin2> opinExtI2C_SCL;
				typedef xc32::output_pin<xc32::sfr::portA::pin3> opinExtI2C_SDA;

				//===============================RF modeule===================
				typedef xc32::output_pin<xc32::sfr::portG::pin9> opinRF0Power;		//PowerSwitch for RF module
				typedef xc32::input_pin<xc32::sfr::portD::pin9> ipinRF0_CTS;
				typedef xc32::output_pin<xc32::sfr::portD::pin0> opinRF0_RTS;
				typedef xc32::output_pin<xc32::sfr::portC::pin13> opinRF0_TX;
				typedef xc32::input_pin<xc32::sfr::portC::pin14> ipinRF0_RX;

//				typedef xc32::output_pin<xc32::sfr::portG::pin12> opinRF1Power;
				typedef xc32::output_pin<xc32::sfr::portE::pin2> opinRF1Power;
				typedef xc32::input_pin<xc32::sfr::portD::pin5> ipinRF1_CTS;
				typedef xc32::output_pin<xc32::sfr::portD::pin4> opinRF1_RTS;
				typedef xc32::output_pin<xc32::sfr::portD::pin2> opinRF1_TX;
				typedef xc32::input_pin<xc32::sfr::portD::pin3> ipinRF1_RX;

				//==============================Servo,Motor=========================
				//���[�^�p�̃��x���ϊ���H�@Enable�M����p�~�D���̑��C�d�����̂��̂�ON/OFF���s���D
				//#define _device_PIN_SERVO_PW		PORTFbits.RF8		//PowerSwitch for Servo
				//#define _device_PIN_SERVO_PW_set(val) _device_PIN_SERVO_PW=itob(val!=0);__asm__("nop")
				typedef xc32::output_pin<xc32::sfr::portA::pin5> opinMotorLA;
				typedef xc32::output_pin<xc32::sfr::portG::pin15> opinMotorLB;
				//typedef xc32::output_pin<xc32::sfr::portC::pin3> opinMotorLPower;
				typedef xc32::output_pin<xc32::sfr::portF::pin1> opinMotorRA;
				typedef xc32::output_pin<xc32::sfr::portF::pin0> opinMotorRB;
				//typedef xc32::output_pin<xc32::sfr::portC::pin15> opinMotorRPower;
				typedef xc32::output_pin<xc32::sfr::portE::pin5> opinMotorPower;

				//============================GPS========================
				typedef xc32::output_pin<xc32::sfr::portA::pin0> opinGPS0Power;
				typedef xc32::input_pin<xc32::sfr::portC::pin2> ipinGPS0_RX;
				typedef xc32::output_pin<xc32::sfr::portC::pin3> opinGPS0_TX;
				typedef xc32::output_pin<xc32::sfr::portB::pin5> opinGPS1Power;
				typedef xc32::input_pin<xc32::sfr::portE::pin9> ipinGPS1_RX;
				typedef xc32::output_pin<xc32::sfr::portE::pin8> opinGPS1_TX;

				//===========================�w�b�h���C�g=======================
				typedef xc32::output_pin<xc32::sfr::portG::pin13> opinHeadLightPower;

				//===========================SPI============================
				typedef xc32::output_pin<xc32::sfr::portG::pin6> opinSPI2_SCK;
				typedef xc32::input_pin<xc32::sfr::portG::pin7> ipinSPI2_SDI;
				typedef xc32::output_pin<xc32::sfr::portG::pin8> opinSPI2_SDO;

				//==============================SD�J�[�h================================
				typedef xc32::output_pin<xc32::sfr::portG::pin12> opinSDPower;
				typedef xc32::output_pin<xc32::sfr::portE::pin7> opinSD_SPISelect;

				//==============================USB================================]
				typedef xc32::output_pin<xc32::sfr::portG::pin1> opinUSBPower;
			};
		}
	}
}
#
#endif
