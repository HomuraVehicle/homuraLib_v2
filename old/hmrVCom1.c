#ifndef HMR_VCOM1_C_INC
#define HMR_VCOM1_C_INC 104
#
#ifndef HMR_VCOM1_INC
#	include"hmrVCom1.h"
#endif
#define vcom1_recvmode_ID		0
#define vcom1_recvmode_SIZE1	1
#define vcom1_recvmode_SIZE2	2
#define vcom1_recvmode_DATA		3
#define vcom1_recvmode_IGNORE	4
#define vcom1_recvmode_ERROR	5

#define vcom1_sendmode_ID		0
#define vcom1_sendmode_SIZE1	1
#define vcom1_sendmode_SIZE2	2
#define vcom1_sendmode_DATA		3

//vcom1初期化
void vcom1_initialize(vcom1* pVCom1,
	vcom1_bFp_v Fp_can_move_push,
	vcom1_vFp_pdata Fp_move_push,
	vcom1_bFp_v Fp_can_move_pop,
	vcom1_vFp_pdata Fp_move_pop,
	vcom1_bFp_pstr_strsize Fp_cstring_construct) {
	pVCom1->RecvCh=0x00;
	pVCom1->RecvCnt=0;
	vcom_data_format(&(pVCom1->RecvData));
	pVCom1->RecvMode=vcom1_recvmode_ID;

	pVCom1->SendCnt=0;
	vcom_data_format(&(pVCom1->SendData));
	pVCom1->SendEof=1;
	pVCom1->SendMode=vcom1_sendmode_ID;

	pVCom1->_fp_can_move_push=Fp_can_move_push;
	pVCom1->_fp_move_push=Fp_move_push;
	pVCom1->_fp_can_move_pop=Fp_can_move_pop;
	pVCom1->_fp_move_pop=Fp_move_pop;
	pVCom1->_fp_cstring_construct=Fp_cstring_construct;
}
//vcom1終端化
void vcom1_finalize(vcom1* pVCom1) {
	pVCom1->RecvCh=0x00;
	pVCom1->RecvCnt=0;
	vcom_data_destruct(&(pVCom1->RecvData));
	pVCom1->RecvMode=vcom1_recvmode_ID;

	pVCom1->SendCnt=0;
	vcom_data_destruct(&(pVCom1->SendData));
	pVCom1->SendEof=1;
	pVCom1->SendMode=vcom1_sendmode_ID;

	pVCom1->_fp_can_move_push=0;
	pVCom1->_fp_move_push=0;
	pVCom1->_fp_can_move_pop=0;
	pVCom1->_fp_move_pop=0;
	pVCom1->_fp_cstring_construct=0;
}

//======== gate interface =======
//sendできるか？
hmLib_boolian vcom1_can_getc(vcom1* pVCom) {
	//Dataが空でないなら、終了
	if(vcom_data_is_construct(&(pVCom->SendData))) {
		if(pVCom->SendMode==vcom1_sendmode_DATA && pVCom->SendData.Accessible!=0)return pVCom->SendData.Accessible() > pVCom->SendCnt;
		else return 1;
	}
	//Dataを読み出せないなら終了
	if(pVCom->_fp_can_move_pop()==0)return 0;

	//Data読み出し
	pVCom->_fp_move_pop(&(pVCom->SendData));
	pVCom->SendCnt=0;

	//空データならはじく
	if(!vcom_data_is_construct(&(pVCom->SendData)))return 0;

	//eofなら、フラグを立てて次を見に行く
	if(vcom_data_eof(&(pVCom->SendData))) {
		vcom_data_destruct(&pVCom->SendData);
		pVCom->SendEof=1;
		return vcom1_can_getc(pVCom);
	}

	//Data読み出しに成功すれば、OK
	return 1;
}
//送信文字列を1byte取得する
unsigned char vcom1_getc(vcom1* pVCom) {
	unsigned char c;

	//EOFフラグを外す
	pVCom->SendEof=0;

	//Dataが空なら、終了
	if(vcom1_can_getc(pVCom)==0) {
		pVCom->SendMode=vcom1_sendmode_ID;
		return 0xC0;
	}

	//ID送信モード
	switch(pVCom->SendMode) {
	case vcom1_sendmode_ID:
		pVCom->SendMode=vcom1_sendmode_SIZE1;
		return pVCom->SendData.ID;
	case vcom1_sendmode_SIZE1:
		pVCom->SendMode=vcom1_sendmode_SIZE2;
		return (unsigned char)(cstring_size(&(pVCom->SendData.Data)));
	case vcom1_sendmode_SIZE2:
		if(cstring_size(&(pVCom->SendData.Data))==0) {
			pVCom->SendMode=vcom1_sendmode_ID;
			c=(unsigned char)(cstring_size(&(pVCom->SendData.Data))>>8);
			vcom_data_destruct(&(pVCom->SendData));
			pVCom->SendMode=vcom1_sendmode_ID;
		} else {
			pVCom->SendMode=vcom1_sendmode_DATA;
			c=(unsigned char)(cstring_size(&(pVCom->SendData.Data))>>8);
		}
		pVCom->SendCnt=0;
		return c;
	case vcom1_sendmode_DATA:
		c=cstring_getc(&(pVCom->SendData.Data), (pVCom->SendCnt)++);
		if(pVCom->SendCnt==cstring_size(&(pVCom->SendData.Data))) {
			vcom_data_destruct(&(pVCom->SendData));
			pVCom->SendMode=vcom1_sendmode_ID;
		}
		return c;
	default:
		pVCom->SendMode=vcom1_sendmode_ID;
		return 0;
	}
}
//送信文字列がeof位置=Pac終端かどうかを検知する
hmLib_boolian vcom1_flowing(vcom1* pVCom) {
	vcom1_can_getc(pVCom);
	return pVCom->SendEof==0;
}
//recvできるか？(単にreturn 1)
hmLib_boolian vcom1_can_putc(vcom1* pVCom) {
	//エラー状態の時には、大歓迎
	if(pVCom->RecvMode==vcom1_recvmode_ERROR)return 1;

	//バッファ側が受信できるならOK
	return pVCom->_fp_can_move_push();
}
//受信文字列を1byte与える
void vcom1_putc(vcom1* pVCom, unsigned char c) {
	//データid受信時
	if(pVCom->RecvMode==vcom1_recvmode_ID) {
		//NullIDはつかえないので無視
		if(c==vcom_data_id_NULL) {
			//以後のデータはflushがあるまで信じられないのでモードを移行
			pVCom->RecvMode=vcom1_recvmode_ERROR;
			return;
		}

		//データIDを取得
		pVCom->RecvData.ID=c;
		pVCom->RecvData.Ch=pVCom->RecvCh;
		pVCom->RecvData.Err=vcom_data_error_NULL;
		pVCom->RecvData.Accessible=0;

		//IDに関するエラー登録
		if(!vcom_data_id_is_valid(c))pVCom->RecvData.Err|=vcom_data_error_UNKNOWN;

		//Chに関するエラー登録
		if(pVCom->RecvCh>7)pVCom->RecvData.Err|=vcom_data_error_STRANGER;

		//モードをサイズ受信モード(SIZE1)へ移行
		pVCom->RecvMode=vcom1_recvmode_SIZE1;
	}//データサイズ受信待1
	else if(pVCom->RecvMode==vcom1_recvmode_SIZE1) {
		//データサイズを取得
		pVCom->RecvCnt=(vcom_size_t)(c);

		//モードをサイズ受信モード(SIZE2)へ移行
		pVCom->RecvMode=vcom1_recvmode_SIZE2;
	}//データサイズ受信待2
	else if(pVCom->RecvMode==vcom1_recvmode_SIZE2) {
		//データサイズを取得
		pVCom->RecvCnt|=((vcom_size_t)(c)<<8);

		//データサイズが規格上限を上回っていた場合は無視
		if(pVCom->RecvCnt>vcom_data_MAXSIZE) {
			//データを送る
			if(pVCom->_fp_can_move_push()) {
				//エラー付きで送付
				pVCom->RecvData.Err|=vcom_data_error_SIZEOVER;

				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}

			//以後のデータはflushがあるまで信じられないのでモードを移行
			pVCom->RecvMode=vcom1_recvmode_ERROR;
			return;
		}

		//データサイズが0の時
		if(pVCom->RecvCnt==0) {
			//データを送る
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}

			//モードをID受信モードへ移行
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}

		//バッファを確保
		pVCom->_fp_cstring_construct(&(pVCom->RecvData.Data), pVCom->RecvCnt);

		//データサイズ確保に失敗したとき
		if(cstring_size(&(pVCom->RecvData.Data)) != pVCom->RecvCnt) {
			//確保失敗エラー登録
			pVCom->RecvData.Err|=vcom_data_error_FAILNEW;
			//以後、書き込みできないので、次のデータがあるまで無視
			pVCom->RecvMode=vcom1_recvmode_IGNORE;
			return;
		}

		//モードをPac受信モードへ移行
		pVCom->RecvCnt=0;
		pVCom->RecvMode=vcom1_recvmode_DATA;
	}//データ受信中
	else if(pVCom->RecvMode==vcom1_recvmode_DATA) {
		//データ書き込み
		cstring_putc(&(pVCom->RecvData.Data), (pVCom->RecvCnt)++, c);

		//書き込みサイズ上限に達した場合
		if(pVCom->RecvCnt==cstring_size(&(pVCom->RecvData.Data))) {
			//データを送る
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}//送付できない場合は、破棄する
			else {
				vcom_data_destruct(&(pVCom->RecvData));
			}

			//モードをID受信モードへ移行
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}
	}//データ無視中
	else if(pVCom->RecvMode==vcom1_recvmode_IGNORE) {
		--(pVCom->RecvCnt);

		//全データ受信し終えたとき
		if(pVCom->RecvCnt==0) {
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}//送付できない場合は、破棄する
			else {
				vcom_data_destruct(&(pVCom->RecvData));
			}

			//モードをID受信に移行
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}
	}
}
//flushする(eof化する=Pacを閉じる)
void vcom1_flush(vcom1* pVCom) {
	//受信中のデータが存在する場合
	if(pVCom->RecvMode==vcom1_recvmode_DATA || pVCom->RecvMode==vcom1_recvmode_IGNORE) {
		//未受信完了エラー登録
		pVCom->RecvData.Err|=vcom_data_error_UNDERFLOW;

		//データ送付
		if(pVCom->_fp_can_move_push()) {
			pVCom->_fp_move_push(&(pVCom->RecvData));
			vcom_data_format(&(pVCom->RecvData));
		}//送付できない場合は、破棄する
		else {
			vcom_data_destruct(&(pVCom->RecvData));
		}
	}

	//データ末尾送付(注意：失敗すると、PacTrmnが登録されない)
	if(pVCom->_fp_can_move_push()) {
		vcom_data_set_eof(&(pVCom->RecvData));

		pVCom->_fp_move_push(&(pVCom->RecvData));
		vcom_data_format(&(pVCom->RecvData));
	}

	//モードをID受信に移行
	pVCom->RecvMode=vcom1_recvmode_ID;
}

//======= vcom functions ========
//送信先chを取得する
unsigned char vcom1_get_ch(vcom1* pVCom) {
	if(vcom1_can_getc(pVCom)==0)return 0xDD;

	return pVCom->SendData.Ch;
}
//送信をキャンセルする
void vcom1_cancel_get(vcom1* pVCom) {
	pVCom->SendMode=vcom1_sendmode_ID;
	pVCom->SendCnt=0;
}
//送信をスキップする
void vcom1_skip_get(vcom1* pVCom) {
	pVCom->SendMode=vcom1_sendmode_ID;
	pVCom->SendCnt=0;
	vcom_data_destruct(&pVCom->SendData);
}
//受信するchを取得する
void vcom1_put_ch(vcom1* pVCom, unsigned char Ch) {
	pVCom->RecvCh=Ch;
}
//受信をキャンセルする
void vcom1_cancel_put(vcom1* pVCom) {
	pVCom->RecvMode=vcom1_recvmode_ID;
	pVCom->RecvCnt=0;
	vcom_data_destruct(&pVCom->RecvData);
}
#
#endif
