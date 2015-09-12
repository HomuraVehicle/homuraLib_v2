#ifndef HMR_MACHINE_MODULE_GYROL3G4200D_INC
#define HMR_MACHINE_MODULE_GYROL3G4200D_INC 100
#
#include<hmLib/coordinates.hpp>
#include<hmLib/type.h>
#include<XCBase/function.hpp>
#include<XCBase/lock.hpp>
#include<XC32/i2c.hpp>
#include<homuraLib_v2/type.hpp>
#include<homuraLib_v2/exceptions.hpp>
#include<homuraLib_v2/machine/service/exclusive_delay.hpp>

namespace hmr {
	namespace machine {
		namespace module {
			namespace gyroL3G4200D{
				struct non_locked_exception:public hmr::exception{
				public:
					non_locked_exception() :hmr::exception(){}
					non_locked_exception(unsigned char Code_) :hmr::exception(Code_) {}
				};
				namespace sampling_rate{
					enum type{ _100Hz = 0, _200Hz = 1, _400Hz = 2, _800Hz = 3 };
					inline double sec(type Type_){
						switch (Type_){
						case _100Hz:
							return 1.0/100;
						case _200Hz:
							return 1.0/200;
						case _400Hz:
							return 1.0/400;
						default:
							return 1.0/800;
						}
					}
				}
				namespace fullscale{
					enum type{ _250dps=0,_500dps=1,_2000dps=2};
					inline double dps_per_digit( type Type_){
						switch (Type_){
						case _250dps:
							return 250. / 0x8000;
						case _500dps:
							return 500. / 0x8000;
						default:
							return 2000. / 0x8000;
						}
					}
				}
				namespace buffer_mode{
					enum type{by_pass=0,fifo=1,stream=2,stream_to_fifo=3,bypass_to_stream=4};
				}
				struct raw_data{
					typedef hmLib::coordinates3D::angle angle;
					sint16 x;
					sint16 y;
					sint16 z;
					gyroL3G4200D::fullscale::type FullScale;
					gyroL3G4200D::sampling_rate::type SamplingRate;
					angle to_angle(){
						return angle(x, y, z)*gyroL3G4200D::fullscale::dps_per_digit(FullScale)*gyroL3G4200D::sampling_rate::sec(SamplingRate)/180*hmLib::pi();
					}
				};
				struct bufstatus{
					unsigned char val;
				public:
					bool over_watermark()const{ return (val & 0x80 )!= 0; }
					bool overrun()const{ return (val & 0x40) != 0; }
					bool empty()const{ return (val & 0x20) == 0; }
					unsigned char size()const{ return val & 0x1F; }
				public:
					bufstatus() :val(0){}
					bufstatus(unsigned char val_) :val(val_){}
				};
			}

			//使用上の注意
			//	再起動後は、一度電源をオフにしてから、パワーを入れ直す必要がある
			//	500msほど、間にdelayをかませること
			template<typename i2c_register_, typename shared_i2c_identifier_>
			class cGyroL3G4200D_I2C{
				typedef cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_> my_type;
			private:
				//GyroL3G4200D固有アドレス
				static const unsigned char ModuleI2CAddress;
				//GyroL3G4200D用コマンド
				static const unsigned char i2ccmd_reg_ctrl;
				static const unsigned char i2ccmd_reg_data;
				static const unsigned char i2ccmd_reg_fifoctrl;
				static const unsigned char i2ccmd_reg_fifosrc;
				static const unsigned char i2ccmd_regmode_single;
				static const unsigned char i2ccmd_regmode_multi;
			private:
				typedef typename xc32::shared_i2c<i2c_register_, shared_i2c_identifier_>::client shared_i2c_client;
				shared_i2c_client I2C;
				xc::unique_lock<shared_i2c_client> LockI2c;

			public:
				typedef hmLib::coordinates3D::angle angle;
				typedef gyroL3G4200D::raw_data raw_data;
				typedef gyroL3G4200D::bufstatus bufstatus;
				typedef xc::function<void(angle)> observer;
				typedef xc::function<void(raw_data)> raw_observer;
			private:
				//最大再送回数
				unsigned int MaxTryNum;
				//サンプリング間隔
				gyroL3G4200D::sampling_rate::type SamplingRate;
				//データのフルスケール
				gyroL3G4200D::fullscale::type FullScale;
			public:
				cGyroL3G4200D_I2C()
					: I2C()
					, LockI2c(I2C,true)
					, MaxTryNum(5){
				}
				~cGyroL3G4200D_I2C(){ unlock(); }
			private:
				//No Copying 
				cGyroL3G4200D_I2C(const my_type& My_);
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
				void module_config(bool Power, gyroL3G4200D::sampling_rate::type SamplingRate_ = gyroL3G4200D::sampling_rate::_100Hz, gyroL3G4200D::fullscale::type FullScale_ = gyroL3G4200D::fullscale::_250dps){
					hmr_assert(is_lock(), non_locked_exception());
					//======================= Gyro 初期化 ===========================
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(ModuleI2CAddress, 0))continue;
						//CTRL_REGレジスタへの書き込み要求(複数バイト)
						if (I2C.write(i2ccmd_reg_ctrl | i2ccmd_regmode_multi))continue;
						//CTRL_REG1レジスタ NormalModeに設定する 100Hz
						if (I2C.write((static_cast<uint8>(SamplingRate_) << 6) | (Power ? 0x0F : 0x00)))continue;
						//CTRL_REG2レジスタ Default
						if (I2C.write(0x00))continue;
						//CTRL_REG3レジスタ BufOverrunで割り込みPinON
						if (I2C.write(0x02))continue;
						//CTRL_REG4レジスタ Default 250dps 8.75mdps/digit
						if (I2C.write(static_cast<uint8>(FullScale_ & 0x03) << 4))continue;
						//CTRL_REG5レジスタ FIFO enable
						if (I2C.write(0x40))continue;
						//if(I2C.write(0x00))continue;

						//I2C終了
						I2C.stop();

						SamplingRate = SamplingRate_;
						FullScale = FullScale_;

						return;
					}
				}
				void buffer_config(gyroL3G4200D::buffer_mode::type BufferMode_,unsigned char WatermarkLevel_){
					hmr_assert(is_lock(), non_locked_exception());
					//===== FIFOバッファモードを設定 =====
					for (unsigned TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(ModuleI2CAddress, 0))continue;
						//FIFO_CTRL_REGレジスタへの書き込み要求
						if (I2C.write(i2ccmd_reg_fifoctrl | i2ccmd_regmode_single))continue;
						//FIFO_CTRL_REGレジスタ StreamMode WTM=32に設定する
						if (I2C.write((static_cast<uint8>(BufferMode_)<<5)|(WatermarkLevel_&0x1F)))continue;

						//I2C終了
						I2C.stop();
						return;
					}
				}
				bufstatus buffer_status(){
					hmr_assert(is_lock(), non_locked_exception());
					//FIFOSRC取得要請
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(ModuleI2CAddress, 0))continue;
						if (I2C.write(i2ccmd_reg_fifosrc | i2ccmd_regmode_single))continue;
						if (I2C.restart(ModuleI2CAddress, 1))continue;

						bufstatus BufStatus(I2C.read(1));
						I2C.stop();
						return BufStatus;
					}
					return bufstatus();	
				}
				raw_data read_raw(){
					hmr_assert(is_lock(), non_locked_exception());
					//Data取得要請
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(ModuleI2CAddress, 0))continue;
						if (I2C.write(i2ccmd_reg_data | i2ccmd_regmode_multi))continue;
						if (I2C.restart(ModuleI2CAddress, 1))continue;

						//データ数の数だけ、読み出し処理
						raw_data Data;
						Data.FullScale = FullScale;
						Data.SamplingRate = SamplingRate;

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

						I2C.stop();

						return Data;
					}

					return raw_data();
				}
				angle read(){ return read_raw().to_angle(); }
				void read_raw(const raw_observer& RawObserver_, unsigned int DataNum_ = 1){
					hmr_assert(is_lock(), non_locked_exception());

					if(DataNum_ == 0)return;

					//Data取得要請
					for (unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if (I2C.start(ModuleI2CAddress, 0))continue;
						if (I2C.write(i2ccmd_reg_data | i2ccmd_regmode_multi))continue;
						if (I2C.restart(ModuleI2CAddress, 1))continue;

						//データ数の数だけ、読み出し処理
						raw_data Data;
						Data.FullScale = FullScale;
						Data.SamplingRate = SamplingRate;

						uint16 LowData, HighData;
						for (unsigned int DataCnt = 0; DataCnt < DataNum_; ++DataCnt){
							LowData = I2C.read(0);
							HighData = I2C.read(0);
							Data.x = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
							LowData = I2C.read(0);
							HighData = I2C.read(0);
							Data.y = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
							LowData = I2C.read(0);
							HighData = I2C.read(DataCnt + 1 == DataNum_);
							Data.z = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));

							//オブザーバーに通知
							if (RawObserver_) RawObserver_(Data);
						}
						I2C.stop();
						return;
					}
				}
				void read(const observer& Observer_, unsigned int DataNum_ = 1){
					hmr_assert(is_lock(), non_locked_exception());

					if(DataNum_ == 0)return;

					//Data取得要請
					for(unsigned int TryCnt = 0; TryCnt < MaxTryNum; ++TryCnt){
						if(I2C.start(ModuleI2CAddress, 0))continue;
						if(I2C.write(i2ccmd_reg_data | i2ccmd_regmode_multi))continue;
						if(I2C.restart(ModuleI2CAddress, 1))continue;

						//データ数の数だけ、読み出し処理
						raw_data Data;
						Data.FullScale = FullScale;
						Data.SamplingRate = SamplingRate;

						uint16 LowData, HighData;
						for(unsigned int DataCnt = 0; DataCnt < DataNum_; ++DataCnt){
							LowData = I2C.read(0);
							HighData = I2C.read(0);
							Data.x = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
							LowData = I2C.read(0);
							HighData = I2C.read(0);
							Data.y = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));
							LowData = I2C.read(0);
							HighData = I2C.read(DataCnt + 1 == DataNum_);
							Data.z = static_cast<sint16>((LowData & 0x00FF) | ((HighData << 8) & 0xFF00));

							//オブザーバーに通知
							if(Observer_) Observer_(Data.to_angle());
						}
						I2C.stop();
						return;
					}
				}
				void read_raw_all(const raw_observer& RawObserver_){
					unsigned int Size = buffer_status().size();
					read_raw(RawObserver_, Size);
				}
				void read_all(const observer& Observer_){
					unsigned int Size = buffer_status().size();
					read(Observer_, Size);
				}
			};

			//template static values
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::ModuleI2CAddress = 0x69;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_reg_ctrl = 0x20;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_reg_data = 0x28;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_reg_fifoctrl = 0x2E;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_reg_fifosrc = 0x2F;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_regmode_single = 0x00;
			template<typename i2c_register_, typename shared_i2c_identifier_>
			const unsigned char cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_>::i2ccmd_regmode_multi = 0x80;

			template<typename i2c_register_, typename shared_i2c_identifier_>
			class cGyroL3G4200D{
				typedef cGyroL3G4200D<i2c_register_, shared_i2c_identifier_> my_type;
			private:
				typedef cGyroL3G4200D_I2C<i2c_register_, shared_i2c_identifier_> my_i2c;
				typedef typename my_i2c::observer observer;
				typedef xc::lock_guard<my_i2c> lock_guard;
			private:
				my_i2c I2C;
				observer Observer;
				bool IsLock;
			public:
				cGyroL3G4200D()
					:I2C()
					,IsLock(false){
				}
			public:
				void config(const observer& Observer_){
					if (is_lock())return;

					Observer = Observer_;
				}
				bool lock(const observer& Observer_){
					config(Observer_);
					lock();
				}
				bool lock(){
					if (is_lock())return false;

					lock_guard Lock(I2C);
					if (!Lock.owns_lock())return true;

					I2C.module_config(false);

					machine::service::exclusive_delay_ms(500);

					//power ON!
					I2C.module_config(true, gyroL3G4200D::sampling_rate::_100Hz, gyroL3G4200D::fullscale::_250dps);

					//buffer 設定
					I2C.buffer_config(gyroL3G4200D::buffer_mode::stream, 31);

					IsLock = true;
				}
				bool is_lock()const{
					return IsLock;
				}
				void unlock(){
					if (!is_lock())return;

					lock_guard Lock(I2C);
					if (!Lock.owns_lock())return;

					//power OFF!
					I2C.module_config(false);

					IsLock = false;
				}
			public:
				void operator()(void){
					if (!is_lock())return;

					lock_guard Lock(I2C);
					if (!Lock.owns_lock())return;

					if(Observer){
						I2C.read_all(Observer);
					}
				}
			};
		}
	}
}
#
#endif
