#ifndef HMR_VMC1X_C_INC
#define HMR_VMC1X_C_INC 102
#
#ifndef HMR_VMC1x_INC
#	include"hmrVMC1x.h"
#endif

#include<stdlib.h>

#define vmc1x_putmode_IDLE	0	//待機中
#define vmc1x_putmode_DATA	1	//受信中

#define vmc1x_getmode_STRT	0
#define vmc1x_getmode_DATA	1
#define vmc1x_getmode_TRMN	2

//static const unsigned char vmc1x_PacStrt[4]={'#', '[', 0x00, ']'};
//static const unsigned char vmc1x_PacStrtSize=4;
static const unsigned char vmc1x_PacTrmn[4]={'#', '#', 0x0d, 0x0a};
static const unsigned char vmc1x_PacTrmnSize=4;

//vmc1xを初期化する
void vmc1x_initialize(vmc1x* pVMC1x, vcom* pVCom, const unsigned char VMCID_[3], const unsigned char TVMCID_[3]) {
	//VCom,Ch
	pVMC1x->pVCom=pVCom;
	pVMC1x->VMCID[0]=VMCID_[0];
	pVMC1x->VMCID[1]=VMCID_[1];
	pVMC1x->VMCID[2]=VMCID_[2];
	pVMC1x->TVMCID[0]=TVMCID_[0];
	pVMC1x->TVMCID[1]=TVMCID_[1];
	pVMC1x->TVMCID[2]=TVMCID_[2];

	//受信側変数
	pVMC1x->RecvErr=vmc1x_puterr_NULL;
	pVMC1x->RecvMode=vmc1x_putmode_IDLE;
	pVMC1x->RecvCnt=0;
	pVMC1x->RecvCh=0xFF;

	//送信側変数
	pVMC1x->SendErr=vmc1x_geterr_NULL;
	pVMC1x->SendMode=vmc1x_getmode_STRT;
	pVMC1x->SendCnt=0;
}
//vmc1xを終端化する
void vmc1x_finalize(vmc1x* pVMC1x) {
	//受信モードのままの場合
	if(pVMC1x->RecvMode==vmc1x_putmode_DATA) {
		vcom1_flush(pVMC1x->pVCom);
		pVMC1x->RecvMode=vmc1x_putmode_IDLE;
		pVMC1x->RecvCnt=0;
	}
	//送信モードのままの場合
	if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		vcom1_cancel_get(pVMC1x->pVCom);
		pVMC1x->SendMode=vmc1x_getmode_STRT;
		pVMC1x->SendCnt=0;
	}
}
//フラッシュする（実質何もしない）
void vmc1x_flush(vmc1x* pVMC1x) { return; }
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian vmc1x_can_putc(vmc1x* pVMC1x) { 
	if(pVMC1x->RecvMode==vmc1x_putmode_DATA) {
		return vcom1_can_putc(pVMC1x->pVCom);
	}
	return 1;
}
//受信データを投げ入れる
void vmc1x_putc(vmc1x* pVMC1x, unsigned char c) {
	unsigned char cnt;
	//非受信モード(vmc1x_RecvPacStrt待ち)
	if(pVMC1x->RecvMode==vmc1x_putmode_IDLE) {
		//先頭識別記号の受信待
		if(pVMC1x->RecvCnt==0) {
			if(c=='#') {
				++(pVMC1x->RecvCnt);
			}
		}//先頭識別記号の受信済 ID確認中
		else {
			//IDが一致
			if(c==pVMC1x->VMCID[pVMC1x->RecvCnt-1]) {
				++(pVMC1x->RecvCnt);
				if(pVMC1x->RecvCnt==4) {
					//モードをPac受信モードへ移行
					pVMC1x->RecvMode=vmc1x_putmode_DATA;
					pVMC1x->RecvCnt=0;
				}
			}//IDは不一致だが先頭識別記号を受信
			else if(c=='#') {
				pVMC1x->RecvCnt=1;
			}//それ以外
			else {
				pVMC1x->RecvCnt=0;
			}
		}
	}//受信モード(PacTrmn待ち)
	else {
		//受信文字がputmodeCnt番目のPacTrmn文字と一致
		if(vmc1x_PacTrmn[pVMC1x->RecvCnt]==c) {
			//カウンター加算
			++(pVMC1x->RecvCnt);

			//全vmc1x_RecvPacTrmn受信
			if(pVMC1x->RecvCnt==vmc1x_PacTrmnSize) {
				//終了通知
				vcom1_flush(pVMC1x->pVCom);

				//モードをPac受信モードへ移行
				pVMC1x->RecvMode=vmc1x_putmode_IDLE;
				pVMC1x->RecvCnt=0;
			}
		}//受信文字が0番目のPacTrmn文字と一致
		else if(vmc1x_PacTrmn[0]==c) {
			for(cnt=0; cnt<pVMC1x->RecvCnt;++cnt) {
				//受信できない場合
				if(vcom1_can_putc(pVMC1x->pVCom)==0) {
					//エラー発報
					pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
					//受信をストップ
					vcom1_flush(pVMC1x->pVCom);
					//モードを戻す
					pVMC1x->RecvMode=vmc1x_putmode_IDLE;
					pVMC1x->RecvCnt=0;
					return;
				}

				//データを引き渡す
				vcom1_putc(pVMC1x->pVCom, vmc1x_PacTrmn[cnt]);
			}
			//カウンターを1に(1文字は一致したから)
			pVMC1x->RecvCnt=1;
		}//受信文字がPacTrmn文字と不一致
		else{
			for(cnt=0; cnt<pVMC1x->RecvCnt; ++cnt) {
				//受信できない場合
				if(vcom1_can_putc(pVMC1x->pVCom)==0) {
					//エラー発報
					pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
					//受信をストップ
					vcom1_flush(pVMC1x->pVCom);
					//モードを戻す
					pVMC1x->RecvMode=vmc1x_putmode_IDLE;
					pVMC1x->RecvCnt=0;
					return;
				}

				//データを引き渡す
				vcom1_putc(pVMC1x->pVCom, vmc1x_PacTrmn[cnt]);
			}
			//カウンターリセット
			pVMC1x->RecvCnt=0;

			//受信できない場合
			if(vcom1_can_putc(pVMC1x->pVCom)==0) {
				//エラー発報
				pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
				//受信をストップ
				vcom1_flush(pVMC1x->pVCom);
				//モードを戻す
				pVMC1x->RecvMode=vmc1x_putmode_IDLE;
				pVMC1x->RecvCnt=0;
				return;
			}
			
			//データを引き渡す
			vcom1_putc(pVMC1x->pVCom, c);

		}
	}
}
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian vmc1x_can_getc(vmc1x* pVMC1x) {
	if(pVMC1x->SendMode==vmc1x_getmode_STRT&&pVMC1x->SendCnt==0) {
		//送受信できるデータが存在するか
		return vcom1_can_getc(pVMC1x->pVCom);
	}else if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		//送信済み状態データ末尾にいる場合は、次はTRMNのはずなので1
		if(vcom1_flowing(pVMC1x->pVCom)==0 && pVMC1x->SendCnt!=0)return 1;
		return vcom1_can_getc(pVMC1x->pVCom);
	}

	return 1;
}
//送信データを呼び出す
unsigned char vmc1x_getc(vmc1x* pVMC1x) {
	unsigned char c=0x00;
	//非送信モード(vmc1x_SendPacStrt待ち)
	if(pVMC1x->SendMode==vmc1x_getmode_STRT) {
		//最初の文字の場合
		if(pVMC1x->SendCnt==0) {
			c='#';
			++(pVMC1x->SendCnt);
		//ID送付フェーズ
		} else {
			c=pVMC1x->TVMCID[pVMC1x->SendCnt-1];
			++(pVMC1x->SendCnt);
			//PacStrtを送り終わったら、送信用に初期化
			if(pVMC1x->SendCnt>=4) {
				pVMC1x->SendMode=vmc1x_getmode_DATA;
				pVMC1x->SendCnt=0;
			}
		}
	}//Data送信モード
	else if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		//データが終了した場合、TRMNに移行する
		if(vcom1_flowing(pVMC1x->pVCom)==0 && pVMC1x->SendCnt!=0) {
			//TRMN送信
			pVMC1x->SendMode=vmc1x_getmode_TRMN;
			c=vmc1x_PacTrmn[0];
			pVMC1x->SendCnt=1;
		}//sendに失敗する場合は強制終了
		else if(vcom1_can_getc(pVMC1x->pVCom)==0) {
			//エラー発報
			pVMC1x->SendErr=vmc1x_geterr_SENDREJECTED;

			//vcom後処理
			vcom1_cancel_get(pVMC1x->pVCom);

			//TRMNを強制送信
			pVMC1x->SendMode=vmc1x_getmode_TRMN;
			c=vmc1x_PacTrmn[0];
			pVMC1x->SendCnt=1;
		} else {
			//データを送信
			c=vcom1_getc(pVMC1x->pVCom);
			pVMC1x->SendCnt=1;
		}
	}//TRMNを送る
	else if(pVMC1x->SendMode==vmc1x_getmode_TRMN) {
		//vmc1x_SendPacTrmn送信
		c=vmc1x_PacTrmn[pVMC1x->SendCnt];
		++(pVMC1x->SendCnt);

		//PacTrmnを送り終わったら、送信用に初期化
		if(pVMC1x->SendCnt>=vmc1x_PacTrmnSize) {
			pVMC1x->SendMode=vmc1x_getmode_STRT;
			pVMC1x->SendCnt=0;
			pVMC1x->SendErr=0;
		}
	}
	return c;
}
//送信データがeof位置でないかを確認する
hmLib_boolian vmc1x_flowing(vmc1x* pVMC1x) {
	return !(pVMC1x->SendMode==vmc1x_getmode_STRT&&pVMC1x->SendCnt==0);
}
//受信を強制的に終了させる
void vmc1x_force_end_recv(vmc1x* pVMC1x) {
	if(pVMC1x->RecvMode==vmc1x_putmode_IDLE)return;
	//データ受信をなかったことにする
	vcom1_flush(pVMC1x->pVCom);

	//vmc1xがらみを後始末
	pVMC1x->RecvMode=vmc1x_putmode_IDLE;
	pVMC1x->RecvCnt=0;
}
//送信を強制的に終了させる
void vmc1x_force_end_send(vmc1x* pVMC1x) {
	if(pVMC1x->SendMode==vmc1x_getmode_STRT)return;

	//データ送信をなかったことにする
	vcom1_cancel_get(pVMC1x->pVCom);

	pVMC1x->SendMode=vmc1x_getmode_STRT;
	pVMC1x->SendCnt=0;
}
//送信エラーを取得する
unsigned char vmc1x_error_of_get(vmc1x* pVMC1x) { return pVMC1x->SendErr; }
//送信エラーをクリアする
void vmc1x_clear_error_of_get(vmc1x* pVMC1x) { pVMC1x->SendErr=0; }
//受信エラーを取得する
unsigned char vmc1x_error_of_put(vmc1x* pVMC1x) { return pVMC1x->RecvErr; }
//受信エラーをクリアする
void vmc1x_clear_error_of_put(vmc1x* pVMC1x) { pVMC1x->RecvErr=0; }
#
#endif
