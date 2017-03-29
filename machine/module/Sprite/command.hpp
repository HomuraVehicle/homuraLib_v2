#ifndef HMR_MACHINE_MODULE_SPRITE_SPRITECOMMAND_INC
#define HMR_MACHINE_MODULE_SPRITE_SPRITECOMMAND_INC 100
#
#include<homuraLib_v2/type.hpp>
namespace hmr{
	namespace machine{
		namespace module{
			namespace sprite{
				struct command_recv{
					virtual bool operator()(unsigned char c, unsigned int pos) = 0;
				};
				struct command{
					unsigned char ID;
					unsigned char SendStr[16];
					unsigned int SendSize;
					unsigned int RecvSize;
					command_recv* pRecvFunc;
					command()
						: ID(0)
						, SendSize(0)
						, RecvSize(0)
						, pRecvFunc(0){
					}
					command(unsigned char ID_,unsigned char* ptr, unsigned int SendSize_, unsigned int RecvSize_)
						: ID(ID_)
						, SendSize(SendSize_)
						, RecvSize(RecvSize_)
						, pRecvFunc(0){
						for (unsigned int Cnt = 0; Cnt < SendSize_; ++Cnt){
							SendStr[Cnt] = *ptr++;
						}
					}
					command(unsigned char ID_, unsigned char* ptr, unsigned int SendSize_, unsigned int RecvSize_, command_recv& RecvFunc_)
						: ID(ID_)
						, SendSize(SendSize_)
						, RecvSize(RecvSize_)
						, pRecvFunc(&RecvFunc_){
						for (unsigned int Cnt = 0; Cnt < SendSize_; ++Cnt){
							SendStr[Cnt] = *ptr++;
						}
					}
					command(const command& My_)
						: ID(My_.ID)
						, SendSize(My_.SendSize)
						, RecvSize(My_.SendSize)
						, pRecvFunc(My_.pRecvFunc){
						for (unsigned int Cnt = 0; Cnt < SendSize; ++Cnt){
							SendStr[Cnt] = My_.SendStr[Cnt];
						}
					}
					const command& operator=(const command& My_){
						if (this != &My_){
							ID=My_.ID;
							SendSize = My_.SendSize;
							RecvSize = My_.RecvSize;
							pRecvFunc = My_.pRecvFunc;
							for (unsigned int Cnt = 0; Cnt < SendSize; ++Cnt){
								SendStr[Cnt] = My_.SendStr[Cnt];
							}

						}
						return *this;
					}
					bool valid()const{ return ID!=0; }
				};
				namespace commands{
					namespace id{
						enum type{
							null = 0x00,
							ResetCamera = 0x01,
							TakePicture = 0x02,
							GetDataSize = 0x03,
							StopTakePicture = 0x04,
							SavePower = 0x05,
							StopSavePower = 0x06,
							GetData = 0x07,
							SetCompressRate = 0x08,
							SetImageSize = 0x09,
							SetBaudrate = 0x0A
						};
					}
					namespace imagesize{
						enum type{
							null = 0xff, size_160_120 = 0x22, size_320_240 = 0x11, size_640_480 = 0x00
						};
					}
					//リセット
					inline command resetCamera(){
						unsigned char Str[4] = { 0x56, 0x00, 0x26, 0x00 };
						return command(id::ResetCamera,Str, 4, 70);
					}
					//写真撮影
					inline command takePicture(){
						unsigned char Str[5] = { 0x56, 0x00, 0x36, 0x01, 0x00 };
						return command(id::TakePicture,Str, 5, 5);
					}
					//データサイズを得る
					struct ans_getDataSize :public command_recv{
						uint16 Size;
					public:
						bool operator()(unsigned char c, unsigned int pos){
							//DataSize_high受信
							if (pos == 7){
								Size = (static_cast<uint16>(c) << 8) & 0xFF00;
							}//DataSize_low受信
							else if (pos == 8){
								//DataSize初期化完了
								Size += static_cast<uint16>(c)& 0x00FF;
							}
							return false;
						}
					};
					//データサイズを得る
					inline command getDataSize(ans_getDataSize& Ans_getDataSize){
						unsigned char Str[5] = { 0x56, 0x00, 0x34, 0x01, 0x00 };
						return command(id::GetDataSize, Str, 5, 9, Ans_getDataSize);
					}
					//写真撮るのやめる
					inline command stop_takePicture(){
						unsigned char Str[5] = { 0x56, 0x00, 0x36, 0x01, 0x03 };
						return command(id::StopTakePicture, Str, 5, 5);
					}
					//省電力モード
					inline command savePower(){
						unsigned char Str[7] = { 0x56, 0x00, 0x3E, 0x03, 0x00, 0x01, 0x01 };
						return command(id::SavePower, Str, 7, 4);
					}
					//省電力から復帰モード
					inline command stop_savePower(){
						unsigned char Str[7] = { 0x56, 0x00, 0x3E, 0x03, 0x00, 0x01, 0x00 };
						return command(id::StopSavePower, Str, 7, 5);
					}
					//データを得るコマンドの返答用オブジェクト
					struct ans_getData:public command_recv{
					public:
						unsigned char* Itr;
						unsigned char* End;
					public:
						bool operator()(unsigned char c, unsigned int pos){
							if (pos >= 5 && Itr<End){
								*Itr = c;
								++Itr;
							}
							return false;
						}
					};
					//データを得るコマンド(初めのADDRESS(8の倍数)と、データ数指定)
					command getData(unsigned int Address_, unsigned int PackSize_, ans_getData& Ans_getData);
					//圧縮率の指定(defo 0x36)
					inline command setCompessRate(unsigned char CompressRate_){
						unsigned char Str[9] = { 0x56, 0x00, 0x31, 0x05, 0x01, 0x01, 0x12, 0x04, 0x36 };

						command Command(id::SetCompressRate, Str, 9, 6);

						//受信開始アドレス指定
						Command.SendStr[8] = CompressRate_;

						return Command;
					}
					//画像サイズの指定(定義した３通り)リセット不要
					inline command setImageSize(imagesize::type ImageSize_){
						unsigned char Str[5] = { 0x56, 0x00, 0x54, 0x01, 0xFF };

						command Command(id::SetImageSize, Str, 5, 5);

						//受信開始アドレス指定
						Command.SendStr[4] = static_cast<unsigned char>(ImageSize_);

						return Command;
					}
					//ボーレートの指定(定義した５通り)
					inline command setBaudrate(unsigned int Baudrate_){
						unsigned char Str[7] = { 0x56, 0x00, 0x24, 0x03, 0x01, 0x00, 0x00 };

						command Command(id::SetBaudrate, Str, 7, 5);

						//受信開始アドレス指定
						Command.SendStr[5] = (unsigned char)(Baudrate_ / 256);
						Command.SendStr[6] = (unsigned char)(Baudrate_ % 256);

						return Command;
					}
				}
			}
		}
	}
}
#
#endif
