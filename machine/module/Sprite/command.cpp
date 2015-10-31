#ifndef HMR_MACHINE_MODULE_SPRITE_SPRITECOMMAND_CPP_INC
#define HMR_MACHINE_MODULE_SPRITE_SPRITECOMMAND_CPP_INC 100
#
#include<homuraLib_v2/type.hpp>
#include"command.hpp"
namespace hmr{
	namespace machine{
		namespace module{
			namespace sprite{
				namespace commands{
					//データを得るコマンド(初めのADDRESS(8の倍数)と、データ数指定)
					command getData(unsigned int Address_, unsigned int PackSize_, ans_getData& Ans_getData){
						static unsigned char Str[16] = { 0x56, 0x00, 0x32, 0x0C, 0x00, 0x0A, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x0B };

						//PackSizeは８の倍数なので、8の倍数にする！！　超えた分は読み捨てる方針で
						if (PackSize_ % 8 != 0){
							PackSize_ = PackSize_ + 8 - PackSize_ % 8;
						}

						command Command(id::GetData, Str, 16, 10 + PackSize_, Ans_getData);

						//受信開始アドレス指定
						Command.SendStr[8] = (unsigned char)(Address_ / 256);
						Command.SendStr[9] = (unsigned char)(Address_ % 256);

						//受信パケットのサイズ指定
						Command.SendStr[12] = (unsigned char)(PackSize_ / 256);
						Command.SendStr[13] = (unsigned char)(PackSize_ % 256);

						return Command;
					}
				}
			}
		}
	}
}
#
#endif
