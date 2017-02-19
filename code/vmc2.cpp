#ifndef HMR_CODE_VMC2_CPP_INC
#define HMR_CODE_VMC2_CPP_INC 200
#
#ifndef HMR_CODE_VMC2_INC
#	include"vmc2.hpp"
#endif

namespace hmr {
	namespace code {
		const unsigned char vmc2::PacStrt[4]={'#', '[', 0x00, ']'};
		const unsigned char vmc2::PacStrtSize=4;
		const unsigned char vmc2::PacTrmn[4]={'#', '|', 0x0d, 0x0a};
		const unsigned char vmc2::PacTrmnSize=4;

		//vmc2を初期化する
		vmc2::vmc2(unsigned char Ch_) :Ch(Ch_) {}
		vmc2::vmc2(unsigned char Ch_, client& rVCom1) :Ch(Ch_) {open(rVCom1);}
		//vmc2を終端化する
		vmc2::~vmc2() {close();}
		//VCom1を開く
		bool vmc2::open(client& rVCom1) {
			if(is_open())return true;

			pVCom1=&rVCom1;

			//受信側変数
			RecvErr=puterr_NULL;
			RecvMode=putmode_IDLE;
			RecvCnt=0;
			RecvCh=0xFF;

			//送信側変数
			SendErr=geterr_NULL;
			SendMode=getmode_STRT;
			SendCnt=0;

			return false;
		}
		//VCom1を閉じる
		bool vmc2::close() {
			if(!is_open())return true;

			//受信モードのままの場合
			if(RecvMode==putmode_DATA) {
				pVCom1->flush();
				RecvMode=putmode_IDLE;
				RecvCnt=0;
			}
			//送信モードのままの場合
			if(SendMode==getmode_DATA) {
				pVCom1->cancel_get();
				SendMode=getmode_STRT;
				SendCnt=0;
			}

			pVCom1=0;
		}
		//すでに利用可能になっているか
		bool vmc2::is_open() { return pVCom1!=0; }
		//フラッシュする（実質何もしない）
		void vmc2::flush() { return; }
		//受信データを投げ入れ可能か確認 0:不可,1:可
		bool vmc2::can_putc() {
			if(RecvMode==putmode_DATA) {
				return pVCom1->can_putc();
			}
			return 1;
		}
		//受信データを投げ入れる
		void vmc2::putc(unsigned char c) {
			unsigned char cnt;

			//受信できないのに投げ入れたらエラー
			if(!can_putc()) {
				RecvErr=puterr_INVALID_PUTC;
			}

			//非受信モード(RecvPacStrt待ち)
			if(RecvMode==putmode_IDLE) {
				//CH部分受信中にChと一致
				if(RecvCnt==2 && (c&0x0F)==Ch) {
					++(RecvCnt);

					//相手のChを受け取る
					RecvCh=((c>>4)&0x0F);
				}//非CH部分受信中にStrtと一致
				else if(RecvCnt!=2 && c==PacStrt[RecvCnt]) {
					++(RecvCnt);

					//すべてのSTRTを受信
					if(RecvCnt==PacStrtSize) {
						//モードをPac受信モードへ移行
						RecvMode=putmode_DATA;
						RecvCnt=0;

						//受信chを通知
						pVCom1->put_ch(RecvCh);
					}
				}//STRTは不一致だが先頭STRTとは一致
				else if(c==PacStrt[0]) {
					RecvCnt=1;
				}//それ以外
				else {
					RecvCnt=0;
				}
			}//受信モード(PacTrmn待ち)
			else {
				//受信文字がputmodeCnt番目のPacTrmn文字と一致
				if(PacTrmn[RecvCnt]==c) {
					//カウンター加算
					++(RecvCnt);

					//全RecvPacTrmn受信
					if(RecvCnt==PacTrmnSize) {
						//終了通知
						pVCom1->flush();

						//モードをPac受信モードへ移行
						RecvMode=putmode_IDLE;
						RecvCnt=0;
					}
				}//受信文字が0番目のPacTrmn文字と一致
				else if(PacTrmn[0]==c) {
					for(cnt=0; cnt<RecvCnt; ++cnt) {
						//受信できない場合
						if(pVCom1->can_putc()==0) {
							//エラー発報
							RecvErr=puterr_RECVREJECTED;
							//受信をストップ
							pVCom1->flush();
							//モードを戻す
							RecvMode=putmode_IDLE;
							RecvCnt=0;
							return;
						}

						//データを引き渡す
						pVCom1->putc(PacTrmn[cnt]);
					}
					//カウンターを1に(1文字は一致したから)
					RecvCnt=1;
				}//受信文字がPacTrmn文字と不一致
				else {
					for(cnt=0; cnt<RecvCnt; ++cnt) {
						//受信できない場合
						if(pVCom1->can_putc()==0) {
							//エラー発報
							RecvErr=puterr_RECVREJECTED;
							//受信をストップ
							pVCom1->flush();
							//モードを戻す
							RecvMode=putmode_IDLE;
							RecvCnt=0;
							return;
						}

						//データを引き渡す
						pVCom1->putc(PacTrmn[cnt]);
					}
					//カウンターリセット
					RecvCnt=0;

					//受信できない場合
					if(pVCom1->can_putc()==0) {
						//エラー発報
						RecvErr=puterr_RECVREJECTED;
						//受信をストップ
						pVCom1->flush();
						//モードを戻す
						RecvMode=putmode_IDLE;
						RecvCnt=0;
						return;
					}

					//データを引き渡す
					pVCom1->putc(c);

				}
			}
		}
		//送信データを呼び出し可能か確認 0:不可,1:可
		bool vmc2::can_getc() {
			if(SendMode==getmode_STRT&&SendCnt==0) {
				//送受信できるデータが存在するか
				return pVCom1->can_getc();
			} else if(SendMode==getmode_DATA) {
				//送信済み状態データ末尾にいる場合は、次はTRMNのはずなので1
				if(pVCom1->flowing()==0 && SendCnt!=0)return 1;
				return pVCom1->can_getc();
			}

			return 1;
		}
		//送信データを呼び出す
		unsigned char vmc2::getc() {
			unsigned char c;

			//getcできないときには，エラー
			if(!can_getc()) {
				SendCnt=geterr_INVALID_GETC;
				return 0xC0;
			}

			//非送信モード(SendPacStrt待ち)
			if(SendMode==getmode_STRT) {
				//CHフェーズの時
				if(SendCnt==2) {
					c = ((Ch)<<4)|(pVCom1->get_ch()&0x0F);
					++(SendCnt);
				}//固定文字列送信フェーズ
				else {
					c = PacStrt[SendCnt];
					++(SendCnt);

					//PacStrtを送り終わったら、送信用に初期化
					if(SendCnt>=4) {
						SendMode=getmode_DATA;
						SendCnt=0;
					}
				}
			}//Data送信モード
			else if(SendMode==getmode_DATA) {
				//データが終了した場合、TRMNに移行する
				if(pVCom1->flowing()==0 && SendCnt!=0) {
					//TRMN送信
					SendMode=getmode_TRMN;
					c=PacTrmn[0];
					SendCnt=1;
				}//sendに失敗する場合は強制終了
				else if(pVCom1->can_getc()==0) {
					//エラー発報
					SendErr=geterr_SENDREJECTED;

					//vcom後処理
					pVCom1->cancel_get();

					//TRMNを強制送信
					SendMode=getmode_TRMN;
					c=PacTrmn[0];
					SendCnt=1;
				} else {
					//データを送信
					c=pVCom1->getc();
					SendCnt=1;
				}
			}//TRMNを送る
			else if(SendMode==getmode_TRMN) {
				//SendPacTrmn送信
				c=PacTrmn[SendCnt];
				++(SendCnt);

				//PacTrmnを送り終わったら、送信用に初期化
				if(SendCnt>=PacTrmnSize) {
					SendMode=getmode_STRT;
					SendCnt=0;
					SendErr=0;
				}
			}
			return c;
		}
		//送信データがeof位置でないかを確認する
		bool vmc2::flowing() {
			return !(SendMode==getmode_STRT&&SendCnt==0);
		}
		//受信を強制的に終了させる
		void vmc2::force_end_get() {
			if(RecvMode==putmode_IDLE)return;
			//データ受信をなかったことにする
			pVCom1->flush();

			//vmc2がらみを後始末
			RecvMode=putmode_IDLE;
			RecvCnt=0;
		}
		//送信を強制的に終了させる
		void vmc2::force_end_put() {
			if(SendMode==getmode_STRT)return;

			//データ送信をなかったことにする
			pVCom1->cancel_get();

			SendMode=getmode_STRT;
			SendCnt=0;
		}
	}
}
#
#endif
