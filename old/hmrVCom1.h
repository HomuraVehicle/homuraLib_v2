#ifndef HMR_VCOM1_INC
#define HMR_VCOM1_INC 104
#
#include"hmrVComData.h"
/*---vcom1---
vcom_dataとvcom1形式のデータ文字列とを変換する

=== vcom1 ===
v1_04/140302 hmIto
	vcom1_skip_get関数を追加
v1_03/140118 hmIto
	0byteデータを送信する際に、データの破棄をしていなかったため、何度も同じデータを送ろうとしてしまっていた問題を修正
v1_02/140110 hmIto
	vcom_dataのidにかかわる規約変更に対応
v1_01/131224 hmIto
	デバッグデバッグデバッグ
	ほぼテストしつくしたはず
v1_00/131220 hmIto
	VMCから分離して作成
*/
#ifdef __cplusplus
extern "C"{
#endif

	typedef hmLib_bFp_v vcom1_bFp_v;
	typedef void(*vcom1_vFp_pdata)(vcom_data*);
	typedef hmLib_boolian(*vcom1_bFp_pstr_strsize)(hmLib_cstring *, hmLib_cstring_size_t);
	typedef struct {
		//============== recv ===============
		unsigned char RecvMode;
		vcom_data RecvData;
		vcom_ch_t RecvCh;
		vcom_size_t RecvCnt;
		//============== send ===============
		unsigned char SendMode;
		vcom_data SendData;
		vcom_size_t SendCnt;
		hmLib_boolian SendEof;
		//============== functions ===============
		vcom1_bFp_v _fp_can_move_push;					//受信データをぶちこんでいいか
		vcom1_vFp_pdata _fp_move_push;					//受信データをぶち込む
		vcom1_bFp_v _fp_can_move_pop;					//送信データを抜き出していいか
		vcom1_vFp_pdata _fp_move_pop;					//送信データを抜き出す
		vcom1_bFp_pstr_strsize _fp_cstring_construct;	//cstring確保依頼関数	
	}vcom1;
	//vcom1初期化
	void vcom1_initialize(vcom1* pVCom1,
		vcom1_bFp_v Fp_can_move_push,
		vcom1_vFp_pdata Fp_move_push,
		vcom1_bFp_v Fp_can_move_pop,
		vcom1_vFp_pdata Fp_move_pop,
		vcom1_bFp_pstr_strsize Fp_cstring_construct);
	//vcom1終端化
	void vcom1_finalize(vcom1* pVCom1);
	//======== gate interface =======
	//sendできるか？
	hmLib_boolian vcom1_can_getc(vcom1* pVCom);
	//送信文字列を1byte取得する
	unsigned char vcom1_getc(vcom1* pVCom);
	//送信文字列がeof位置=Pac終端かどうかを検知する
	hmLib_boolian vcom1_flowing(vcom1* pVCom);
	//recvできるか？(単にreturn 1)
	hmLib_boolian vcom1_can_putc(vcom1* pVCom);
	//受信文字列を1byte与える
	void vcom1_putc(vcom1* pVCom, unsigned char c);
	//flushする(eof化する=Pacを閉じる)
	void vcom1_flush(vcom1* pVCom);

	//======= vcom functions ========
	//送信先chを取得する
	unsigned char vcom1_get_ch(vcom1* pVCom);
	//送信をキャンセルする（次回送信は、今のデータ）
	void vcom1_cancel_get(vcom1* pVCom);
	//送信をスキップする（次回送信は、次のデータ）
	void vcom1_skip_get(vcom1* pVCom);
	//受信するchを取得する
	void vcom1_put_ch(vcom1* pVCom, unsigned char Ch);
	//受信をキャンセルする
	void vcom1_cancel_put(vcom1* pVCom);

#ifdef __cplusplus
}
#endif
#
#endif
