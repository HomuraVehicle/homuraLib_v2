#ifndef HMR_MACHINE_MODULE_ACCELECOMPASSLSM303DLH_INC
#define HMR_MACHINE_MODULE_ACCELECOMPASSLSM303DLH_INC 100
#
#include<hmLib/coordinates.hpp>
#include<XCBase/function.hpp>
#include<XCBase/lock.hpp>
#include<XC32/i2c.hpp>
#include<homuraLib_v2/type.hpp>
#include<homuraLib_v2/exceptions.hpp>
#include<homuraLib_v2/delay.hpp>

namespace hmr{
	namespace machine{
		namespace module{
			namespace acceleLSM303DLH{
				struct non_locked_exception :public hmr::exception{
				public:
					non_locked_exception() :hmr::exception(){}
					non_locked_exception(unsigned char Code_) :hmr::exception(Code_) {}
				};
				namespace sampling_rate{
					enum type{
						_50Hz=0,
						_100Hz=1,
						_400Hz=2,
						_1000Hz=3
					};
					inline double sec(type Type){
						switch (Type){
						case _50Hz:
							return 1.0 / 50;
						case _100Hz:
							return 1.0 / 100;
						case _400Hz:
							return 1.0 / 400;
						default:
							return 1.0 / 1000;								
						}
					}

				}
				namespace fullscale{
					enum type{ _2g = 0, _4g= 1, _8g= 3 };
					inline double mps2_per_digit(type Type_){
						switch (Type_){
						case _2g:
							return 2.* 9.80665 / 0x8000;
						case _4g:
							return 4.* 9.80665 / 0x8000;
						default:
							return 8.* 9.80665 / 0x8000;
						}
					}
				}
				struct raw_data{
					typedef hmLib::coordinates3D::position position;
					sint16 x;
					sint16 y;
					sint16 z;
					fullscale::type FullScale;
					position to_vector()const{
						return position(x, y, z)*fullscale::mps2_per_digit(FullScale);
					}
				};
				struct status{
					unsigned char val;
					status() :val(){}
					status(unsigned char val_) :val(val_){}
					bool overrun()const{ return (val & 0x80 ) != 0x00; }
					bool empty()const{ return (val & 0x08 )== 0x00; }
				};
			}
			namespace compassLSM303DLH{
				typedef acceleLSM303DLH::non_locked_exception non_locked_exception;
				namespace sampling_rate{
					enum type{
						_0_75Hz = 0,
						_1_5Hz = 1,
						_3Hz = 2,
						_7_5Hz = 3,
						_15Hz = 4,
						_30Hz = 5,
						_75Hz=6
					};
					inline double sec(type Type){
						switch (Type){
						case _0_75Hz:
							return 1.0 / 0.75;
						case _1_5Hz:
							return 1.0 / 1.5;
						case _3Hz:
							return 1.0 / 3;
						case _7_5Hz:
							return 1.0 / 7.5;
						case _15Hz:
							return 1.0 / 15;
						case _30Hz:
							return 1.0 / 30;
						default:
							return 1.0 / 75;
						}
					}
				}
				namespace fullscale{
					enum type{
						_1_3Gauss = 1,
						_1_9Gauss = 2,
						_2_5Gauss = 3,
						_4_0Gauss = 4,
						_4_7Gauss = 5,
						_5_6Gauss = 6,
						_8_1Gauss = 7
					};
					inline double gauss_per_digit(type Type_){
						switch (Type_){
						case _1_3Gauss:
							return 1.3 / 2048;
						case _1_9Gauss:
							return 1.9 / 2048;
						case _2_5Gauss:
							return 2.5 / 2048;
						case _4_0Gauss:
							return 4.0 / 2048;
						case _4_7Gauss:
							return 4.7 / 2048;
						case _5_6Gauss:
							return 5.6 / 2048;
						default:
							return 8.1 / 2048;
						}
					}
				}
				struct raw_data{
					typedef hmLib::coordinates3D::position position;
					sint16 x;
					sint16 y;
					sint16 z;
					fullscale::type FullScale;
					position to_vector()const{
						return position(x, y, z)*fullscale::gauss_per_digit(FullScale);
					}
				};
				struct status{
					unsigned char val;
					status() :val(){}
					status(unsigned char val_) :val(val_){}
					bool empty()const{ return (val & 0x01 )== 0x00; }
				};
			}
			template<typename i2c_register_,typename shared_i2c_identifier>
			class cAcceleCompassLSM303DLH_I2C{
				typedef cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier> my_type;
			private:
				//LSM303DLH固有アドレス
				static const unsigned char AcceleI2CAddress;
				static const unsigned char CompassI2CAddress;
				//GyroL3G4200D用コマンド
				static const unsigned char i2ccmd_acl_reg_ctrl;
				static const unsigned char i2ccmd_acl_reg_data;
				static const unsigned char i2ccmd_acl_reg_status;
				static const unsigned char i2ccmd_cmp_reg_ctrl;
				static const unsigned char i2ccmd_cmp_reg_data;
				static const unsigned char i2ccmd_cmp_reg_status;
				static const unsigned char i2ccmd_regmode_single;
				static const unsigned char i2ccmd_regmode_multi;
			private:
				typedef typename xc32::shared_i2c<i2c_register_, shared_i2c_identifier>::client shared_i2c_client;
				shared_i2c_client I2C;
				xc::unique_lock<shared_i2c_client> LockI2c;
			private:
				//最大再送回数
				unsigned int MaxTryNum;

				//共通データ型
				typedef hmLib::coordinates3D::position position;

				//Accele関係
				typedef acceleLSM303DLH::status acl_status;
				typedef acceleLSM303DLH::raw_data acl_raw_data;
				//データのフルスケール
				acceleLSM303DLH::fullscale::type AcceleFullScale;

				//Compass関係
				typedef compassLSM303DLH::status cmp_status;
				typedef compassLSM303DLH::raw_data cmp_raw_data;
				//データのフルスケール
				compassLSM303DLH::fullscale::type CompassFullScale;
			public:
				cAcceleCompassLSM303DLH_I2C()
					: I2C()
					, LockI2c(I2C, true)
					, MaxTryNum(5){
					//I2C.config(xc32::i2c::clockmode::type::_1MHz, 0);
				}
				~cAcceleCompassLSM303DLH_I2C(){ unlock(); }
			private:
				//No Copying 
				cAcceleCompassLSM303DLH_I2C(const my_type& My_);
				const my_type& operator=(const my_type& My_);
			public:
				void config(unsigned int MaxTryNum_){ MaxTryNum = MaxTryNum_; }
				bool lock(unsigned int MaxTryNum_){
					config(MaxTryNum_);
					return lock();
				}
				bool lock(){
					if (is_lock())return false;
					return LockI2c.lock();
				}
				bool is_lock()const{
					return LockI2c;
				}
				void unlock(){
					if (!is_lock())return;
					LockI2c.unlock();
				}
			public:
				void accele_config(bool Power_, acceleLSM303DLH::sampling_rate::type SamplingRate_,acceleLSM303DLH::fullscale::type FullScale_){
					hmr_assert(is_lock(), acceleLSM303DLH::non_locked_exception());
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(AcceleI2CAddress, 0))continue;
						//CTRL_REGレジスタへの書き込み要求(複数バイト)
						if (I2C.write(i2ccmd_acl_reg_ctrl | i2ccmd_regmode_multi))continue;
						//CTRL_REG1の初期化 Normal 10Hz
						if (I2C.write((Power_ ? 0x27 : 0x00) | (static_cast<uint8>(SamplingRate_ << 3) & 0x18)))continue;
						//CTRL_REG2の初期化 default
						if (I2C.write(0x00))continue;
						//CTRL_REG3の初期化 default
						if (I2C.write(0x00))continue;
						//CTRL_REG4の初期化 default
						if (I2C.write(0x30&static_cast<uint8>(FullScale_<<4)))continue;
						//CTRL_REG5の初期化 default
						if (I2C.write(0x00))continue;
						//CTRL_REG6の初期化 default
						if (I2C.write(0x00))continue;

						AcceleFullScale = FullScale_;

						//i2c1終了
						I2C.stop();
						return;
					}
				}
				acl_status accele_status(){
					hmr_assert(is_lock(), acceleLSM303DLH::non_locked_exception());

					//FIFOSRC取得要請
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(AcceleI2CAddress, 0))continue;
						if (I2C.write(i2ccmd_acl_reg_status | i2ccmd_regmode_single))continue;
						if (I2C.restart(AcceleI2CAddress, 1))continue;

						acl_status Status(I2C.read(1));
						I2C.stop();
						return Status;
					}
					return acl_status();
				}
				acl_raw_data accele_read_raw(){
					hmr_assert(is_lock(), acceleLSM303DLH::non_locked_exception());

					//======== ACC
					for(unsigned int TryCnt = 0; TryCnt <MaxTryNum; ++TryCnt){
						if(I2C.start(AcceleI2CAddress, 0))continue;
						//x軸加速度LowByte要求
						if(I2C.write(i2ccmd_acl_reg_data | i2ccmd_regmode_multi))continue;
						//読み取り用に再起動
						if(I2C.restart(AcceleI2CAddress, 1))continue;

						acl_raw_data Data;

						uint8 LowData = I2C.read(0);
						uint8 HighData = I2C.read(0);
						Data.x = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
						LowData = I2C.read(0);
						HighData = I2C.read(0);
						Data.y = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
						LowData = I2C.read(0);
						HighData = I2C.read(1);
						Data.z = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));

						Data.FullScale = AcceleFullScale;

						I2C.stop();

						return Data;
					}

					return acl_raw_data();
				}
				position accele_read(){ return accele_read_raw().to_vector(); }
				void compass_config(compassLSM303DLH::sampling_rate::type SamplinRate_, compassLSM303DLH::fullscale::type FullScale_){
					hmr_assert(is_lock(), compassLSM303DLH::non_locked_exception());

					//Magne#1
					for (unsigned int TryCnt = 0; TryCnt <MaxTryNum; ++TryCnt){
						if (I2C.start(CompassI2CAddress, 0))continue;
						//CTRL_REGレジスタへの書き込み要求(複数バイト)
						if (I2C.write(i2ccmd_cmp_reg_ctrl | i2ccmd_regmode_multi))continue;
						//CRA_REG_Mを初期化
						if (I2C.write(0x1C & static_cast<uint8>(SamplinRate_ << 2)))continue;
						//CRB_REG_Mを初期化　1.3 Gauss
						if (I2C.write(static_cast<uint8>(FullScale_ << 5)&0xE0))continue;
						//MR_REG_Mを初期化　continuous
						if (I2C.write(0x00))continue;

						//i2c1終了
						I2C.stop();
						return;
					}
				}
				cmp_status compass_status(){
					hmr_assert(is_lock(), compassLSM303DLH::non_locked_exception());

					//FIFOSRC取得要請
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(CompassI2CAddress, 0))continue;
						if (I2C.write(i2ccmd_cmp_reg_status | i2ccmd_regmode_single))continue;
						if (I2C.restart(CompassI2CAddress, 1))continue;

						cmp_status Status(I2C.read(1));
						I2C.stop();
						return Status;
					}
					return cmp_status();

				}
				cmp_raw_data compass_read_raw(){
					hmr_assert(is_lock(), compassLSM303DLH::non_locked_exception());

					//======== ACC
					for (unsigned int TryCnt = 0; TryCnt <MaxTryNum; ++TryCnt){
						if (I2C.start(CompassI2CAddress, 0))continue;
						//x軸加速度LowByte要求
						if (I2C.write(i2ccmd_cmp_reg_data | i2ccmd_regmode_multi))continue;
						//読み取り用に再起動
						if (I2C.restart(CompassI2CAddress, 1))continue;

						cmp_raw_data Data;
						uint16 LowData, HighData;

						LowData = I2C.read(0);
						HighData = I2C.read(0);
						Data.x = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
						LowData = I2C.read(0);
						HighData = I2C.read(0);
						Data.y = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
						LowData = I2C.read(0);
						HighData = I2C.read(1);
						Data.z = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));

						Data.FullScale = CompassFullScale;

						I2C.stop();

						return Data;
					}

					return cmp_raw_data();
				}
				position compass_read(){ return compass_read_raw().to_vector(); }
			};
			//LSM303DLH固有アドレス
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::AcceleI2CAddress = 0x19;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::CompassI2CAddress = 0x1E;
			//LSM303DLH用コマンド
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_acl_reg_ctrl = 0x20;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_acl_reg_data = 0x28;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_acl_reg_status = 0x27;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_cmp_reg_ctrl = 0x00;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_cmp_reg_data = 0x03;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_cmp_reg_status = 0x09;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_regmode_single = 0x00;
			template<typename i2c_register_, typename shared_i2c_identifier>
			const unsigned char cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier>::i2ccmd_regmode_multi = 0x80;

			template<typename i2c_register_, typename shared_i2c_identifier>
			class cAcceleCompassLSM303DLH{
				typedef cAcceleCompassLSM303DLH<i2c_register_, shared_i2c_identifier> my_type;
			private:
				typedef cAcceleCompassLSM303DLH_I2C<i2c_register_, shared_i2c_identifier> my_i2c;
				typedef hmLib::coordinates3D::position position;
				typedef xc::function<void(position)> observer;
				typedef xc::lock_guard<my_i2c> lock_guard;
			private:
				my_i2c I2C;
				observer AcceleObserver;
				observer CompassObserver;
				hmr::delay_interface* pDelay;
				bool IsLock;
			public:
				cAcceleCompassLSM303DLH()
					: I2C()
					, IsLock(false)
					, pDelay(0){
				}
			public:
				void config(const observer& AcceleObserver_, const observer& CompassObserver_, hmr::delay_interface& Delay_){
					if (is_lock())return;
					AcceleObserver = AcceleObserver_;
					CompassObserver = CompassObserver_;
					pDelay = &Delay_;
				}
				bool lock(const observer& AcceleObserver_, const observer& CompassObserver_, hmr::delay_interface& Delay_){
					config(AcceleObserver_, CompassObserver_, Delay_);
					return lock();
				}
				bool lock(){
					if (is_lock())return false;

					lock_guard Lock(I2C);
					if (!Lock.owns_lock())return true;

					//一旦パワーオフ
					I2C.accele_config(false, acceleLSM303DLH::sampling_rate::_100Hz, acceleLSM303DLH::fullscale::_2g);

					pDelay->exclusive_delay_ms(500);

					//power ON!
					I2C.accele_config(true, acceleLSM303DLH::sampling_rate::_100Hz, acceleLSM303DLH::fullscale::_2g);
					I2C.compass_config(compassLSM303DLH::sampling_rate::_0_75Hz, compassLSM303DLH::fullscale::_1_3Gauss);

					IsLock = true;

					return false;
				}
				bool is_lock()const{
					return IsLock;
				}
				void unlock(){
					if (!is_lock())return;

					lock_guard Lock(I2C);
					if (!Lock.owns_lock())return;

					//power OFF!
					I2C.accele_config(false,acceleLSM303DLH::sampling_rate::_100Hz, acceleLSM303DLH::fullscale::_2g);

					IsLock = false;
				}
			public:
				void operator()(void){
					if (!is_lock())return;

					lock_guard Lock(I2C);

					if (!Lock.owns_lock())return;

					if(AcceleObserver && !I2C.accele_status().empty()){
						AcceleObserver(I2C.accele_read());
					}

					if(CompassObserver && !I2C.compass_status().empty()){
						CompassObserver(I2C.compass_read());
					}
				}
			};

		}
	}
}
#
#endif
