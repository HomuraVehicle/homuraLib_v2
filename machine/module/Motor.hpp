#ifndef HMR_MACHINE_MODULE_MOTOR_INC
#define HMR_MACHINE_MODULE_MOTOR_INC 100
#
namespace hmr {
	namespace machine {
		namespace module {
			template<typename opinLA,typename opinLB,typename opinRA,typename opinRB>
			class cMotor{
			private:
				opinLA PinLA;
				opinLB PinLB;
				opinRA PinRA;
				opinRB PinRB;
			private:
				bool IsBackMode;
				bool IsOpen;
			public:
				cMotor() 
					: IsBackMode(false)
					, IsOpen(false){
					PinLA.open();
					PinLB.open();
					PinRA.open();
					PinRB.open();
				}
				~cMotor() {
					PinLA.close();
					PinLB.close();
					PinRA.close();
					PinRB.close();
				}
			public:
				void open() {
					if(is_open())return;
				}
				void close() {
					if(!is_open())return;
					PinLA(false);
					PinLB(false);
					PinRA(false);
					PinRB(false);
				}
				bool is_open()const {
					return IsOpen;
				}
			public:
				void operator()(signed char LeftVal, bool IsLeftBrake,signed char RightVal, bool IsRightBrake) {
					if(!IsBackMode) {
						if(IsLeftBrake){
							PinLA(true);
							PinLB(true);
						} else if(LeftVal==100){
							PinLA(true);
							PinLB(false);
						} else if(LeftVal==-100) {
							PinLA(false);
							PinLB(true);
						} else {
							PinLA(false);
							PinLB(false);
						}
						if(IsRightBrake){
							PinRA(true);
							PinRB(true);
						}
						else if (RightVal == 100){
							PinRA(true);
							PinRB(false);
						}
						else if (RightVal == -100) {
							PinRA(false);
							PinRB(true);
						} else {
							PinRA(false);
							PinRB(false);
						}
					} else {
						if(IsLeftBrake){
							PinRA(true);
							PinRB(true);
						} else if(LeftVal==100){
							PinRA(false);
							PinRB(true);
						} else if(LeftVal==-100) {
							PinRA(true);
							PinRB(false);
						} else {
							PinRA(false);
							PinRB(false);
						}
						if(IsRightBrake){
							PinLA(true);
							PinLB(true);
						}else if (RightVal == 100){
							PinLA(false);
							PinLB(true);
						}else if (RightVal == -100) {
							PinLA(true);
							PinLB(false);
						} else {
							PinLA(false);
							PinLB(false);
						}

					}
				}
				void setBackMode(bool IsBackMode_) { IsBackMode=IsBackMode_; }
				bool isBackMode()const { return IsBackMode; }
			};
		}
	}
}
#
#endif
