#ifndef HMR_VMC1_C_INC
#define HMR_VMC1_C_INC 400
#
#define HMLIB_NOLIB
/*
vmc1_recvの仕様に要注意
	Trmn「候補」を受信した場合には、バッファには報告せずTrmnカウンターを回す
	最後までTrmnを受信した場合、そのまま終了=>Trmnはバッファには最後まで報告されない
	一方、途中でTrmnではない事が分かった場合には、本来報告すべきだった文字=Trmn[0～Trmnカウンター]をバッファに報告する
*/
#ifndef HMR_VMC1_INC
#	include"hmrVMC1.h"
#endif

#include<stdlib.h>

#define vmc1_RECVMODE_WAIT	0	//PacketStrt待ち
#define vmc1_RECVMODE_DTID	1	//DatIDを受信中
#define vmc1_RECVMODE_SIZE	2	//DatSizeを受信中
#define vmc1_RECVMODE_DATA	3	//Datを受信中
#define vmc1_SENDMODE_STRT	1
#define vmc1_SENDMODE_CHEK	2
#define vmc1_SENDMODE_DTID	3
#define vmc1_SENDMODE_SIZE	4
#define vmc1_SENDMODE_DATA	5
#define vmc1_SENDMODE_TRMN	6

//multi-threadで使えないため、廃止
//以後、VMCIDと直打ちで対応
//**************** config ***********************//
//static const unsigned char vmc1_RecvPacStrt[4]={'#','h','m','r'};
//static const unsigned char vmc1_RecvPacStrtSize=4;
//static const unsigned char vmc1_RecvPacTrmn[4]={'#','#',0x0d,0x0a};
//static const unsigned char vmc1_RecvPacTrmnSize=4;
//static const unsigned char vmc1_SendPacStrt[4]={'#','c','t','r'};
//static const unsigned char vmc1_SendPacStrtSize=4;
//static const unsigned char vmc1_SendPacTrmn[4]={'#','#',0x0d,0x0a};
//static const unsigned char vmc1_SendPacTrmnSize=4;
//***********************************************//
//vmc1PacStrt={'#',VMCID[3]}
static const unsigned char vmc1_PacTrmn[4]={'#','#',0x0d,0x0a};
static const unsigned char vmc1_PacTrmnSize=4;

//VMC1を初期化する
VMC1* vmc1_create
	(vmc1_bFp_v fp_can_iniRecvPac_
	,vmc1_vFp_v fp_iniRecvPac_
	,vmc1_vFp_uc fp_finRecvPac_
	,vmc1_bFp_v fp_can_iniRecvDat_
	,vmc1_vFp_did_dsize fp_iniRecvDat_
	,vmc1_vFp_uc fp_finRecvDat_
	,vmc1_bFp_v fp_can_recv_
	,vmc1_vFp_uc fp_recv_
	,vmc1_bFp_v fp_can_iniSendPac_
	,vmc1_vFp_v fp_iniSendPac_
	,vmc1_vFp_uc fp_finSendPac_
	,vmc1_bFp_v fp_can_existSendDat_
	,vmc1_bFp_v fp_existSendDat_
	,vmc1_bFp_v fp_can_iniSendDat_
	,vmc1_vFp_pdid_pdsize fp_iniSendDat_
	,vmc1_vFp_uc fp_finSendDat_
	,vmc1_bFp_v fp_can_send_
	,vmc1_ucFp_v fp_send_){
	VMC1* p;
	p=(VMC1*)malloc(sizeof(VMC1));
	p->fp_can_iniRecvPac=fp_can_iniRecvPac_;
	p->fp_iniRecvPac=fp_iniRecvPac_;
	p->fp_finRecvPac=fp_finRecvPac_;
	p->fp_can_iniRecvDat=fp_can_iniRecvDat_;
	p->fp_iniRecvDat=fp_iniRecvDat_;
	p->fp_finRecvDat=fp_finRecvDat_;
	p->fp_can_recv=fp_can_recv_;
	p->fp_recv=fp_recv_;
	p->fp_can_iniSendPac=fp_can_iniSendPac_;
	p->fp_iniSendPac=fp_iniSendPac_;
	p->fp_finSendPac=fp_finSendPac_;
	p->fp_can_existSendDat=fp_can_existSendDat_;
	p->fp_existSendDat=fp_existSendDat_;
	p->fp_can_iniSendDat=fp_can_iniSendDat_;
	p->fp_iniSendDat=fp_iniSendDat_;
	p->fp_finSendDat=fp_finSendDat_;
	p->fp_can_send=fp_can_send_;
	p->fp_send=fp_send_;
	return p;
}
//VMC1を終了する
void vmc1_release(VMC1* pVMC1){
	free(pVMC1);
}
//VMC1を初期化する
void vmc1_initialize(VMC1* pVMC1,const unsigned char VMCID_[3],const unsigned char TVMCID_[3]){
	pVMC1->VMCID[0]=VMCID_[0];
	pVMC1->VMCID[1]=VMCID_[1];
	pVMC1->VMCID[2]=VMCID_[2];
	pVMC1->TVMCID[0]=TVMCID_[0];
	pVMC1->TVMCID[1]=TVMCID_[1];
	pVMC1->TVMCID[2]=TVMCID_[2];
	pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
	pVMC1->RecvModeCnt=0;
	pVMC1->RecvDatCnt=0;
	pVMC1->RecvErr=0;
	pVMC1->RecvDatSize=0;
	pVMC1->SendMode=vmc1_SENDMODE_STRT;
	pVMC1->SendCnt=0;
	pVMC1->SendErr=0;
	pVMC1->SendDatSize=0;
}
//VMC1を終端化する
void vmc1_finalize(VMC1* pVMC1){return;}
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian vmc1_can_recv(VMC1* pVMC1){
	switch(pVMC1->RecvMode){
	case vmc1_RECVMODE_WAIT:
		return pVMC1->fp_can_iniRecvPac();
	case vmc1_RECVMODE_DTID:
		return 1;
	case vmc1_RECVMODE_SIZE:
		if(pVMC1->RecvDatCnt==1)return pVMC1->fp_can_iniRecvDat();
		else return 1;
	case vmc1_RECVMODE_DATA:
		return pVMC1->fp_can_recv();
	default:
		return 0;
	}
}
//受信データを投げ入れる（受信モードの場合の実際のデータの受信処理
void _vmc1_recv_data(VMC1* pVMC1,unsigned char c){
	//データid受信時
	if(pVMC1->RecvMode==vmc1_RECVMODE_DTID){
		//データIDを取得
		pVMC1->RecvDatID=c;
		++(pVMC1->RecvDatCnt);

		//データid受信完了時
		if(pVMC1->RecvDatCnt>=1){
			//モードをサイズ受信モード(SIZE)へ移行
			pVMC1->RecvMode=vmc1_RECVMODE_SIZE;
			pVMC1->RecvDatCnt=0;
			pVMC1->RecvDatSize=0;
		}
	}//データサイズ受信待
	else if(pVMC1->RecvMode==vmc1_RECVMODE_SIZE){
		//データサイズを取得
		pVMC1->RecvDatSize|=((vmc1_dsize_t)(c)<<(8*pVMC1->RecvDatCnt));
		++(pVMC1->RecvDatCnt);

		//データサイズ受信完了時
		if(pVMC1->RecvDatCnt>=2){
			//データをバッファが受信できない時
			if(!pVMC1->fp_can_iniRecvDat()){
				//PACを強制的に閉じる
				pVMC1->fp_finRecvPac(1);
				//モードをPac待機モードへ移行
				pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
				pVMC1->RecvModeCnt=0;
				pVMC1->RecvErr=vmc1_RECVERR_fail_to_iniRecvDat;
			}//受信すべきデータが存在する場合
			else{
				//モードをPac受信モードへ移行
				pVMC1->RecvMode=vmc1_RECVMODE_DATA;
				pVMC1->RecvDatCnt=0;
				//データが存在しない時(0byteデータ)
				if(pVMC1->RecvDatSize==0){
					//モードをID受信モードへ移行
					pVMC1->RecvMode=vmc1_RECVMODE_DTID;
					pVMC1->RecvDatID=0;
					pVMC1->RecvDatSize=0;
					pVMC1->RecvDatCnt=0;
				}else{
					pVMC1->fp_iniRecvDat(pVMC1->RecvDatID,pVMC1->RecvDatSize);
					pVMC1->RecvDatCnt=0;
				}
			}
		}
	}//データ受信中
	else if(pVMC1->RecvMode==vmc1_RECVMODE_DATA){
		//データをバッファが受信できない時
		if(pVMC1->fp_can_recv()==0){
			//DAT,PACを強制的に閉じる
			pVMC1->fp_finRecvDat(1);
			pVMC1->fp_finRecvPac(1);
			//モードをPac待機モードへ移行
			pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
			pVMC1->RecvModeCnt=0;
			pVMC1->RecvErr=vmc1_RECVERR_fail_to_recv;
		}else{
			//バッファへ挿入
			pVMC1->fp_recv(c);

			//カウンターリセット
			++(pVMC1->RecvDatCnt);

			//全Dat受信完了(idが含まれるので、すでに1文字加算しておく)
			if(pVMC1->RecvDatCnt >= pVMC1->RecvDatSize){
				pVMC1->fp_finRecvDat(0);
				//モードをDat受信モードへ移行
				pVMC1->RecvMode=vmc1_RECVMODE_DTID;
				pVMC1->RecvDatCnt=0;
				pVMC1->RecvDatID=0;
				pVMC1->RecvDatSize=0;
			}
		}
	}
}
//受信データを投げ入れる
void vmc1_recv(VMC1* pVMC1,unsigned char c){
	vmc1_dsize_t cnt;
	//非受信モード(vmc1_RecvPacStrt待ち)
	if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT){
		//Pacをそもそも開けない場合は、読み捨て
		if(!pVMC1->fp_can_iniRecvPac())return;
		//先頭識別記号の受信待
		if(pVMC1->RecvModeCnt==0){
			if(c=='#'){
				++(pVMC1->RecvModeCnt);
			}
		}//先頭識別記号の受信済 ID確認中
		else{
			//IDが一致
			if(c==pVMC1->VMCID[pVMC1->RecvModeCnt-1]){
				++(pVMC1->RecvModeCnt);
				if(pVMC1->RecvModeCnt==4){
					//モードをPac受信モードへ移行
					pVMC1->RecvMode=vmc1_RECVMODE_DTID;
					pVMC1->RecvModeCnt=0;
					pVMC1->RecvDatID=0;
					pVMC1->RecvDatSize=0;
					pVMC1->RecvDatCnt=0;
					pVMC1->fp_iniRecvPac();
				}
			}//IDは不一致だが先頭識別記号を受信
			else if(c=='#'){
				pVMC1->RecvModeCnt=1;
			}//それ以外
			else{
				pVMC1->RecvModeCnt=0;
			}
		}
	}//受信モード(PacTrmn待ち)
	else{
		//受信文字がRecvModeCnt番目のPacTrmn文字と一致
		if(vmc1_PacTrmn[pVMC1->RecvModeCnt]==c){
			//カウンター加算
			++(pVMC1->RecvModeCnt);
			//全vmc1_RecvPacTrmn受信
			if(pVMC1->RecvModeCnt==vmc1_PacTrmnSize){
				//モード修正
				if(pVMC1->RecvMode!=vmc1_RECVMODE_DTID && pVMC1->RecvMode!=vmc1_RECVMODE_WAIT){
					pVMC1->fp_finRecvDat(1);
					pVMC1->fp_finRecvPac(1);
				}else{
					pVMC1->fp_finRecvPac(0);
				}

				//モードをPac待機モードへ移行
				pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
				pVMC1->RecvModeCnt=0;
			}
		}//受信文字が0番目のPacTrmn文字と一致
		else if(vmc1_PacTrmn[0]==c){
			//Trmnに一致したこれまでの文字はTrmnでなかったことになるので、その分を改めて再受信
			for(cnt=0; cnt< pVMC1->RecvModeCnt;++cnt){
				_vmc1_recv_data(pVMC1,vmc1_PacTrmn[cnt]);
				if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)break;
			}
			//カウンターを1に(1文字は一致したから)
			pVMC1->RecvModeCnt=1;
		}//受信文字がPacTrmn文字と不一致だったが、これまでにTrmnと思ってパスした文字がある場合
		else if(pVMC1->RecvModeCnt>0){
			//これまでの文字はTrmnでなかったことになるので、その分を改めて再受信
			for(cnt=0; cnt< pVMC1->RecvModeCnt;++cnt){
				_vmc1_recv_data(pVMC1,vmc1_PacTrmn[cnt]);
				if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)break;
			}
			if(pVMC1->RecvMode!=vmc1_RECVMODE_WAIT)_vmc1_recv_data(pVMC1,c);
			//カウンターリセット
			pVMC1->RecvModeCnt=0;
		}//Trmnカウンターには引っかかっていない
		else{
			//通常の受信処理
			_vmc1_recv_data(pVMC1,c);
		}
	}
}
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian vmc1_can_send(VMC1* pVMC1){
	switch(pVMC1->SendMode){
	case vmc1_SENDMODE_STRT:
		return pVMC1->fp_can_iniSendPac();
	case vmc1_SENDMODE_CHEK:
		if(!pVMC1->fp_can_existSendDat())return 0;	//確認できないときは無理なので0
		if(!pVMC1->fp_existSendDat())return 1;		//送信すべきDatが存在しない=TRMN条件なので1
		return pVMC1->fp_can_iniSendDat();
	case vmc1_SENDMODE_DTID:
		return 1;
	case vmc1_SENDMODE_SIZE:
		return 1;
	case vmc1_SENDMODE_DATA:
		return pVMC1->fp_can_send();
	case vmc1_SENDMODE_TRMN:
		return 1;
	default:
		return 0;
	}
}
//送信データを呼び出す
unsigned char vmc1_send(VMC1* pVMC1){
	unsigned char c;
	//非送信モード(vmc1_SendPacStrt待ち)
	if(pVMC1->SendMode==vmc1_SENDMODE_STRT){
		//そもそもパケットを開けないときは、0を返す。
		if(!pVMC1->fp_can_iniSendPac())return 0;
		//最初の文字の場合
		if(pVMC1->SendCnt==0){
			c='#';
			++(pVMC1->SendCnt);
		//ID送付フェーズ
		}else{
			c=pVMC1->TVMCID[pVMC1->SendCnt-1];
			++(pVMC1->SendCnt);
			//PacStrtを送り終わったら、送信用に初期化
			if(pVMC1->SendCnt>=4){
				pVMC1->fp_iniSendPac();
				pVMC1->SendMode=vmc1_SENDMODE_CHEK;
				pVMC1->SendCnt=0;
			}
		}
	}
	else{
		//確認モードになっている場合
		if(pVMC1->SendMode==vmc1_SENDMODE_CHEK){
			//次のデータ存在を確認できない場合は，エラー終了
			if(!pVMC1->fp_can_existSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_existSendDat;
			}//次データが存在しない場合は，普通にTRMNに設定
			else if(!pVMC1->fp_existSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
			}//次データが取得できない場合は，エラー終了
			else if(!pVMC1->fp_can_iniSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_iniSendDat;
			}//それ以外の場合は，データID送信モードへ移行
			else{
				pVMC1->fp_iniSendDat(&(pVMC1->SendDatID),&(pVMC1->SendDatSize));
				pVMC1->SendMode=vmc1_SENDMODE_DTID;
				pVMC1->SendCnt=0;
			}
		}//DATAモードの場合も再度確認
		else if(pVMC1->SendMode==vmc1_SENDMODE_DATA){
			//sendに失敗する場合は強制終了
			if(!pVMC1->fp_can_send()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_send;
			}
		}

		//IDを送る
		if(pVMC1->SendMode==vmc1_SENDMODE_DTID){
			//IDを含めた数を送信するので、Size+1
			c=pVMC1->SendDatID;
			++(pVMC1->SendCnt);
			//送信完了時
			if(pVMC1->SendCnt==1){
				pVMC1->SendMode=vmc1_SENDMODE_SIZE;
				pVMC1->SendCnt=0;
			}
		}//サイズを送る
		else if(pVMC1->SendMode==vmc1_SENDMODE_SIZE){
			c=(unsigned char)((pVMC1->SendDatSize>>(pVMC1->SendCnt*8))&0xFF);
			++(pVMC1->SendCnt);
			//送信完了時
			if(pVMC1->SendCnt==2){
				if(pVMC1->SendDatSize>0){
					pVMC1->SendMode=vmc1_SENDMODE_DATA;
					pVMC1->SendCnt=0;
				}else{
					//送信の正常終了を通知
					pVMC1->fp_finSendDat(0);
					pVMC1->SendMode=vmc1_SENDMODE_CHEK;
					pVMC1->SendCnt=0;
				}
			}
		}//データを送る
		else if(pVMC1->SendMode==vmc1_SENDMODE_DATA){	
			c=pVMC1->fp_send();
			++(pVMC1->SendCnt);
			//全データ送信完了時
			if(pVMC1->SendCnt==pVMC1->SendDatSize){
				//送信の正常終了を通知
				pVMC1->fp_finSendDat(0);
				//次データの有無は今確認せず、確認モードへ移行
				pVMC1->SendMode=vmc1_SENDMODE_CHEK;
				pVMC1->SendCnt=0;
			}
		}//TRMNを送る
		else if(pVMC1->SendMode==vmc1_SENDMODE_TRMN){
			//vmc1_SendPacStrt1文字送信
			c=vmc1_PacTrmn[pVMC1->SendCnt];
			++(pVMC1->SendCnt);

			//PacStrtを送り終わったら、送信用に初期化
			if(pVMC1->SendCnt>=vmc1_PacTrmnSize){
				pVMC1->SendMode=vmc1_SENDMODE_STRT;
				pVMC1->SendCnt=0;
				pVMC1->fp_finSendPac(pVMC1->SendErr);
				pVMC1->SendErr=0;
			}
		}
	}
	return c;
}
//受信を強制的に終了させる
void vmc1_force_end_recv(VMC1* pVMC1,unsigned char Err){
	if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)return;
	if(pVMC1->RecvMode==vmc1_RECVMODE_DATA){
		pVMC1->fp_finRecvDat(Err);
	}
	pVMC1->fp_finRecvPac(Err);
	
	pVMC1->RecvDatCnt=0;
	pVMC1->RecvDatID=0;
	pVMC1->RecvDatSize=0;
	pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
	pVMC1->RecvModeCnt=0;
}
//送信を強制的に終了させる
void vmc1_force_end_send(VMC1* pVMC1,unsigned char Err){
	if(pVMC1->SendMode==vmc1_SENDMODE_STRT)return;
	if(pVMC1->SendMode==vmc1_SENDMODE_DATA){
		pVMC1->fp_finSendDat(Err);
	}
	pVMC1->fp_finSendPac(Err);

	pVMC1->SendCnt=0;
	pVMC1->SendDatID=0;
	pVMC1->SendDatSize=0;
	pVMC1->SendErr=0;
	pVMC1->SendMode=vmc1_SENDMODE_STRT;
}
//送信エラーを取得する
unsigned char vmc1_getSendErr(VMC1* pVMC1){return pVMC1->SendErr;}
//送信エラーをクリアする
void vmc1_clearSendErr(VMC1* pVMC1){pVMC1->SendErr=0;}
//受信エラーを取得する
unsigned char vmc1_getRecvErr(VMC1* pVMC1){return pVMC1->RecvErr;}
//受信エラーをクリアする
void vmc1_clearRecvErr(VMC1* pVMC1){pVMC1->RecvErr=0;}
#
#endif
