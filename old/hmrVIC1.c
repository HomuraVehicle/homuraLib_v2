#ifndef HMR_VIC1_C_INC
#define HMR_VIC1_C_INC 102
#
#ifndef CRC8_INC
#	include"crc8.h"
#endif
#ifndef HMR_VIC1_INC
#	include"hmrVIC1.h"
#endif
#include<stdlib.h>

#define vic1_MaxTimeOutNum 5
//ch
#define vic1_ch_NULL		0xFF

//Cmd
#define vic1_cmd_ACK		0x00
#define vic1_cmd_NACK		0x10
#define vic1_cmd_START		0x20
#define vic1_cmd_STOP		0x30
#define vic1_cmd_DATA		0x40
#define vic1_cmd_ESC		0x50
#define vic1_cmd_WHAT		0x60
#define vic1_cmd_ERR		0xF0
#define vic1_cmd_NULL		0xFF

//Mode
#define vic1_mode_IDLE		0x00
#define vic1_mode_START		0x10
#define vic1_mode_SEND		0x20
#define vic1_mode_RESEND	0x21
#define vic1_mode_STOP		0x30
#define vic1_mode_RECV		0x40

//========== vic1::buf ===========
//補充できる限り補充
void x_vic1_buf_load(vic1* pVIC1) {
	//受信可能である限り実行
	while(vcom1_can_getc(pVIC1->pVCom)){
		//実際に受信
		*((pVIC1->_buf_VComItr)++) = vcom1_getc(pVIC1->pVCom);

		//バッファがいっぱいになった場合、あるいはvcomからの送信が途切れた場合
		if(pVIC1->_buf_VComItr+1 == pVIC1->_buf_End || vcom1_flowing(pVIC1->pVCom)==0) {
			//crc8を計算して、末尾に書き込む
			*(pVIC1->_buf_VComItr) = crc8_puts(0x00,pVIC1->_buf_Begin,pVIC1->_buf_VComItr - pVIC1->_buf_Begin);
                        ++(pVIC1->_buf_VComItr);
			break;
		}
	}
}
//バッファ初期化
void _vic1_buf_initialize(vic1* pVIC1, unsigned char* BufBegin, unsigned char* BufEnd) {
	pVIC1->_buf_Begin=BufBegin;
	pVIC1->_buf_End=BufEnd;
	pVIC1->_buf_VComItr=BufBegin;
	pVIC1->_buf_GateItr=BufBegin;
}
//バッファ終端化
void _vic1_buf_finalize(vic1* pVIC1) {
	pVIC1->_buf_Begin=0;
	pVIC1->_buf_End=0;
	pVIC1->_buf_VComItr=0;
	pVIC1->_buf_GateItr=0;
}
//バッファへ書き込んだデータを処理
void _vic1_buf_flush(vic1* pVIC1) {
	unsigned char CRC8;

	//すでに失敗していれば無視
	if(pVIC1->Fail==1)return;
	
	//そもそも受信していなければ無視
	if(pVIC1->_buf_GateItr==pVIC1->_buf_Begin) {
		//失敗したことを通知
		pVIC1->Fail=1;
		//Iteratorを初期位置へ
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;

		return;
	}

	//1byte戻す
	--(pVIC1->_buf_GateItr);

	//GateItrまでのチェックサムを演算
	CRC8=crc8_puts(0x00, pVIC1->_buf_Begin, pVIC1->_buf_GateItr - pVIC1->_buf_Begin);

	//チェックサムに失敗している場合は破棄
	if(*(pVIC1->_buf_GateItr)!=CRC8) {
		//失敗したことを通知
		pVIC1->Fail=1;
		//Iteratorを初期位置へ
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;

		return;
	}

	//データセーブ
	//VComに順次投げ入れる
	pVIC1->_buf_VComItr = pVIC1->_buf_Begin;
	while(pVIC1->_buf_VComItr!= pVIC1->_buf_GateItr) {
		if(vcom1_can_putc(pVIC1->pVCom)==0) {
			//エラー発報
			pVIC1->Err=vic1_error_SAVE_REJECTED;
			break;
		}
		vcom1_putc(pVIC1->pVCom, *(pVIC1->_buf_VComItr++));
	}

	//Iteratorを初期位置へ
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
}
//バッファの内部データをすべて破棄し、vcomにもPacket終了を通知
void _vic1_buf_errorflush(vic1* pVIC1) {
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
	vcom1_flush(pVIC1->pVCom);
}
//バッファに書き込み可能か
hmLib_boolian _vic1_buf_can_putc(vic1* pVIC1) {
	//バッファに空きがある場合
	//	送信と違ってflushするまでどれがCheckSumかわからないので、
	//	1byteあけておく必要がない点に注意
	return pVIC1->_buf_GateItr < pVIC1->_buf_End;
}
//バッファへ書き込み
void _vic1_buf_putc(vic1* pVIC1, unsigned char c) {
	//書き込み可能でない場合、エラー発報
	if(!_vic1_buf_can_putc(pVIC1)) {
		//エラー発報
		pVIC1->Err=vic1_error_OVERFLOW;
		//失敗したことを通知
		pVIC1->Fail=1;
		//Iteratorを初期位置へ
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	}

	//データ書き込み
	*((pVIC1->_buf_GateItr)++)=c;
}
//バッファへからの読み出しが区切り位置にいるか
hmLib_boolian _vic1_buf_flowing(vic1* pVIC1) {
	//書き込み済みバッファ末尾まで読み出していない
	//あるいは、バッファ末尾まで書き込んでおらずかつデータは継続している（vcomから受信余地がある）
	return pVIC1->_buf_GateItr < pVIC1->_buf_VComItr
		|| (pVIC1->_buf_VComItr < pVIC1->_buf_End && vcom1_flowing(pVIC1->pVCom));
}
//バッファからの読み出し可能か
hmLib_boolian _vic1_buf_can_getc(vic1* pVIC1) {
	//書き込み済みバッファ末尾まで読み出していない
	//あるいは、vcomから受信可能である
	return pVIC1->_buf_GateItr != pVIC1->_buf_VComItr
		|| vcom1_can_getc(pVIC1->pVCom);
}
//バッファからの読み出し
unsigned char _vic1_buf_getc(vic1* pVIC1) {
	//バッファへの補充余地がある場合
	if(pVIC1->_buf_VComItr != pVIC1->_buf_End && vcom1_flowing(pVIC1->pVCom)){
		//補充できる限り補充
		x_vic1_buf_load(pVIC1);
	}//補充余地がないが全て送信しつくした => 新規バッファを作れる場合
	else if(pVIC1->_buf_GateItr == pVIC1->_buf_VComItr) {
		//Iteratorを初期位置へ
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;

		//補充できる限り補充
		x_vic1_buf_load(pVIC1);
	}

	//まだ残っている場合
	if(pVIC1->_buf_GateItr < pVIC1->_buf_VComItr) {
		return *((pVIC1->_buf_GateItr)++);
	} else {
		//エラー発報
		pVIC1->Err=vic1_error_OVERREAD;
		return 0;
	}
}
//バッファへの書き込み/読み込みを終了し、バッファをクリアする
void _vic1_buf_clear(vic1* pVIC1) {
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
}
//バッファへの読み込みをなかったことにする
void _vic1_buf_cancel_get(vic1* pVIC1) { pVIC1->_buf_GateItr=pVIC1->_buf_Begin; }

//========== vic1::gate ===========
//-------- Mode遷移4関数 ----------------
//コマンド送信完了時
void _vic1_inform_send(vic1* pVIC1) {
	//ActiveCmdに登録
	pVIC1->ActiveCmd=pVIC1->Send;

	switch(pVIC1->Mode) {
	case vic1_mode_IDLE:
		//ひまになったし、話したい人がいた場合
		if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
			pVIC1->TergetCh=pVIC1->NextTergetCh;
			pVIC1->NextTergetCh=vic1_ch_NULL;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_WHAT;

		}//本当に単に暇な場合
		else {
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_NULL;
		}
		break;
	case vic1_mode_RESEND:
		//comから送信データを取得可能な場合
		if(_vic1_buf_can_getc(pVIC1)) {
			//データ送信開始
			pVIC1->Mode=vic1_mode_SEND;
			pVIC1->SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
			pVIC1->Send=vic1_cmd_DATA;
		} else {
			//Stop送信
			pVIC1->Mode=vic1_mode_STOP;
			pVIC1->Send=vic1_cmd_STOP;
			pVIC1->SendCnt=0;
		}
		break;
	default:
		//送信を終えて待機状態に入る
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_NULL;
		break;
	}
}
//自分宛のコマンド受信完了
void _vic1_inform_recv(vic1* pVIC1) {
	pVIC1->RecvCnt=0;
	//ひまにしているとき
	if(pVIC1->Mode==vic1_mode_IDLE) {
		//相手からのコマンド内容で分岐
		switch(pVIC1->Recv) {
			//相手がStart要求をしてきた場合
		case vic1_cmd_START:
			//Ackを返し、自身をRecvにする
			pVIC1->Mode=vic1_mode_RECV;
			_vic1_buf_clear(pVIC1);
			pVIC1->TergetCh=pVIC1->RecvCh;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		case vic1_cmd_ERR:
			//無視する
			break;
		default:
			//おかしなことを言っているので、教えてあげる
			pVIC1->TergetCh=pVIC1->RecvCh;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
		return;
	}

	//通信中に相手が知らない人から通信が来た場合
	if(pVIC1->RecvCh!=pVIC1->TergetCh) {
		//Start要求だった場合、切り替え予約
		if(pVIC1->Recv==vic1_cmd_START && pVIC1->NextTergetCh>pVIC1->RecvCh) {
			pVIC1->NextTergetCh=pVIC1->RecvCh;
			return;
		}
		//それ以外なら、無視
	}

	//返事が来たので、ひとまずタイムアウトリセット
	pVIC1->Fp_timeout_restart();
	//返事が来たので、タイムアウトのカウントを戻す
	pVIC1->TimeOutCnt=0;

	//エラーの場合は問答無用でIDLEへ
	if(pVIC1->Recv == vic1_cmd_ERR){
		//次に話しかける相手が登録されていた場合
		if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
			//WHATコマンドを返送し、自分はIDLEになる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->TergetCh=pVIC1->NextTergetCh;
			pVIC1->NextTergetCh=vic1_ch_NULL;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_WHAT;
		}//何の予定もない場合
		else {
			//モードをIdleにし、黙る
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_NULL;
		}
	}//Recv時
	else if(pVIC1->Mode==vic1_mode_RECV) {
		//データを送ってきた場合
		switch(pVIC1->Recv) {
		case vic1_cmd_START:
			//ACK送信
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		case vic1_cmd_DATA:
			//フラッシュ
			_vic1_buf_flush(pVIC1);

			//受信に成功しているか確認
			if(pVIC1->Fail==0) {
				//ACK送信
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			} else {
				//フラグをおろす
				pVIC1->Fail=0;
				//NACK送信
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NACK;
			}
			break;
		case vic1_cmd_STOP:
			//パケット終了通知
			_vic1_buf_errorflush(pVIC1);

			//comから送信データを取得可能な場合(折り返しなのでflowingは気にしない)
			if(_vic1_buf_can_getc(pVIC1)) {
				//送受信入れ替え要求
				pVIC1->Mode=vic1_mode_START;
				pVIC1->Fp_timeout_restart();
				pVIC1->Fp_timeout_enable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NACK;
			}//comから送信データを取得できない場合
			else {
				//終了受け入れ
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			}
			break;
		default:
			//パケット終了通知
			_vic1_buf_errorflush(pVIC1);

			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			break;
		}

		//ReecvModeの時には、データをバッファにいれてしまっているので、リセット。
		_vic1_buf_clear(pVIC1);
		pVIC1->Fail=0;
	}//Startコマンドに対する返答
	else if(pVIC1->Mode==vic1_mode_START && (pVIC1->ActiveCmd==vic1_cmd_START || pVIC1->ActiveCmd==vic1_cmd_NACK)) {
		switch(pVIC1->Recv) {
		//相手が受諾した場合
		case vic1_cmd_ACK:
			//comから送信データを取得可能な場合
			if(_vic1_buf_can_getc(pVIC1)) {
				//送信中だった場合
				if(pVIC1->Send!=vic1_cmd_NULL) {
					//ESCを一旦送信
					pVIC1->Mode=vic1_mode_RESEND;
					pVIC1->SendCnt=0;
					pVIC1->Send=vic1_cmd_ESC;
				}//即送信可能な場合
				else {
					//DATAを送信
					pVIC1->Mode=vic1_mode_SEND;
					pVIC1->SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
					pVIC1->Send=vic1_cmd_DATA;
				}
			}//comから送信データを取得できない場合
			else {
				//Stop送信
				pVIC1->Mode=vic1_mode_STOP;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_STOP;
			}
			break;
			//相手が拒否してきた場合
		case vic1_cmd_NACK:
			//次に話しかける相手が登録されていた場合
			if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
				//WHATコマンドを返送し、自分はIDLEになる
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->TergetCh=pVIC1->NextTergetCh;
				pVIC1->NextTergetCh=vic1_ch_NULL;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_WHAT;
			}//何の予定もない場合
			else {
				//モードをIdleにし、黙る
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NULL;
			}
			break;
			//相手がStart要求を重ねてきた場合
		case vic1_cmd_START:
			//相手より自分の方が大きなchのとき
			if(pVIC1->RecvCh < pVIC1->Ch) {
				//Ackを返し、自身をRecvにする
				pVIC1->Mode=vic1_mode_RECV;
				pVIC1->Fp_timeout_disable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			}
			//小さい場合は無視
			break;
			//それ以外のことを言っている場合
		default:
			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			break;
		}
	}//DATAコマンドに対する返答
	else if(pVIC1->Mode==vic1_mode_SEND && pVIC1->ActiveCmd==vic1_cmd_DATA) {
		switch(pVIC1->Recv) {
			//相手が受信に成功していた場合
		case vic1_cmd_ACK:
			//comから送信データを取得可能な場合（DATAに継続性がある場合に限る）
			if(vcom1_can_getc(pVIC1->pVCom) && vcom1_flowing(pVIC1->pVCom)) {
				//送信中だった場合
				if(pVIC1->Send!=vic1_cmd_NULL) {
					//ESCを一旦送信
					pVIC1->Mode=vic1_mode_RESEND;
					pVIC1->SendCnt=0;
					pVIC1->Send=vic1_cmd_ESC;
				}//即送信可能な場合
				else {
					//DATAを送信
					pVIC1->Mode=vic1_mode_SEND;
					pVIC1->SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
					pVIC1->Send=vic1_cmd_DATA;
				}
			}//comから送信データを取得できない場合
			else {
				//Stop送信
				pVIC1->Mode=vic1_mode_STOP;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_STOP;
			}
			break;
			//相手が受信に失敗していた場合
		case vic1_cmd_NACK:
			//バッファの読み出し位置を初期化
			_vic1_buf_cancel_get(pVIC1);
			//送信中だった場合
			if(pVIC1->Send!=vic1_cmd_NULL) {
				//ESCを一旦送信
				pVIC1->Mode=vic1_mode_RESEND;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ESC;
			}//即送信可能な場合
			else {
				//DATAを送信
				pVIC1->Mode=vic1_mode_SEND;
				pVIC1->SendCnt=0xFF;		//Data送信の場合のみ、SendCntをffにセットする必要がある
				pVIC1->Send=vic1_cmd_DATA;
			}
			break;
		default:
			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
	}//Stopコマンドに対する返答
	else if(pVIC1->Mode==vic1_mode_STOP && pVIC1->ActiveCmd==vic1_cmd_STOP) {
		switch(pVIC1->Recv) {
		//相手が受諾した場合
		case vic1_cmd_ACK:
			//次に話しかける相手が登録されていた場合
			if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
				//WHATコマンドを返送し、自分はIDLEになる
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->TergetCh=pVIC1->NextTergetCh;
				pVIC1->NextTergetCh=vic1_ch_NULL;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_WHAT;
			}//何の予定もない場合
			else {
				//モードをIdleにし、黙る
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NULL;
			}
			break;
		//相手が拒否してきた場合
		case vic1_cmd_NACK:
			//自身は受信モードにし、相手に受諾の意思を伝える
			pVIC1->Mode=vic1_mode_RECV;
			_vic1_buf_clear(pVIC1);	//バッファをクリアして受信に備える
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		default:
			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
	} else {
		//おかしなことを言っているので、教えてあげる
		pVIC1->Mode=vic1_mode_IDLE;
		pVIC1->Fp_timeout_disable();
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_ERR;
	}
}
//データの有無を確認して、IDLEからSTARTへ遷移する関数
hmLib_boolian _vic1_checkData(vic1* pVIC1) {
	//IDLEである、送信中コマンドがない、送信したいデータがある、の３条件が満たされた場合のみ発動
	if(pVIC1->Mode==vic1_mode_IDLE && pVIC1->Send==vic1_cmd_NULL && vcom1_can_getc(pVIC1->pVCom)) {
		//ch取得し、startモードへ移行
		pVIC1->TergetCh=vcom1_get_ch(pVIC1->pVCom);
		pVIC1->Mode=vic1_mode_START;
		pVIC1->Fp_timeout_restart();
		pVIC1->Fp_timeout_enable();	//START,REDATA,DATA,STOPは再送機能付き
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_START;
		return hmLib_true;
	}
	return hmLib_false;
}
//timeoutもモードを一部変えているでござる
//vic1のtimeoutコマンド再送関数
void vic1_timeout(vic1* pVIC1) {
	if(pVIC1->Mode==vic1_mode_START) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			vcom1_skip_get(pVIC1->pVCom);
			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			return;
		}
		pVIC1->Send=vic1_cmd_START;
		pVIC1->SendCnt=0;
	} else if(pVIC1->Mode==vic1_mode_SEND) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			vcom1_skip_get(pVIC1->pVCom);
			//ESCを一旦送信
			pVIC1->Mode=vic1_mode_RESEND;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ESC;
			return;
		}
		//ESCを一旦送信
		pVIC1->Mode=vic1_mode_RESEND;
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_ESC;
	} else if(pVIC1->Mode==vic1_mode_STOP) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			//おかしなことを言っているので、教えてあげる
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			return;
		}		pVIC1->Send=vic1_cmd_STOP;
		pVIC1->SendCnt=0;
	}
	++(pVIC1->TimeOutCnt);
}

//========== vic1 基本関数 ==========
//vic1を初期化する 引数は、自分のチャンネル番号(0x00-0x0F)、相手のチャンネル番号、与える送受信用固定長バッファアドレス
void vic1_initialize(vic1* pVIC1, vcom1* pVCom1, const unsigned char Ch, unsigned char* BufBegin, unsigned char* BufEnd, hmLib_vFp_v FpTimeoutRestart, hmLib_vFp_v FpTimeoutEnable, hmLib_vFp_v FpTimeoutDisable) {
	pVIC1->pVCom=pVCom1;

	_vic1_buf_initialize(pVIC1, BufBegin,BufEnd);

	pVIC1->Fp_timeout_disable=FpTimeoutDisable;
	pVIC1->Fp_timeout_enable=FpTimeoutEnable;
	pVIC1->Fp_timeout_restart=FpTimeoutRestart;

	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Ch=Ch;
	pVIC1->Err=vic1_error_NULL;
	pVIC1->Fail=0x00;
	pVIC1->TimeOutCnt=0;

	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->NextTergetCh=vic1_ch_NULL;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->Send=vic1_cmd_NULL;
	pVIC1->SendCnt=0;

	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCh=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
//	maintask_start(vic1_task, 0, 0, 0);
}
//vic1を終端化する
void vic1_finalize(vic1* pVIC1){
	if(pVIC1->Fp_timeout_disable)pVIC1->Fp_timeout_disable();

	pVIC1->pVCom=0;

	pVIC1->Ch=vic1_ch_NULL;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->NextTergetCh=vic1_ch_NULL;

	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->Err=0;

	_vic1_buf_finalize(pVIC1);

	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Send=vic1_cmd_NULL;
	pVIC1->SendCnt=0;
	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
	pVIC1->RecvCh=vic1_ch_NULL;

//	maintask_stop(vic1_task);
}


//========== gate interface =================
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian  vic1_can_putc(vic1* pVIC1){
	//受信データは常に受け入れ可能（読み捨てるかどうかは別）
	return hmLib_true;
}
//受信データ1byte目判別関数
void x_vic1_putc_1byte(vic1* pVIC1, unsigned char c) {
	//自分宛のChか？
	if(pVIC1->Ch==(c&0x0F)) {
		pVIC1->RecvCh=(c>>4);
		++(pVIC1->RecvCnt);
	}//上記以外なら、少なくとも自分にとってコマンドではない
	else {
		//受信中ならデータ扱いをしておく
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c);
		}
	}
}
//受信データ2byte目判別関数
void x_vic1_putc_2byte(vic1* pVIC1, unsigned char c1, unsigned char c2) {
	//今通信中の相手の場合
	if(pVIC1->RecvCh==pVIC1->TergetCh) {
		//とにかくコメントは聞いておく
		pVIC1->Recv=c2;
		++(pVIC1->RecvCnt);
	}//知らない人からStartコマンドが来た場合
	else if(c2==vic1_cmd_START) {
		//これも聞いておく
		pVIC1->Recv=c2;
		++(pVIC1->RecvCnt);
	}//上記以外なら、少なくとも自分にとってコマンドではない
	else {
		//カウンタを戻す
		pVIC1->RecvCnt=0;

		//受信中ならデータ扱いをしておく
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}
		x_vic1_putc_1byte(pVIC1, c2);
	}
}
//受信データ3byte目判別関数
void x_vic1_putc_3byte(vic1* pVIC1, unsigned char c1, unsigned char c2, unsigned char c3) {
	//Trmn1
	if(c3==0x0d) {
		++(pVIC1->RecvCnt);
	}//上記以外なら、少なくとも1byte目の文字は自分にとってコマンドではない
	else {
		//カウンタを戻す
		pVIC1->RecvCnt=0;

		//受信中ならデータ扱いをしておく
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}
		x_vic1_putc_1byte(pVIC1, c2);

		if(pVIC1->RecvCnt==0) {
			x_vic1_putc_1byte(pVIC1, c3);
		} else if(pVIC1->RecvCnt==1) {
			x_vic1_putc_2byte(pVIC1, c2, c3);
		}
	}
}
//受信データ3byte目判別関数
void x_vic1_putc_4byte(vic1* pVIC1, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4) {
	//Trmn1
	if(c4==0x0a) {
		//コマンド受信完了、発報
		_vic1_inform_recv(pVIC1);
	}//上記以外なら、少なくとも1byte目の文字は自分にとってコマンドではない
	else {
		//カウンタを戻す
		pVIC1->RecvCnt=0;

		//受信中ならデータ扱いをしておく
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}

		x_vic1_putc_1byte(pVIC1, c2);

		if(pVIC1->RecvCnt==0) {
			x_vic1_putc_1byte(pVIC1, c3);

			if(pVIC1->RecvCnt==0) {
				x_vic1_putc_1byte(pVIC1, c4);
			} else if(pVIC1->RecvCnt==1) {
				x_vic1_putc_2byte(pVIC1, c3, c4);
			}

		} else if(pVIC1->RecvCnt==1) {
			x_vic1_putc_2byte(pVIC1, c2, c3);

			if(pVIC1->RecvCnt==0) {
				x_vic1_putc_1byte(pVIC1, c4);
			} else if(pVIC1->RecvCnt==1) {
				x_vic1_putc_2byte(pVIC1, c3,c4);
			} else if(pVIC1->RecvCnt==2) {
				x_vic1_putc_3byte(pVIC1, c2,c3,c4);
			}
		}

	}
}
//受信データを投げ入れる
void vic1_putc(vic1* pVIC1, unsigned char c){
	if(pVIC1->RecvCnt==0) {
		x_vic1_putc_1byte(pVIC1, c);
	} else if(pVIC1->RecvCnt==1) {
		x_vic1_putc_2byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), c);
	} else if(pVIC1->RecvCnt==2) {
		x_vic1_putc_3byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), pVIC1->Recv, c);
	} else {
		x_vic1_putc_4byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), pVIC1->Recv, 0x0d, c);
	}
}
//受信データをフラッシュする（gate互換用、VIC内での処理はない）
void vic1_flush(vic1* pVIC1) {}
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian  vic1_can_getc(vic1* pVIC1){
	//送信モードに遷移できるとの通告があれば、遷移
	if(_vic1_checkData(pVIC1))return hmLib_true;

	//送信モードで、送信データが全て送信完了していない場合
	if(pVIC1->Send==vic1_cmd_DATA && pVIC1->SendCnt==0xFF)return _vic1_buf_can_getc(pVIC1);

	//コマンド送信中の場合
	if(pVIC1->Send!=vic1_cmd_NULL)return hmLib_true;

	return hmLib_false;
}
//送信データを呼び出す
unsigned char vic1_getc(vic1* pVIC1) {
	unsigned char c=0x00;

	//送信できない場合はエラー
	if(!vic1_can_getc(pVIC1)) {
		pVIC1->Err=vic1_error_INVALID_GETC;
		return 0xC0;
	}

	//データ送信モードで、かつデータを送り終えていない場合
	if(pVIC1->Send==vic1_cmd_DATA && pVIC1->SendCnt==0xFF) {
		//データ送信中の場合は、データを送信処理
		c=_vic1_buf_getc(pVIC1);

		//送信すべきデータを送り終えた場合は、SendCntを戻す
		if(_vic1_buf_flowing(pVIC1)==0) {
			//SendCntを正しい値に戻す
			pVIC1->SendCnt=0;
		}
	}else{
		//SendCntが壊れていたら、エラー発報
		if(pVIC1->SendCnt>3) {
			pVIC1->Err=vic1_error_INCORRECT_SENDCNT;
			pVIC1->Send=vic1_cmd_ERR;
			pVIC1->SendCnt=0;
		}

		//コマンド送信中の場合は、コマンドを読む
		if(pVIC1->SendCnt==0) {
			c=((pVIC1->Ch<<4) | pVIC1->TergetCh);
			//送信カウンタを進める
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==1) {
			switch(pVIC1->Send) {
			case vic1_cmd_ACK:
				c=vic1_cmd_ACK;
				break;
			case vic1_cmd_NACK:
				c=vic1_cmd_NACK;
				break;
			case vic1_cmd_START:
				c=vic1_cmd_START;
				break;
			case vic1_cmd_STOP:
				c=vic1_cmd_STOP;
				break;
			case vic1_cmd_DATA:
				c=vic1_cmd_DATA;
				break;
			default:
				pVIC1->Send=vic1_cmd_ERR;
				c=vic1_cmd_ERR;
				break;
			}
			//送信カウンタを進める
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==2) {
			c=0x0d;
			//送信カウンタを進める
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==3) {
			_vic1_inform_send(pVIC1);
			c=0x0a;
		}

	}
	return c;
}
//送信データを流れ続けているか確認（VICの場合は、コマンド末尾が0,それ以外は1）
hmLib_boolian vic1_flowing(vic1* pVIC1) { return pVIC1->Send!=vic1_cmd_NULL; }

//============ vic functions ================
//通信を強制的に終了させる
void vic1_force_end(vic1* pVIC1) {
	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->Send=vic1_cmd_ERR;
	pVIC1->SendCnt=0;
	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Fail=0;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->NextTergetCh=vic1_ch_NULL;

	//データ受信をなかったことにする
	vcom1_flush(pVIC1->pVCom);
	//データ送信をなかったことにする
	vcom1_cancel_get(pVIC1->pVCom);
}
//エラーを取得する
unsigned char vic1_error(vic1* pVIC1) { return pVIC1->Err; }
//エラーをクリアする
void vic1_clear_error(vic1* pVIC1) { pVIC1->Err=vic1_error_NULL; }
#
#endif
