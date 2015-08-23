#ifndef HMR_VMC2_C_INC
#define HMR_VMC2_C_INC 102
#
#ifndef HMR_VMC2_INC
#	include"hmrVMC2.h"
#endif

#include<stdlib.h>

#define vmc2_putmode_IDLE	0	//待機中
#define vmc2_putmode_DATA	1	//受信中

#define vmc2_getmode_STRT	0
#define vmc2_getmode_DATA	1
#define vmc2_getmode_TRMN	2

static const unsigned char vmc2_PacStrt[4]={'#', '[', 0x00, ']'};
static const unsigned char vmc2_PacStrtSize=4;
static const unsigned char vmc2_PacTrmn[4]={'#', '|', 0x0d, 0x0a};
static const unsigned char vmc2_PacTrmnSize=4;

//vmc2を初期化する
void vmc2_initialize(vmc2* pVMC2, vcom* pVCom, unsigned char Ch) {
	//VCom,Ch
	pVMC2->pVCom=pVCom;
	pVMC2->Ch=Ch;

	//受信側変数
	pVMC2->RecvErr=vmc2_puterr_NULL;
	pVMC2->RecvMode=vmc2_putmode_IDLE;
	pVMC2->RecvCnt=0;
	pVMC2->RecvCh=0xFF;

	//送信側変数
	pVMC2->SendErr=vmc2_geterr_NULL;
	pVMC2->SendMode=vmc2_getmode_STRT;
	pVMC2->SendCnt=0;
}
//vmc2を終端化する
void vmc2_finalize(vmc2* pVMC2) {
	//受信モードのままの場合
	if(pVMC2->RecvMode==vmc2_putmode_DATA) {
		vcom1_flush(pVMC2->pVCom);
		pVMC2->RecvMode=vmc2_putmode_IDLE;
		pVMC2->RecvCnt=0;
	}
	//送信モードのままの場合
	if(pVMC2->SendMode==vmc2_getmode_DATA) {
		vcom1_cancel_get(pVMC2->pVCom);
		pVMC2->SendMode=vmc2_getmode_STRT;
		pVMC2->SendCnt=0;
	}
}
//フラッシュする（実質何もしない）
void vmc2_flush(vmc2* pVMC2) { return; }
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian vmc2_can_putc(vmc2* pVMC2) { 
	if(pVMC2->RecvMode==vmc2_putmode_DATA) {
		return vcom1_can_putc(pVMC2->pVCom);
	}
	return 1;
}
//受信データを投げ入れる
void vmc2_putc(vmc2* pVMC2, unsigned char c) {
	unsigned char cnt;

	//受信できないのに投げ入れたらエラー
	if(!vmc2_can_putc(pVMC2)) {
		pVMC2->RecvErr=vmc2_puterr_INVALID_PUTC;
	}

	//非受信モード(vmc2_RecvPacStrt待ち)
	if(pVMC2->RecvMode==vmc2_putmode_IDLE) {
		//CH部分受信中にChと一致
		if(pVMC2->RecvCnt==2 && (c&0x0F)==pVMC2->Ch) {
			++(pVMC2->RecvCnt);

			//相手のChを受け取る
			pVMC2->RecvCh=((c>>4)&0x0F);
		}//非CH部分受信中にStrtと一致
		else if(pVMC2->RecvCnt!=2 && c==vmc2_PacStrt[pVMC2->RecvCnt]) {
			++(pVMC2->RecvCnt);

			//すべてのSTRTを受信
			if(pVMC2->RecvCnt==vmc2_PacStrtSize) {
				//モードをPac受信モードへ移行
				pVMC2->RecvMode=vmc2_putmode_DATA;
				pVMC2->RecvCnt=0;

				//受信chを通知
				vcom1_put_ch(pVMC2->pVCom,pVMC2->RecvCh);
			}
		}//STRTは不一致だが先頭STRTとは一致
		else if(c==vmc2_PacStrt[0]) {
			pVMC2->RecvCnt=1;
		}//それ以外
		else {
			pVMC2->RecvCnt=0;
		}
	}//受信モード(PacTrmn待ち)
	else {
		//受信文字がputmodeCnt番目のPacTrmn文字と一致
		if(vmc2_PacTrmn[pVMC2->RecvCnt]==c) {
			//カウンター加算
			++(pVMC2->RecvCnt);

			//全vmc2_RecvPacTrmn受信
			if(pVMC2->RecvCnt==vmc2_PacTrmnSize) {
				//終了通知
				vcom1_flush(pVMC2->pVCom);

				//モードをPac受信モードへ移行
				pVMC2->RecvMode=vmc2_putmode_IDLE;
				pVMC2->RecvCnt=0;
			}
		}//受信文字が0番目のPacTrmn文字と一致
		else if(vmc2_PacTrmn[0]==c) {
			for(cnt=0; cnt<pVMC2->RecvCnt;++cnt) {
				//受信できない場合
				if(vcom1_can_putc(pVMC2->pVCom)==0) {
					//エラー発報
					pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
					//受信をストップ
					vcom1_flush(pVMC2->pVCom);
					//モードを戻す
					pVMC2->RecvMode=vmc2_putmode_IDLE;
					pVMC2->RecvCnt=0;
					return;
				}

				//データを引き渡す
				vcom1_putc(pVMC2->pVCom, vmc2_PacTrmn[cnt]);
			}
			//カウンターを1に(1文字は一致したから)
			pVMC2->RecvCnt=1;
		}//受信文字がPacTrmn文字と不一致
		else{
			for(cnt=0; cnt<pVMC2->RecvCnt; ++cnt) {
				//受信できない場合
				if(vcom1_can_putc(pVMC2->pVCom)==0) {
					//エラー発報
					pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
					//受信をストップ
					vcom1_flush(pVMC2->pVCom);
					//モードを戻す
					pVMC2->RecvMode=vmc2_putmode_IDLE;
					pVMC2->RecvCnt=0;
					return;
				}

				//データを引き渡す
				vcom1_putc(pVMC2->pVCom, vmc2_PacTrmn[cnt]);
			}
			//カウンターリセット
			pVMC2->RecvCnt=0;

			//受信できない場合
			if(vcom1_can_putc(pVMC2->pVCom)==0) {
				//エラー発報
				pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
				//受信をストップ
				vcom1_flush(pVMC2->pVCom);
				//モードを戻す
				pVMC2->RecvMode=vmc2_putmode_IDLE;
				pVMC2->RecvCnt=0;
				return;
			}
			
			//データを引き渡す
			vcom1_putc(pVMC2->pVCom, c);

		}
	}
}
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian vmc2_can_getc(vmc2* pVMC2) {
	if(pVMC2->SendMode==vmc2_getmode_STRT&&pVMC2->SendCnt==0) {
		//送受信できるデータが存在するか
		return vcom1_can_getc(pVMC2->pVCom);
	}else if(pVMC2->SendMode==vmc2_getmode_DATA) {
		//送信済み状態データ末尾にいる場合は、次はTRMNのはずなので1
		if(vcom1_flowing(pVMC2->pVCom)==0 && pVMC2->SendCnt!=0)return 1;
		return vcom1_can_getc(pVMC2->pVCom);
	}

	return 1;
}
//送信データを呼び出す
unsigned char vmc2_getc(vmc2* pVMC2) {
	unsigned char c;

	//getcできないときには，エラー
	if(!vmc2_can_getc(pVMC2)) {
		pVMC2->SendCnt=vmc2_geterr_INVALID_GETC;
		return 0xC0;
	}

	//非送信モード(vmc2_SendPacStrt待ち)
	if(pVMC2->SendMode==vmc2_getmode_STRT) {
		//CHフェーズの時
		if(pVMC2->SendCnt==2) {
			c = ((pVMC2->Ch)<<4)|(vcom1_get_ch(pVMC2->pVCom)&0x0F);
			++(pVMC2->SendCnt);
		}//固定文字列送信フェーズ
		else{
			c = vmc2_PacStrt[pVMC2->SendCnt];
			++(pVMC2->SendCnt);

			//PacStrtを送り終わったら、送信用に初期化
			if(pVMC2->SendCnt>=4) {
				pVMC2->SendMode=vmc2_getmode_DATA;
				pVMC2->SendCnt=0;
			}
		}
	}//Data送信モード
	else if(pVMC2->SendMode==vmc2_getmode_DATA) {
		//データが終了した場合、TRMNに移行する
		if(vcom1_flowing(pVMC2->pVCom)==0 && pVMC2->SendCnt!=0) {
			//TRMN送信
			pVMC2->SendMode=vmc2_getmode_TRMN;
			c=vmc2_PacTrmn[0];
			pVMC2->SendCnt=1;
		}//sendに失敗する場合は強制終了
		else if(vcom1_can_getc(pVMC2->pVCom)==0) {
			//エラー発報
			pVMC2->SendErr=vmc2_geterr_SENDREJECTED;

			//vcom後処理
			vcom1_cancel_get(pVMC2->pVCom);

			//TRMNを強制送信
			pVMC2->SendMode=vmc2_getmode_TRMN;
			c=vmc2_PacTrmn[0];
			pVMC2->SendCnt=1;
		} else {
			//データを送信
			c=vcom1_getc(pVMC2->pVCom);
			pVMC2->SendCnt=1;
		}
	}//TRMNを送る
	else if(pVMC2->SendMode==vmc2_getmode_TRMN) {
		//vmc2_SendPacTrmn送信
		c=vmc2_PacTrmn[pVMC2->SendCnt];
		++(pVMC2->SendCnt);

		//PacTrmnを送り終わったら、送信用に初期化
		if(pVMC2->SendCnt>=vmc2_PacTrmnSize) {
			pVMC2->SendMode=vmc2_getmode_STRT;
			pVMC2->SendCnt=0;
			pVMC2->SendErr=0;
		}
	}
	return c;
}
//送信データがeof位置でないかを確認する
hmLib_boolian vmc2_flowing(vmc2* pVMC2) {
	return !(pVMC2->SendMode==vmc2_getmode_STRT&&pVMC2->SendCnt==0);
}
//受信を強制的に終了させる
void vmc2_force_end_get(vmc2* pVMC2) {
	if(pVMC2->RecvMode==vmc2_putmode_IDLE)return;
	//データ受信をなかったことにする
	vcom1_flush(pVMC2->pVCom);

	//vmc2がらみを後始末
	pVMC2->RecvMode=vmc2_putmode_IDLE;
	pVMC2->RecvCnt=0;
}
//送信を強制的に終了させる
void vmc2_force_end_put(vmc2* pVMC2) {
	if(pVMC2->SendMode==vmc2_getmode_STRT)return;

	//データ送信をなかったことにする
	vcom1_cancel_get(pVMC2->pVCom);

	pVMC2->SendMode=vmc2_getmode_STRT;
	pVMC2->SendCnt=0;
}
//送信エラーを取得する
unsigned char vmc2_error_of_get(vmc2* pVMC2) { return pVMC2->SendErr; }
//送信エラーをクリアする
void vmc2_clear_error_of_get(vmc2* pVMC2) { pVMC2->SendErr=0; }
//受信エラーを取得する
unsigned char vmc2_error_of_put(vmc2* pVMC2) { return pVMC2->RecvErr; }
//受信エラーをクリアする
void vmc2_clear_error_of_put(vmc2* pVMC2) { pVMC2->RecvErr=0; }
#
#endif
