#ifndef HMR_VIC1_INC
#define HMR_VIC1_INC 102
#
/*
===hmrVIC1===
Homura Interactive Communication
	一定バイト長ごとにチェックサムによるデータ抜け検証＆再送を行う通信形式
	チェックサムの確認のためにやり取りを何度も行う形式のため、送受信の切り替えが遅い場合は通信が遅くなる。

--- command ---
・送信側
Start	送信側としてえ回ることを通知
Stop	送信側を終えることを通知
Data	データを送信
Esc		ここまでの送信内容を一度キャンセルすることを通知
What	受信側となれることを通知
Err		エラーが生じたため送受信関係を終了することを通知
・受信側
Ack
Nack

--- protocol ---
送信フォーマット

・データ送信時
Data[1-Max], CheckSum, CH/TCH, CMD=Data, _cr, _lf

・その他のコマンド送信時
CH/TCH,CMD,_cr,_lf
CH/TCH:CH=4bit+TCH4bit


=== hmrVIC1 ===
v1_02/140302 hmIto
	タイムアウト回数に上限を設け、オーバーした場合は相応の措置をとるように修正
		例えば、存在しない相手宛のStartを無限に繰り返してしまう問題を解決
v1_01/140115 hmIto
	チェックサムにcrc8を採用
v1_00/131224 hmIto
	リリース
*/

#include<hmLib/type.h>
#include"hmrVCom1.h"
#ifdef __cplusplus
extern "C"{
#endif

//---------------- error ----------------
#define vic1_error_NULL					0x00
#define vic1_error_SAVE_REJECTED		0x10
#define vic1_error_INCORRECT_SENDCNT	0x20
#define vic1_error_OVERFLOW				0x30
#define vic1_error_OVERREAD				0x40
#define vic1_error_INVALID_GETC			0x50

//vic1構造体の宣言
typedef struct{
	//送信主識別CH (0x00-0x08)
	unsigned char Ch;
	//送信先識別CH (0x00-0x08)
	unsigned char TergetCh;
	//予約済み送信先識別CH (0x00-0x08)
	unsigned char NextTergetCh;
	//モード
	unsigned char Mode;
	//現在返信待ち中の送信コマンド
	unsigned char ActiveCmd;
	//送信中コマンド
	unsigned char Send;
	//送信カウンタ
	unsigned char SendCnt;
	//受信中コマンド
	unsigned char Recv;
	//受信カウンタ
	unsigned char RecvCnt;
	//受信中のCh
	unsigned char RecvCh;

	//送受信時発生エラーログ
	unsigned char Err;
	//データ送受信が失敗していないか
	unsigned char Fail;
	//タイムアウト回数カウンター
	unsigned char TimeOutCnt;

	//送受信共有バッファ
	unsigned char* _buf_Begin;
	unsigned char* _buf_End;
	//送受信共有バッファのイテレータ
	unsigned char* _buf_GateItr;
	//VCom側のイテレータ
	unsigned char* _buf_VComItr;

	//timeout制御用関数群
	hmLib_vFp_v Fp_timeout_restart;
	hmLib_vFp_v Fp_timeout_enable;
	hmLib_vFp_v Fp_timeout_disable;

	//送受信用インターフェース
	vcom1* pVCom;
}vic1;
//vic1を初期化する
//	BufEnd - BufBegin - 1 が最大データサイズ長となる
//	最大データサイズ長は通信するすべてのデバイス間で同一である必要がある
//	timeout関数の呼び出しタイミング通告用関数群を引き渡す
void vic1_initialize(vic1* pVIC1, vcom1* pVCom1, const unsigned char Ch, unsigned char* BufBegin, unsigned char* BufEnd
	,hmLib_vFp_v FpTimeoutRestart, hmLib_vFp_v FpTimeoutEnable, hmLib_vFp_v FpTimeoutDisable);
//vic1を終端化する
void vic1_finalize(vic1* pVIC1);
//vic1のtimeout処理用毎秒タスク
void vic1_timeout(vic1* pVIC1);
//========== gate interface =================
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian  vic1_can_putc(vic1* pVIC1);
//受信データを投げ入れる
void vic1_putc(vic1* pVIC1, unsigned char c_);
//受信データをフラッシュする（gate互換用、VIC内での処理はない）
void vic1_flush(vic1* pVIC1);
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian  vic1_can_getc(vic1* pVIC1);
//送信データを呼び出す
unsigned char vic1_getc(vic1* pVIC1);
//送信データを流れ続けているか確認（VICの場合は、データシーケンス末尾が0、それ以外は1）
hmLib_boolian vic1_flowing(vic1* pVIC1);
//============ vic functions ================
//通信を強制的に終了させる
void vic1_force_end(vic1* pVIC1);
//エラーを取得する
unsigned char vic1_error(vic1* pVIC1);
//エラーをクリアする
void vic1_clear_error(vic1* pVIC1);
#ifdef __cplusplus
}	//extern "C"{
#endif
#
#endif
