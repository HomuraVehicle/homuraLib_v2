#ifndef HMR_CODE_VIC1_CPP_INC
#define HMR_CODE_VIC1_CPP_INC 201
#
/*
=== code::vic1 ===
v2_01/140622 hmIto
	open/close関数で値を返さないパスがあった問題を修正
*/
#ifndef HMR_CRC8_INC
#	include<homuraLib_v2/crc8.h>
#endif
#ifndef HMR_CODE_CLIENT_INC
#	include"client.hpp"
#endif
#ifndef HMR_CODE_VIC1_INC
#	include"vic1.hpp"
#endif

namespace hmr {
	namespace code {
		//========== vic1::buf ===========
		//バッファ初期化
		vic1::buffer::buffer(vic1* VIC1_, client* pClient_, unsigned char* BufBegin, unsigned char* BufEnd)
			: VIC1(VIC1_)
			,pClient(pClient_){
			Begin=BufBegin;
			End=BufEnd;
			VComItr=BufBegin;
			GateItr=BufBegin;
		}
		//バッファ初期化
		vic1::buffer::buffer(vic1* VIC1_, unsigned char* BufBegin, unsigned char* BufEnd)
			: VIC1(VIC1_)
			, pClient(0) {
			Begin=BufBegin;
			End=BufEnd;
			VComItr=BufBegin;
			GateItr=BufBegin;
		}
		//バッファ終端化
		vic1::buffer::~buffer() {
			Begin=0;
			End=0;
			VComItr=0;
			GateItr=0;
			//データ受信をなかったことにする
			pClient->flush();
			//データ送信をなかったことにする
			pClient->cancel_get();
			pClient=0;
		}
		//補充できる限り補充
		void vic1::buffer::load() {
			//受信可能である限り実行
			while(pClient->can_getc()) {
				//実際に受信
				*((VComItr)++) = pClient->getc();

				//バッファがいっぱいになった場合、あるいはvcomからの送信が途切れた場合
				if(VComItr+1 == End || pClient->flowing()==0) {
					//crc8を計算して、末尾に書き込む
					*(VComItr) = crc8_puts(0x00, Begin, VComItr - Begin);
					++(VComItr);
					break;
				}
			}
		}
		bool vic1::buffer::open(client* pClient_) {
			pClient=pClient_;
			return false;
		}
		bool vic1::buffer::close() {
			pClient=0;
			return false;
		}
		//すでに利用可能になっているか
		bool vic1::buffer::is_open() {return pClient!=0;}
		//バッファへ書き込んだデータを処理
		void vic1::buffer::flush() {
			unsigned char CRC8;

			//すでに失敗していれば無視
			if(VIC1->Fail==1)return;

			//そもそも受信していなければ無視
			if(GateItr==Begin) {
				//失敗したことを通知
				VIC1->Fail=1;
				//Iteratorを初期位置へ
				VComItr=Begin;
				GateItr=Begin;

				return;
			}

			//1byte戻す
			--(GateItr);

			//GateItrまでのチェックサムを演算
			CRC8=crc8_puts(0x00, Begin, GateItr - Begin);

			//チェックサムに失敗している場合は破棄
			if(*(GateItr)!=CRC8) {
				//失敗したことを通知
				VIC1->Fail=1;
				//Iteratorを初期位置へ
				VComItr=Begin;
				GateItr=Begin;

				return;
			}

			//データセーブ
			//VComに順次投げ入れる
			VComItr = Begin;
			while(VComItr!= GateItr) {
				if(pClient->can_putc()==0) {
					//エラー発報
					VIC1->Err=error_SAVE_REJECTED;
					break;
				}
				pClient->putc(*(VComItr++));
			}

			//Iteratorを初期位置へ
			VComItr=Begin;
			GateItr=Begin;
		}
		//バッファに書き込み可能か
		bool vic1::buffer::can_putc() {
			//バッファに空きがある場合
			//	送信と違ってflushするまでどれがCheckSumかわからないので、
			//	1byteあけておく必要がない点に注意
			return GateItr < End;
		}
		//バッファへ書き込み
		void vic1::buffer::putc(unsigned char c) {
			//書き込み可能でない場合、エラー発報
			if(!can_putc()) {
				//エラー発報
				VIC1->Err=error_OVERFLOW;
				//失敗したことを通知
				VIC1->Fail=1;
				//Iteratorを初期位置へ
				VComItr=Begin;
				GateItr=Begin;
			}

			//データ書き込み
			*((GateItr)++)=c;
		}
		//バッファへからの読み出しが区切り位置にいるか
		bool vic1::buffer::flowing() {
			//書き込み済みバッファ末尾まで読み出していない
			//あるいは、バッファ末尾まで書き込んでおらずかつデータは継続している（vcomから受信余地がある）
			return GateItr < VComItr
				|| (VComItr < End && pClient->flowing());
		}
		//バッファからの読み出し可能か
		bool vic1::buffer::can_getc() {
			//書き込み済みバッファ末尾まで読み出していない
			//あるいは、vcomから受信可能である
			return GateItr != VComItr
				|| pClient->can_getc();
		}
		//バッファからの読み出し
		unsigned char vic1::buffer::getc() {
			//バッファへの補充余地がある場合
			if(VComItr != End && pClient->flowing()) {
				//補充できる限り補充
				load();
			}//補充余地がないが全て送信しつくした => 新規バッファを作れる場合
			else if(GateItr == VComItr) {
				//Iteratorを初期位置へ
				GateItr=Begin;
				VComItr=Begin;

				//補充できる限り補充
				load();
			}

			//まだ残っている場合
			if(GateItr < VComItr) {
				return *((GateItr)++);
			} else {
				//エラー発報
				VIC1->Err=error_OVERREAD;
				return 0;
			}
		}
		//バッファへの書き込み/読み込みを終了し、バッファをクリアする
		void vic1::buffer::clear() {
			GateItr=Begin;
			VComItr=Begin;
		}
		//バッファへの読み込みをなかったことにする
		void vic1::buffer::cancel_get() { GateItr=Begin; }
		//バッファの内部データをすべて破棄し、vcomにもPacket終了を通知
		void vic1::buffer::errorflush() {
			GateItr=Begin;
			VComItr=Begin;
			pClient->flush();
		}
		//vcom自体がデータを持っているか
		bool vic1::buffer::vcom_can_getc() { return pClient->can_getc(); }
		//vcom自体がデータを持っているか
		bool vic1::buffer::vcom_flowing() { return pClient->flowing(); }
		//vcom自体がデータを持っているか
		void vic1::buffer::vcom_skip_get() { return pClient->skip_get(); }
		//vcom自体がデータを持っているか
		unsigned char vic1::buffer::vcom_get_ch() { return pClient->get_ch(); }

		//========== vic1::gate ===========
		//-------- Mode遷移4関数 ----------------
		//コマンド送信完了時
		void vic1::inform_send() {
			//ActiveCmdに登録
			ActiveCmd=Send;

			switch(Mode) {
			case mode_IDLE:
				//ひまになったし、話したい人がいた場合
				if(NextTergetCh!=ch_NULL) {
					TergetCh=NextTergetCh;
					NextTergetCh=ch_NULL;
					SendCnt=0;
					Send=cmd_WHAT;

				}//本当に単に暇な場合
				else {
					SendCnt=0;
					Send=cmd_NULL;
				}
				break;
			case mode_RESEND:
				//comから送信データを取得可能な場合
				if(Buffer.can_getc()) {
					//データ送信開始
					Mode=mode_SEND;
					SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
					Send=cmd_DATA;
				} else {
					//Stop送信
					Mode=mode_STOP;
					Send=cmd_STOP;
					SendCnt=0;
				}
				break;
			default:
				//送信を終えて待機状態に入る
				SendCnt=0;
				Send=cmd_NULL;
				break;
			}
		}
		//自分宛のコマンド受信完了
		void vic1::inform_recv() {
			RecvCnt=0;
			//ひまにしているとき
			if(Mode==mode_IDLE) {
				//相手からのコマンド内容で分岐
				switch(Recv) {
					//相手がStart要求をしてきた場合
				case cmd_START:
					//Ackを返し、自身をRecvにする
					Mode=mode_RECV;
					Buffer.clear();
					TergetCh=RecvCh;
					SendCnt=0;
					Send=cmd_ACK;
					break;
				case cmd_ERR:
					//無視する
					break;
				default:
					//おかしなことを言っているので、教えてあげる
					TergetCh=RecvCh;
					SendCnt=0;
					Send=cmd_ERR;
				}
				return;
			}

			//通信中に相手が知らない人から通信が来た場合
			if(RecvCh!=TergetCh) {
				//Start要求だった場合、切り替え予約
				if(Recv==cmd_START && NextTergetCh>RecvCh) {
					NextTergetCh=RecvCh;
					return;
				}
				//それ以外なら、無視
			}

			//返事が来たので、ひとまずタイムアウトリセット
			pTimeout->restart();
			//返事が来たので、タイムアウトのカウントを戻す
			TimeOutCnt=0;

			//エラーの場合は問答無用でIDLEへ
			if(Recv == cmd_ERR) {
				//次に話しかける相手が登録されていた場合
				if(NextTergetCh!=ch_NULL) {
					//WHATコマンドを返送し、自分はIDLEになる
					Mode=mode_IDLE;
					pTimeout->disable();
					TergetCh=NextTergetCh;
					NextTergetCh=ch_NULL;
					SendCnt=0;
					Send=cmd_WHAT;
				}//何の予定もない場合
				else {
					//モードをIdleにし、黙る
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_NULL;
				}
			}//Recv時
			else if(Mode==mode_RECV) {
				//データを送ってきた場合
				switch(Recv) {
				case cmd_START:
					//ACK送信
					SendCnt=0;
					Send=cmd_ACK;
					break;
				case cmd_DATA:
					//フラッシュ
					Buffer.flush();

					//受信に成功しているか確認
					if(Fail==0) {
						//ACK送信
						SendCnt=0;
						Send=cmd_ACK;
					} else {
						//フラグをおろす
						Fail=0;
						//NACK送信
						SendCnt=0;
						Send=cmd_NACK;
					}
					break;
				case cmd_STOP:
					//パケット終了通知
					Buffer.errorflush();

					//comから送信データを取得可能な場合(折り返しなのでflowingは気にしない)
					if(Buffer.can_getc()) {
						//送受信入れ替え要求
						Mode=mode_START;
						pTimeout->restart();
						pTimeout->enable();
						SendCnt=0;
						Send=cmd_NACK;
					}//comから送信データを取得できない場合
					else {
						//終了受け入れ
						Mode=mode_IDLE;
						SendCnt=0;
						Send=cmd_ACK;
					}
					break;
				default:
					//パケット終了通知
					Buffer.errorflush();

					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					break;
				}

				//ReecvModeの時には、データをバッファにいれてしまっているので、リセット。
				Buffer.clear();
				Fail=0;
			}//Startコマンドに対する返答
			else if(Mode==mode_START && (ActiveCmd==cmd_START || ActiveCmd==cmd_NACK)) {
				switch(Recv) {
					//相手が受諾した場合
				case cmd_ACK:
					//comから送信データを取得可能な場合
					if(Buffer.can_getc()) {
						//送信中だった場合
						if(Send!=cmd_NULL) {
							//ESCを一旦送信
							Mode=mode_RESEND;
							SendCnt=0;
							Send=cmd_ESC;
						}//即送信可能な場合
						else {
							//DATAを送信
							Mode=mode_SEND;
							SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
							Send=cmd_DATA;
						}
					}//comから送信データを取得できない場合
					else {
						//Stop送信
						Mode=mode_STOP;
						SendCnt=0;
						Send=cmd_STOP;
					}
					break;
					//相手が拒否してきた場合
				case cmd_NACK:
					//次に話しかける相手が登録されていた場合
					if(NextTergetCh!=ch_NULL) {
						//WHATコマンドを返送し、自分はIDLEになる
						Mode=mode_IDLE;
						pTimeout->disable();
						TergetCh=NextTergetCh;
						NextTergetCh=ch_NULL;
						SendCnt=0;
						Send=cmd_WHAT;
					}//何の予定もない場合
					else {
						//モードをIdleにし、黙る
						Mode=mode_IDLE;
						SendCnt=0;
						Send=cmd_NULL;
					}
					break;
					//相手がStart要求を重ねてきた場合
				case cmd_START:
					//相手より自分の方が大きなchのとき
					if(RecvCh < Ch) {
						//Ackを返し、自身をRecvにする
						Mode=mode_RECV;
						pTimeout->disable();
						SendCnt=0;
						Send=cmd_ACK;
					}
					//小さい場合は無視
					break;
					//それ以外のことを言っている場合
				default:
					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
					break;
				}
			}//DATAコマンドに対する返答
			else if(Mode==mode_SEND && ActiveCmd==cmd_DATA) {
				switch(Recv) {
					//相手が受信に成功していた場合
				case cmd_ACK:
					//comから送信データを取得可能な場合（DATAに継続性がある場合に限る）
					if(Buffer.vcom_can_getc() && Buffer.vcom_flowing()) {
						//送信中だった場合
						if(Send!=cmd_NULL) {
							//ESCを一旦送信
							Mode=mode_RESEND;
							SendCnt=0;
							Send=cmd_ESC;
						}//即送信可能な場合
						else {
							//DATAを送信
							Mode=mode_SEND;
							SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
							Send=cmd_DATA;
						}
					}//comから送信データを取得できない場合
					else {
						//Stop送信
						Mode=mode_STOP;
						SendCnt=0;
						Send=cmd_STOP;
					}
					break;
					//相手が受信に失敗していた場合
				case cmd_NACK:
					//バッファの読み出し位置を初期化
					Buffer.cancel_get();
					//送信中だった場合
					if(Send!=cmd_NULL) {
						//ESCを一旦送信
						Mode=mode_RESEND;
						SendCnt=0;
						Send=cmd_ESC;
					}//即送信可能な場合
					else {
						//DATAを送信
						Mode=mode_SEND;
						SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
						Send=cmd_DATA;
					}
					break;
				default:
					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
				}
			}//Stopコマンドに対する返答
			else if(Mode==mode_STOP && ActiveCmd==cmd_STOP) {
				switch(Recv) {
					//相手が受諾した場合
				case cmd_ACK:
					//次に話しかける相手が登録されていた場合
					if(NextTergetCh!=ch_NULL) {
						//WHATコマンドを返送し、自分はIDLEになる
						Mode=mode_IDLE;
						pTimeout->disable();
						TergetCh=NextTergetCh;
						NextTergetCh=ch_NULL;
						SendCnt=0;
						Send=cmd_WHAT;
					}//何の予定もない場合
					else {
						//モードをIdleにし、黙る
						Mode=mode_IDLE;
						pTimeout->disable();
						SendCnt=0;
						Send=cmd_NULL;
					}
					break;
					//相手が拒否してきた場合
				case cmd_NACK:
					//自身は受信モードにし、相手に受諾の意思を伝える
					Mode=mode_RECV;
					Buffer.clear();	//バッファをクリアして受信に備える
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ACK;
					break;
				default:
					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
				}
			} else {
				//おかしなことを言っているので、教えてあげる
				Mode=mode_IDLE;
				pTimeout->disable();
				SendCnt=0;
				Send=cmd_ERR;
			}
		}
		//データの有無を確認して、IDLEからSTARTへ遷移する関数
		bool vic1::checkData() {
			//IDLEである、送信中コマンドがない、送信したいデータがある、の３条件が満たされた場合のみ発動
			if(Mode==mode_IDLE && Send==cmd_NULL && Buffer.vcom_can_getc()) {
				//ch取得し、startモードへ移行
				TergetCh=Buffer.vcom_get_ch();
				Mode=mode_START;
				pTimeout->restart();
				pTimeout->enable();
				SendCnt=0;
				Send=cmd_START;
				return hmLib_true;
			}
			return hmLib_false;
		}
		//timeoutもモードを一部変えているでござる
		//vic1のtimeoutコマンド再送関数
		void vic1::timeout() {
			if(Mode==mode_START) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					Buffer.vcom_skip_get();
					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					return;
				}
				Send=cmd_START;
				SendCnt=0;
			} else if(Mode==mode_SEND) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					Buffer.vcom_skip_get();
					//ESCを一旦送信
					Mode=mode_RESEND;
					SendCnt=0;
					Send=cmd_ESC;
					return;
				}
				//ESCを一旦送信
				Mode=mode_RESEND;
				SendCnt=0;
				Send=cmd_ESC;
			} else if(Mode==mode_STOP) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					//おかしなことを言っているので、教えてあげる
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					return;
				}		Send=cmd_STOP;
				SendCnt=0;
			}
			++(TimeOutCnt);
		}

		//========== vic1 基本関数 ==========
		//vic1を初期化する 引数は、自分のチャンネル番号(0x00-0x0F)、相手のチャンネル番号、与える送受信用固定長バッファアドレス
		vic1::vic1(client& rClient_
			, const unsigned char Ch_
			, unsigned char* BufBegin
			, unsigned char* BufEnd
			, timeout_functions& Timeout_)
			: Buffer(this, &rClient_, BufBegin, BufEnd)
			, pTimeout(&Timeout_) {
			ActiveCmd=cmd_NULL;
			Ch=Ch_;
			Err=error_NULL;
			Fail=0x00;
			TimeOutCnt=0;

			Mode=mode_IDLE;
			NextTergetCh=ch_NULL;
			TergetCh=ch_NULL;
			Send=cmd_NULL;
			SendCnt=0;

			Recv=cmd_NULL;
			RecvCh=cmd_NULL;
			RecvCnt=0;
			//	maintask_start(task, 0, 0, 0);
		}		
		//vic1を初期化する
		vic1::vic1(const unsigned char Ch_
			, unsigned char* BufBegin
			, unsigned char* BufEnd
			, timeout_functions& Timeout_)
			: Buffer(this, BufBegin, BufEnd)
			, pTimeout(&Timeout_) {
			ActiveCmd=cmd_NULL;
			Ch=Ch_;
			Err=error_NULL;
			Fail=0x00;
			TimeOutCnt=0;

			Mode=mode_IDLE;
			NextTergetCh=ch_NULL;
			TergetCh=ch_NULL;
			Send=cmd_NULL;
			SendCnt=0;

			Recv=cmd_NULL;
			RecvCh=cmd_NULL;
			RecvCnt=0;
		}

		//vic1を終端化する
		vic1::~vic1() {
			pTimeout->disable();

			Ch=ch_NULL;
			TergetCh=ch_NULL;
			NextTergetCh=ch_NULL;

			Mode=mode_IDLE;
			Err=0;

			ActiveCmd=cmd_NULL;
			Send=cmd_NULL;
			SendCnt=0;
			Recv=cmd_NULL;
			RecvCnt=0;
			RecvCh=ch_NULL;

			//	maintask_stop(task);
		}
		bool vic1::open(client& rClient_) {
			return Buffer.open(&rClient_);
		}
		bool vic1::close() {
			return Buffer.close();
		}
		//========== gate interface =================
		//受信データ1byte目判別関数
		void vic1::putc_1byte(unsigned char c) {
			//自分宛のChか？
			if(Ch==(c&0x0F)) {
				RecvCh=(c>>4);
				++(RecvCnt);
			}//上記以外なら、少なくとも自分にとってコマンドではない
			else {
				//受信中ならデータ扱いをしておく
				if(Mode==mode_RECV) {
					Buffer.putc(c);
				}
			}
		}
		//受信データ2byte目判別関数
		void vic1::putc_2byte(unsigned char c1, unsigned char c2) {
			//今通信中の相手の場合
			if(RecvCh==TergetCh) {
				//とにかくコメントは聞いておく
				Recv=c2;
				++(RecvCnt);
			}//知らない人からStartコマンドが来た場合
			else if(c2==cmd_START) {
				//これも聞いておく
				Recv=c2;
				++(RecvCnt);
			}//上記以外なら、少なくとも自分にとってコマンドではない
			else {
				//カウンタを戻す
				RecvCnt=0;

				//受信中ならデータ扱いをしておく
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}
				putc_1byte(c2);
			}
		}
		//受信データ3byte目判別関数
		void vic1::putc_3byte(unsigned char c1, unsigned char c2, unsigned char c3) {
			//Trmn1
			if(c3==0x0d) {
				++(RecvCnt);
			}//上記以外なら、少なくとも1byte目の文字は自分にとってコマンドではない
			else {
				//カウンタを戻す
				RecvCnt=0;

				//受信中ならデータ扱いをしておく
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}
				putc_1byte(c2);

				if(RecvCnt==0) {
					putc_1byte(c3);
				} else if(RecvCnt==1) {
					putc_2byte(c2, c3);
				}
			}
		}
		//受信データ3byte目判別関数
		void vic1::putc_4byte(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4) {
			//Trmn1
			if(c4==0x0a) {
				//コマンド受信完了、発報
				inform_recv();
			}//上記以外なら、少なくとも1byte目の文字は自分にとってコマンドではない
			else {
				//カウンタを戻す
				RecvCnt=0;

				//受信中ならデータ扱いをしておく
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}

				putc_1byte(c2);

				if(RecvCnt==0) {
					putc_1byte(c3);

					if(RecvCnt==0) {
						putc_1byte(c4);
					} else if(RecvCnt==1) {
						putc_2byte(c3, c4);
					}

				} else if(RecvCnt==1) {
					putc_2byte(c2, c3);

					if(RecvCnt==0) {
						putc_1byte(c4);
					} else if(RecvCnt==1) {
						putc_2byte(c3, c4);
					} else if(RecvCnt==2) {
						putc_3byte(c2, c3, c4);
					}
				}

			}
		}
		//すでに利用可能になっているか
		bool vic1::is_open() {return Buffer.is_open();}
		//受信データを投げ入れ可能か確認 0:不可,1:可
		bool vic1::can_putc() {
			//受信データは常に受け入れ可能（読み捨てるかどうかは別）
			return hmLib_true;
		}
		//受信データを投げ入れる
		void vic1::putc(unsigned char c) {
			if(RecvCnt==0) {
				putc_1byte(c);
			} else if(RecvCnt==1) {
				putc_2byte(((RecvCh)<<4)|(Ch), c);
			} else if(RecvCnt==2) {
				putc_3byte(((RecvCh)<<4)|(Ch), Recv, c);
			} else {
				putc_4byte(((RecvCh)<<4)|(Ch), Recv, 0x0d, c);
			}
		}
		//受信データをフラッシュする（gate互換用、VIC内での処理はない）
		void vic1::flush() {}
		//送信データを呼び出し可能か確認 0:不可,1:可
		bool vic1::can_getc() {
			//送信モードに遷移できるとの通告があれば、遷移
			if(checkData())return hmLib_true;

			//送信モードで、送信データが全て送信完了していない場合
			if(Send==cmd_DATA && SendCnt==0xFF)return Buffer.can_getc();

			//コマンド送信中の場合
			if(Send!=cmd_NULL)return hmLib_true;

			return hmLib_false;
		}
		//送信データを呼び出す
		unsigned char vic1::getc() {
			unsigned char c;

			//送信できない場合はエラー
			if(!can_getc()) {
				Err=error_INVALID_GETC;
				return 0xC0;
			}

			//データ送信モードで、かつデータを送り終えていない場合
			if(Send==cmd_DATA && SendCnt==0xFF) {
				//データ送信中の場合は、データを送信処理
				c=Buffer.getc();

				//送信すべきデータを送り終えた場合は、SendCntを戻す
				if(Buffer.flowing()==0) {
					//SendCntを正しい値に戻す
					SendCnt=0;
				}
			} else {
				//SendCntが壊れていたら、エラー発報
				if(SendCnt>3) {
					Err=error_INCORRECT_SENDCNT;
					Send=cmd_ERR;
					SendCnt=0;
				}

				//コマンド送信中の場合は、コマンドを読む
				if(SendCnt==0) {
					c=((Ch<<4) | TergetCh);
					//送信カウンタを進める
					++(SendCnt);
				} else if(SendCnt==1) {
					switch(Send) {
					case cmd_ACK:
						c=cmd_ACK;
						break;
					case cmd_NACK:
						c=cmd_NACK;
						break;
					case cmd_START:
						c=cmd_START;
						break;
					case cmd_STOP:
						c=cmd_STOP;
						break;
					case cmd_DATA:
						c=cmd_DATA;
						break;
					default:
						Send=cmd_ERR;
						c=cmd_ERR;
						break;
					}
					//送信カウンタを進める
					++(SendCnt);
				} else if(SendCnt==2) {
					c=0x0d;
					//送信カウンタを進める
					++(SendCnt);
				} else if(SendCnt==3) {
					inform_send();
					c=0x0a;
				}

			}
			return c;
		}
		//送信データを流れ続けているか確認（VICの場合は、コマンド末尾が0,それ以外は1）
		bool vic1::flowing() { return Send!=cmd_NULL; }

		//============ vic functions ================
		//通信を強制的に終了させる
		void vic1::force_end() {
			Mode=mode_IDLE;
			Send=cmd_ERR;
			SendCnt=0;
			Recv=cmd_NULL;
			RecvCnt=0;
			ActiveCmd=cmd_NULL;
			Fail=0;
			TergetCh=ch_NULL;
			NextTergetCh=ch_NULL;

		}

	}
}
#
#endif
