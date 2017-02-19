#ifndef HMR_MACHINE_MODULE_GPSSWITCHER_INC
#define HMR_MACHINE_MODULE_GPSSWITCHER_INC
#
#include<homuraLib_v2/gps.hpp>
namespace hmr {
	namespace machine {
		namespace module {
			template<typename cGPS1_,typename cGPS2_>
			class cGPSSwitcher {
				typedef cGPS1_ cGPS1;
				typedef cGPS2_ cGPS2;
				typedef cGPSSwitcher<cGPS1, cGPS2> my_type;
			public:
				//ch1,ch2	: 何があっても固定
				//roaming	: 両方をONにし、一方が使えるようになるのを待つ。使えるchを見つけると、選択chも自動的にch1/ch2に移行する。
				enum gps_ch {gps_ch1,gps_ch2,gps_roaming};
			private:
				bool IsLock;
				gps_ch GPSCh;
				cGPS1& rGPS1;
				cGPS2& rGPS2;
			public:
				cGPSSwitcher(cGPS1& rGPS1_, cGPS2& rGPS2_)
					: IsLock(false)
					, GPSCh(gps_ch1)
					, rGPS1(rGPS1_)
					, rGPS2(rGPS2_){
					lock_apply(false);
				}
			public:
				bool lock(){
					if(is_lock())return false;
					lock_apply(true);
					return false;
				}
				void unlock(){
					if(!is_lock())return;
					lock_apply(false);
				}
				bool is_lock()const{
					return IsLock;
				}
			private:
				//ロックを2つのGPSへ反映
				//	ロックの成功の可否までは確認していない点に注意
				void lock_apply(bool OnOff_) {
					if(OnOff_) {
						if(GPSCh==gps_ch1) {
							rGPS2.unlock();
							rGPS1.lock();
						} else if(GPSCh==gps_ch2) {
							rGPS1.unlock();
							rGPS2.lock();
						} else {
							rGPS1.lock();
							rGPS2.lock();
						}
						IsLock=true;
					} else {
						rGPS1.unlock();
						rGPS2.unlock();
						IsLock=false;
					}
				}
			public:
				void setCh(gps_ch GPSCh_) {
					GPSCh=GPSCh_;
					lock_apply(IsLock);
				}
				gps_ch getCh()const { return GPSCh; }
				unsigned char getChNo()const {
					switch(GPSCh) {
					case gps_ch1:
						return 1;
					case gps_ch2:
						return 2;
					default:
						return 0;
					}
				}
				bool swapCh() {
					if(GPSCh==gps_roaming)return true;

					if(GPSCh==gps_ch1)setCh(gps_ch2);
					else setCh(gps_ch1);

					return false;
				}
				bool roamCh() {
					if(GPSCh==gps_roaming)return true;

					setCh(gps_roaming);

					return false;
				}
				bool can_read() {
					if(!is_lock())return false;

					//roamingモードなら、取得できる常態か確認し、モードを変えて答える
					if(GPSCh==gps_roaming) {
						if(rGPS1.can_read()) {
							GPSCh=gps_ch1;
							//rGPS2.lock(false);
							return true;
						} else if(rGPS2.can_read()) {
							GPSCh=gps_ch2;
							//rGPS1.lock(false);
							return true;
						} else {
							return false;
						}
					}//通常モードなら、指定されたGPSの取得可能性を返信
					else {
						switch(GPSCh) {
						case gps_ch1:
							return rGPS1.can_read();
						case gps_ch2:
							return rGPS2.can_read();
						default:
							return false;
						}
					}
				}
				gps::data read() {
					if(!can_read())return gps::data();

					switch(GPSCh) {
					case gps_ch1:
						return rGPS1.read();
					case gps_ch2:
						return rGPS2.read();
					default:
						return gps::data();
					}
				}
				gps::ch_data_pair read_withCh() {
					if(!can_read())return gps::ch_data_pair(gps_roaming,gps::data());

					switch(GPSCh) {
					case gps_ch1:
						return gps::ch_data_pair(1,rGPS1.read());
					case gps_ch2:
						return gps::ch_data_pair(2,rGPS2.read());
					default:
						return gps::ch_data_pair(0,gps::data());
					}
				}
			};
		}
	}
}
#
#endif
