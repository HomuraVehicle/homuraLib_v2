#ifndef HMR_MACHINE_MODULE_GPSPIC_INC
#define HMR_MACHINE_MODULE_GPSPIC_INC 100
#
#include<XC32/spi.hpp>
#include<XC32/open.hpp>
#include<homuraLib_v2/machine/service/delay.hpp>

/*=== module::cGPSPic ===
GPSPic�Ƃ̂��Ƃ��S������N���X

//�R�}���h
//>>0x00 : NULL
//<<0xYX : ���[�hY�A�X�e�[�^�XX
//
//>>0x10 : �d��OFF
//<<0xE0 : ACK
//
//>>0x20 : �������[�h�Ŏ擾�J�n
//<<0xE0 : ACK �X�e�[�^�XX
//
//>>0x3X : �蓮���[�h�A�X�e�[�^�XX�Ŏ擾�J�n
//<<0xE0 : ACK�@�X�e�[�^�XX
//
//>>0x40 : �f�[�^�v��
//<<0xF0 : �L���f�[�^�Ȃ�
//<<0xYX, Data[18] : ACK ���[�hY�A�X�e�[�^�XX�̃f�[�^�i�ȍ~�A18byte�f�[�^�j
			//����['1h','1m','1s','0.01s','0.0001s']	//5
			//�ܓx['1d','1m','0.01m','0.0001m']			//4
			//��k['N or S']							//1
			//�o�x['1d','1m','0.01m','0.0001m']			//4
			//����['E or W']							//1
			//���x['1000m','10m','0.1m']				//3
//
//���[�h
//	0x10:�x�~
//	0x2X:�������[�h
//	0x3X:�蓮���[�h
//�p���[
//	0xX[2::00][2::XX] : �d���I�t
//	0xX[2::01][2::XX] : Ch1�̂ݓd��ON
//	0xX[2::10][2::XX] : Ch2�̂ݓd��ON
//	0xX[2::11][2::XX] : Ch1,2�Ƃ��d��ON
//�X�e�[�^�X
//	0xX[2::XX][2::00] : Ch1���^�[�Q�b�g�A����
//	0xX[2::XX][2::01] : Ch1���^�[�Q�b�g�A�L��
//	0xX[2::XX][2::10] : Ch2���^�[�Q�b�g�A����
//	0xX[2::XX][2::11] : Ch2���^�[�Q�b�g�A�L��#define GPSMODE_is_powerGPS(Mode) (0x0C&Mode!=0)

=== sensor::gps_pic ===
v1_00/140714 hmIto
	�쐬
*/
namespace hmr {
	namespace machine {
		namespace module {
			namespace {
				namespace gpscmd {
					enum cmd {
						Null=0x00,			//Null�R�}���h
						Off=0x10,			//�p���[�I�t
						Auto=0x20,			//�I�[�g���[�h
						Manual=0x30,		//�}�j���A�����[�h
						RequestData=0x40	//
					};
				}
				namespace gpscmd_manual {
					enum cmd {
						Ch1Only=0x04,
						Ch2Only=0x0A,
						Ch1Both=0x0
					};
				}
			}
			namespace gps {
				namespace mode {
					enum value {
						Off=0,
						Manual_Ch1=1,
						Manual_Ch2=2,
						Auto=3
					};
				}
			}
			template<typename spi_register_,typename pinPower_>
			class cGPSPic {
			private:
				typedef xc32::synchronous_spi<spi_register_> syncrhronous_spi;
			private:
				syncrhronous_spi SPI;

				gps::mode::value Mode;
				pinPower_ PinPower;
				bool IsOpen;
			public:
				cGPSPic(spi_register_ SPIRegister_)
					: SPI(SPIRegister_)
					, Mode(gps::mode::Off)
					, IsOpen(false){
					PinPower.open();
				}
				~cGPSPic() {
					if(is_open())close();
					PinPower.close();
				}
			public:
				void open() {
					if(is_open())return;
					PinPower(true);
					IsOpen=true;
					Mode=gps::mode::Off;
				}
				void close() {
					if(!is_open())return;
					PinPower(false);
					IsOpen=false;
					Mode=gps::mode::Off;
				}
				bool is_open() const{
					return IsOpen;
				}
			public:
				//�f�[�^��v������
				bool request_data(hmr::gps::data& Data_) {
					if(SPI.open(true, true, 256))return true;
					xc32::scoped_open<syncrhronous_spi> ScopedOpen(SPI):

					//�f�[�^�擾
					service::delay_ms(1);
					SPI.write(gpscmd::GetData);//GPSdata�悱���R�}���h

					unsigned char Str[19];

					service::delay_ms(1);
					Str[0]=SPI.read(gpscmd::Null);

					if(Str[0]==0xEF) {
						return true;
					} else if(Str[0]==0xEE) {
						return true;
					}

					for(int i=1; i<19; ++i) {
						service::delay_ms(1);
						Str[i]=SPI.read(gpscmd::Null);
					}

					Data_.Ch=((Str[0] & 0x02)? 1 : 2);
					Data_.DeviceStatus=Str[0];
					Data_.GPSTime_100uSec  = Str[ 1]*60*60*10000 + Str[ 2]*60*10000 + Str[ 3]*10000 + Str[ 4]*100 + Str[ 5];
					Data_.Latitude_100uMin =(Str[ 6]*60*10000 + Str[ 7]*10000 + Str[ 8]*100 + Str[ 9])*(Str[10]=='N'?1:-1);
					Data_.Longitude_100uMin=(Str[11]*60*10000 + Str[12]*10000 + Str[13]*100 + Str[14])*(Str[15]=='E'?1:-1);
					Data_.Altitude_100cm   =(Str[16]*10000 + Str[17]*100 + Str[18]);

					return false;
				}
				//���[�h�ύX�����݂�
				bool request_setMode(gps::mode::value Mode_) {
					//SPI�I�[�v��
					if(SPI.open(true, true, 256))return true;

					xc32::scoped_open<syncrhronous_spi> ScopedOpen(SPI):

					service::delay_ms(1);

					switch(Mode_){
					case gps::mode::Off:
						//GPS��OFF�ɂ���
						SPI.write(gpscmd::Off);

						service::delay_ms(1);
						unsigned char tmp=SPI.read(gpscmd::Null);	//ACK or NACK����M

						service::delay_ms(1);

						if(tmp==0xE0) {
							Mode=gps::mode::Off;
						}

						return tmp!=0xE0;
					case gps::mode::Manual_Ch1:
						//�}�j���A�������GPS��ON�ɂ���
						SPI.write(gpscmd::Manual | gpscmd_manual::Ch1Only);

						service::delay_ms(1);
						unsigned char tmp=SPI.read(gpscmd::Null);	//ACK or NACK����M

						service::delay_ms(1);

						if(tmp==0xE0) {
							Mode=gps::mode::Manual_Ch1;
						}

						return tmp!=0xE0;
					case gps::mode::Manual_Ch2:
						//�}�j���A�������GPS��ON�ɂ���
						SPI.write(gpscmd::Manual | gpscmd_manual::Ch2Only);

						service::delay_ms(1);
						unsigned char tmp=SPI.read(gpscmd::Null);	//ACK or NACK����M

						service::delay_ms(1);

						if(tmp==0xE0) {
							Mode=gps::mode::Manual_Ch2;
						}

						return tmp!=0xE0;
					case gps::mode::Auto:
						//�I�[�g�����GPS��ON�ɂ���
						SPI.write(gpscmd::Auto);

						service::delay_ms(1);
						unsigned char tmp=SPI.read(gpscmd::Null);	//ACK or NACK����M

						service::delay_ms(1);

						if(tmp==0xE0) {
							Mode=gps::mode::Auto;
						}

						return tmp!=0xE0;
					default:
						return true;
					}
				}
				//GPS��Ch��ς��� ManualMode�̏ꍇ�̂ݎ��s�\
				bool request_setCh(unsigned char Ch_) {
					if(Mode!=gps::mode::Manual)return true;
					if(Ch_!=0x01 && Ch_!=0x02)return true;

					xc32::scoped_open<syncrhronous_spi> ScopedOpen(SPI):

					service::delay_ms(1);

					//�}�j���A�������GPS��ON�ɂ���
					if(Ch_==0x01)SPI.write(gpscmd::Manual | gpscmd_manual::Ch1Only);
					else if(Ch_==0x02)SPI.write(gpscmd::Manual | gpscmd_manual::Ch2Only);

					service::delay_ms(1);
					unsigned char tmp=SPI.read(gpscmd::Null);	//ACK or NACK����M

					service::delay_ms(1);

					return tmp!=0xE0;
				}
				//���݂̃��[�h���擾����
				gps::mode::value getMode()const { return Mode; }
			};
		}
	}
}
#
#endif
