#ifndef HMR_MACHINE_DEVICE_KK10_CPP_INC
#define HMR_MACHINE_DEVICE_KK10_CPP_INC 100
#
#include"kk10.hpp"

#ifndef HMR_TEST
//===========configuration bits（Microchip/xc32/vX.YZ/docs/config_docs に詳しいことは書いてある）================

//#pragma config FMIIEN = OFF				//Ethernet RMII/MII Enable:RMII Enabled
//#pragma config FETHIO = ON				//Ethernet I/O Pin Select:Default Ethernet I/O
#pragma config PGL1WAY = OFF			//Permission Group Lock One Way Configuration:Allow multiple reconfigurations
#pragma config PMDL1WAY = OFF			//Peripheral Module Disable Configuration:Allow multiple reconfigurations
#pragma config IOL1WAY = OFF			//Peripheral Pin Select Configuration:Allow multiple reconfigurations
//#pragma congif FUSBIDIO = OFF			//USB USBID Selection:Controlled by Port Function 

#pragma config FPLLIDIV = DIV_1			//System PLL Input Divider:1x Divider
//#pragma config FPLLRNG = RANGE_BYPASS	//System PLL Input Range:Bypass
#pragma config FPLLICLK = PLL_FRC		//System PLL Input Clock Selection:FRC is input to the System PLL
#pragma config FPLLMULT = MUL_20			//System PLL Multiplier:PLL Multiply by 1 
#pragma config FPLLODIV = DIV_2			//System PLL Output Clock Divider:2x Divider
//#pragma config UPLLFSEL = FREQ_24MHZ	//USB PLL Input Frequency Selection:USB PLL input is 24 MHz
//#pragma config UPLLEN = OFF				//USB PLL Enable:USB PLL is disabled

#pragma config FNOSC = SPLL//FRCDIV			//Oscillator Selection Bits:Fast RC Osc w/Div-by-N (FRCDIV)
//#pragma config DMTINTV = WIN_0			//DMT Count Window Interval:Window/Interval value is zero
#pragma config FSOSCEN = OFF			//Secondary Oscillator Enable:Disable SOSC
#pragma config IESO = OFF				//Internal/External Switch Over:Disabled
#pragma congig POSCMOD = OFF			//Primary Oscillator Configuation:Primary osc disabled
#pragma config OSCIOFNC = OFF			//CLKO Output Signal Active on the OSCO Pin:Disabled
//#pragma config FCKSM = CSDCMD			//Clock Switching and Monitor Selection:Clock Switch Disabled, FSCM Enabled

//#pragma config WDTPS = PS1				//Watch dog Timer Postscalor:1:1
//#pragma config WDTSPGM = STOP			//Watchdog Timer Stop During Flash Programming:WDT stops during Flash programming
//#pragma config WINDIS = NORMAL			//Watchdog Timer Window Mode:Watchdog Timer is in non-Window mode
#pragma config FWDTEN = OFF				//Watchdog Timer Enable:WDT Disabled
//#pragma config FWDTWINSZ = WINSZ_25		//Watchdog Timer Window Size:Window size is 25%
//#pragma config DMTCNT = DMT8			//Deadman Timer Count Selection:2^8 (256)
#pragma config FDMTEN = OFF				//Deadman Timer Enable:Deadman Timer is disabled

#pragma config DEBUG = OFF				//Background Debugger Enable:Debugger is disabled 
#pragma config JTAGEN = OFF				//JTAG Enable:JTAG Disabled
//#pragma config ICESEL = ICS_PGx1		//ICE/ICD Comm Channel Select:Communicate on PGEC1/PGED1 
#pragma config TRCEN = OFF				//Trace Enable:Trace features in the CPU are disabled
//#pragma config BOOTISA = MIPS32			//Boot ISA Selection:Boot code and Exception code is MIPS32 
//#pragma config FECCCON = ON				//Dynamic Flash ECC Configuration:Flash ECC is enabled (ECCCON bits are locked)
#pragma config FSLEEP = OFF				//Flash Sleep Mode:Flash is powered down when the device is in Sleep mode
//#pragma config DBGPER = ALLOW_PG2		//Debug Mode CPU Access Permission:Allow CPU access to Permission Group 2 permission regions 
//#pragma config EJTAGBEN = NORMAL		//EJTAG Boot:Normal EJTAG functionality 
#pragma config CP = OFF					//Code Protect:Protection Disabled 
#endif

//system clockを変更するには上のconfiguration bitsを変更する必要がある
const hmr::uint64 hmr::machine::device::kk10::system_clock = 80000000;

#
#endif
