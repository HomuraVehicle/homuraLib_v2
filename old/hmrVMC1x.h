#ifndef HMR_VMC1X_INC
#define HMR_VMC1X_INC 102
#
/*===hmrVMC1x===
ほむらの操縦者とMainPicをつなぐ主通信の通信フォーマットを規定するファイル
バッファ操作の窓口関数を用意してやると、
受信データをVMC1x関数群に受け渡すと、バッファへの追加処理を行ってくれる
送信データをVMC1x関数群で要求すると、バッファを読み出して適当な送信文字を返す


=== hmrVMC1x ===
v1_00/140118 hmIto
	促成のvmc1型クラス
=== hmrVMC2 ===
v1_02/131224 hmIto
	vmc2 functionsのエラーにかかわる関数名を変更
v1_01/131220 hmIto
	vcom1準拠の新規通信フォーマット、vmc2をひとまず完成
v1_00/131217 hmIto
	VCom共通インターフェースに対応作業開始
=== hmrVMC1 ===
v4_00/130223 hmIto
	cファイル中のextern Cが不要だったのを削除
	vmc1_createの引数だったVMCIDを、vmc1_initializeに移動
	boolianをhmLib_boolianに変更
	hmrTypeのincludeを削除
	can_iniSendPac()/can_iniRecvPac()を追加
		それぞれ、送信/受信時のパケットを開く可否
v3_00/130211 hmIto
	can_existSendDat関数をvmc_interfaceに追加
		existSendDat関数を実行可能かを問い合わせる関数
	getSendErr/getRecvErr/clearSendErr/clearRecvErr関数を追加
		それぞれ，最後に発生した送受信時のエラーを取得/クリアする
		エラー発生中でも，特に動作に支障はない（強制的にパケットを閉じるなどして，送受信は処理されている）
v2_02/121128 hmIto
	hmCLibとhmLibの統合に伴い、hmLib_v3_03に対応するよう変更
	hmrVMC1自身をC++に対応
v2_01/121027 hmIto
	vmc1_getにおいて，0byteデータをエラー扱いにしていたのを修正
v2_00/121020 hmIto
	複数のVMCの同時運用に対応
		3byteのデバイス固有のVMCIDを新たに指定
		送信元IDと送信先IDをVMC構造体に追加
		受信時には自身のVMCID向けの通信のみ受信
		自身のVMCIDはcreate関数で指定
		送信時には送信先のVMCIDを指定することが必要
		相手のVMCIDはinitialize関数で指定
		以後の規格は、#{VMCID[3]}.....##crlf
		staticなSTRT,TRMNは廃止
	関数名をforce_ini_get/putからforce_end_get/putに変更
	finalize追加
	create内でのinitialize自動実行を廃止
v1_04/121014 hmIto
	force_ini_get/put関数を追加
	timeoutなど、VMC1の受信の如何に関わらず直ちにステータスを初期化するのに使用
v1_03/121013 hmIto
	名前空間VMC1をvmc1に変更
v1_02/121008 hmIto
	各DatのIDについてもVMC1で管理するよう変更　送信・受信時の関数形など変更
	データサイズのバッファサイズを2byteへ拡張
		DatID[1],DatSize[2],Dat[DatSize]にデータのフォーマットを変更
		#PacID[3],Dat...,##crlfにパケットのフォーマットを変更
		121009Test_hmrCom_hmrVMC1.cにて、動作確認済み
v1_01/120922 hmIto
	ひとまず動作が安定していることを確認（120922Test_hmrCom_hmrVMC1.cにて動作確認済み）
v1_00/120921 hmIto
	バージョン管理開始
*/
#ifndef HMLIB_TYPE_INC
#	include<hmLib_v3_06/type.h>
#endif
//#include"hmLib_v3_06/type.h"
#include"hmrVCom1.h"
#ifdef __cplusplus
extern "C"{
#endif
typedef vcom1 vcom;
//--------- err-----------------
#define vmc1x_geterr_NULL			0x00
#define vmc1x_geterr_SENDREJECTED	0x10
#define vmc1x_puterr_NULL			0x00
#define vmc1x_puterr_RECVREJECTED	0x10

//VMC1構造体の宣言
typedef struct {
	//送信主識別ID
	unsigned char VMCID[3];
	//送信先識別ID
	unsigned char TVMCID[3];
	//受信モード
	unsigned char RecvMode;
	//受信モードのカウンタ
	unsigned char RecvCnt;
	//受信データの送信Ch
	unsigned char RecvCh;
	//受信モードのエラー
	unsigned char RecvErr;
	//送信モード
	unsigned char SendMode;
	//送信モードのカウンタ
	unsigned char SendCnt;
	//送信モードのエラー
	unsigned char SendErr;
	//VCom
	vcom* pVCom;
}vmc1x;
//vmc1xを初期化する
void vmc1x_initialize(vmc1x* pVMC1x, vcom* pVCom, const unsigned char VMCID_[3], const unsigned char TVMCID_[3]);
//vmc1xを終端化する
void vmc1x_finalize(vmc1x* pVMC1x);
//========= gate interface =================
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian  vmc1x_can_putc(vmc1x* pVMC1x);
//受信データを投げ入れる
void vmc1x_putc(vmc1x* pVMC1x, unsigned char c_);
//受信データをeofする
void vmc1x_flush(vmc1x* pVMC1x);
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian  vmc1x_can_getc(vmc1x* pVMC1x);
//送信データを呼び出す
unsigned char vmc1x_getc(vmc1x* pVMC1x);
//送信データがeof位置でないかを確認する
hmLib_boolian vmc1x_flowing(vmc1x* pVMC1x);
//========= vmc functions ==================
//受信を強制的に終了させる
void vmc1x_force_end_put(vmc1x* pVMC1x);
//受信エラーを取得する
unsigned char vmc1x_error_of_put(vmc1x* pVMC1x);
//受信エラーをクリアする
void vmc1x_clear_error_of_put(vmc1x* pVMC1x);
//送信を強制的に終了させる
void vmc1x_force_end_get(vmc1x* pVMC1x);
//送信エラーを取得する
unsigned char vmc1x_error_of_get(vmc1x* pVMC1x);
//送信エラーをクリアする
void vmc1x_clear_error_of_get(vmc1x* pVMC1x);

#ifdef __cplusplus
}	//extern "C"{
#endif
#
#endif
