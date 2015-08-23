#ifndef HMR_VMC1_INC
#define HMR_VMC1_INC 400
#
/*===hmrVMC1===
	ほむらの操縦者とMainPicをつなぐ主通信の通信フォーマットを規定するファイル
	バッファ操作の窓口関数を用意してやると、
		受信データをVMC1関数群に受け渡すと、バッファへの追加処理を行ってくれる
		送信データをVMC1関数群で要求すると、バッファを読み出して適当な送信文字を返す

hmrVMC1_v4_00/130223 hmIto
	cファイル中のextern Cが不要だったのを削除
	vmc1_createの引数だったVMCIDを、vmc1_initializeに移動
	boolianをhmLib_boolianに変更
	hmrTypeのincludeを削除
	can_iniSendPac()/can_iniRecvPac()を追加
		それぞれ、送信/受信時のパケットを開く可否
hmrVMC1_v3_00/130211 hmIto
	can_existSendDat関数をvmc_interfaceに追加
		existSendDat関数を実行可能かを問い合わせる関数
	getSendErr/getRecvErr/clearSendErr/clearRecvErr関数を追加
		それぞれ，最後に発生した送受信時のエラーを取得/クリアする
		エラー発生中でも，特に動作に支障はない（強制的にパケットを閉じるなどして，送受信は処理されている）
hmrVMC1_v2_02/121128 hmIto
	hmCLibとhmLibの統合に伴い、hmLib_v3_04に対応するよう変更
	hmrVMC1自身をC++に対応
hmrVMC1_v2_01/121027 hmIto
	vmc1_sendにおいて，0byteデータをエラー扱いにしていたのを修正
hmrVMC1_v2_00/121020 hmIto
	複数のVMCの同時運用に対応
		3byteのデバイス固有のVMCIDを新たに指定
			送信元IDと送信先IDをVMC構造体に追加
		受信時には自身のVMCID向けの通信のみ受信
			自身のVMCIDはcreate関数で指定
		送信時には送信先のVMCIDを指定することが必要
			相手のVMCIDはinitialize関数で指定
		以後の規格は、#{VMCID[3]}.....##crlf
			staticなSTRT,TRMNは廃止
	関数名をforce_ini_send/recvからforce_end_send/recvに変更
	finalize追加
	create内でのinitialize自動実行を廃止
hmrVMC1_v1_04/121014 hmIto
	force_ini_send/recv関数を追加
		timeoutなど、VMC1の受信の如何に関わらず直ちにステータスを初期化するのに使用
hmrVMC1_v1_03/121013 hmIto
	名前空間VMC1をvmc1に変更
hmrVMC1_v1_02/121008 hmIto
	各DatのIDについてもVMC1で管理するよう変更　送信・受信時の関数形など変更
	データサイズのバッファサイズを2byteへ拡張
	DatID[1],DatSize[2],Dat[DatSize]にデータのフォーマットを変更
	#PacID[3],Dat...,##crlfにパケットのフォーマットを変更
	121009Test_hmrCom_hmrVMC1.cにて、動作確認済み
hmrVMC1_v1_01/120922 hmIto
	ひとまず動作が安定していることを確認（120922Test_hmrCom_hmrVMC1.cにて動作確認済み）
hmrVMC1_v1_00/120921 hmIto
	バージョン管理開始
*/
#ifndef HMLIB_TYPE_INC
#	include"hmLib_v3_05/bytebase_type.h"
#endif
#ifdef __cplusplus
extern "C"{
#endif
#define vmc1_RECVERR_fail_to_iniRecvDat 1
#define vmc1_RECVERR_fail_to_recv 2
#define vmc1_SENDERR_fail_to_existSendDat 1
#define vmc1_SENDERR_fail_to_iniSendDat 2
#define vmc1_SENDERR_fail_to_send 3

typedef hmLib_uint16 vmc1_dsize_t;
typedef hmLib_uint8 vmc1_did_t;
//typedef hmLib_boolian hmLib_boolian;
typedef void(*vmc1_vFp_v)(void);
typedef void(*vmc1_vFp_uc)(unsigned char);
typedef void(*vmc1_vFp_did_dsize)(vmc1_did_t,vmc1_dsize_t);
typedef unsigned char(*vmc1_ucFp_v)(void);
typedef hmLib_boolian(*vmc1_bFp_v)(void);
typedef void(*vmc1_vFp_pdid_pdsize)(vmc1_did_t*,vmc1_dsize_t*);
//VMC1構造体の宣言
typedef struct{
	//送信主識別ID
	unsigned char VMCID[3];
	//送信先識別ID
	unsigned char TVMCID[3];
	//受信モード
	unsigned char RecvMode;
	//受信モードのカウンタ
	unsigned char RecvModeCnt;
	//受信データのカウンタ
	vmc1_dsize_t RecvDatCnt;
	//受信モードのエラー
	unsigned char RecvErr;
	//受信データのサイズ
	vmc1_dsize_t RecvDatSize;
	//受信データのid
	vmc1_did_t RecvDatID;
	//送信モード
	unsigned int SendMode;
	//送信モードのカウンタ
	vmc1_dsize_t SendCnt;
	//送信モードのエラー
	unsigned char SendErr;
	//送信データのサイズ
	vmc1_dsize_t SendDatSize;
	//受信データのid
	vmc1_did_t SendDatID;
/************************制御関数**************************/
	//iniRecvPacを実行して良いかの確認に呼ばれる
	vmc1_bFp_v fp_can_iniRecvPac;
	//PacStrt受信完了時に呼ばれる
	vmc1_vFp_v fp_iniRecvPac;
	//PacTrmn受信完了時に呼ばれる　引数はエラーの有無
	vmc1_vFp_uc fp_finRecvPac;
	//iniRecvDatを実行して良いかの確認に呼ばれる
	vmc1_bFp_v fp_can_iniRecvDat;
	//Dat受信開始時に呼ばれる 引数は受信するデータサイズ 実行は，必ずcan_iniRecvDatを確認後
	vmc1_vFp_did_dsize fp_iniRecvDat;
	//Dat受信終了時に呼ばれる 引数はエラーの有無
	vmc1_vFp_uc fp_finRecvDat;
	//recvを実行してよいかの確認に呼ばれる
	vmc1_bFp_v fp_can_recv;
	//Datの中身受信時に呼ばれる　引数が受信したデータ 実行は，必ずfp_can_recvを確認後
	vmc1_vFp_uc fp_recv;
	//iniSendPacを実行して良いかの確認に呼ばれる
	vmc1_bFp_v fp_can_iniSendPac;
	//RacStrt送信完了時に呼ばれる
	vmc1_vFp_v fp_iniSendPac;
	//PacTrmn送信完了時に呼ばれる　引数はエラーの有無
	vmc1_vFp_uc fp_finSendPac;
	//existSendDat関数の実行の可否を確認
	vmc1_bFp_v fp_can_existSendDat;
	//送信が必要なデータの有無の確認 実行は，必ずcan_existSendDatを確認後
	vmc1_bFp_v fp_existSendDat;
	//iniSendDatを実行して良いかの確認に呼ばれる 実行は，必ずexistSendDatを確認後
	vmc1_bFp_v fp_can_iniSendDat;
	//Dat送信確定時に呼ばれる　サイズとidを戻す 実行は，必ずcan_iniSendDatを確認後
	vmc1_vFp_pdid_pdsize fp_iniSendDat;
	//Dat送信終了時に呼ばれる　引数はエラーの有無
	vmc1_vFp_uc fp_finSendDat;
	//sendを実行してよいかの確認に呼ばれる
	vmc1_bFp_v fp_can_send;
	//Datの中身送信時に呼ばれる 実行は，必ずfp_can_sendを確認後
	vmc1_ucFp_v fp_send;
}VMC1;
//VMC1を作成する VMCIDの指定が必要
VMC1* vmc1_create
	(vmc1_bFp_v fp_can_iniRecvPac_		//iniRecvPacが実行可能か
	,vmc1_vFp_v fp_iniRecvPac_			//PacStrt受信完了時に呼ばれる
	,vmc1_vFp_uc fp_finRecvPac_			//PacTrmn受信完了時に呼ばれる　引数はエラーの有無
	,vmc1_bFp_v fp_can_iniRecvDat_		//iniRecvDatを実行して良いかの確認に呼ばれる
	,vmc1_vFp_did_dsize fp_iniRecvDat_	//Dat受信確定時にサイズとidを渡される
	,vmc1_vFp_uc fp_finRecvDat_			//Dat受信終了時に呼ばれる 引数はエラーの有無
	,vmc1_bFp_v fp_can_recv_			//recvを実行してよいかの確認に呼ばれる
	,vmc1_vFp_uc fp_recv_				//Datの中身受信時に呼ばれる　引数が受信したデータ
	,vmc1_bFp_v fp_can_iniSendPac		//iniSendPacが実行可能か
	,vmc1_vFp_v fp_iniSendPac_			//RacStrt送信完了時に呼ばれる
	,vmc1_vFp_uc fp_finSendPac_			//PacTrmn送信完了時に呼ばれる　引数はエラーの有無
	,vmc1_bFp_v fp_can_existSendDat_	//existSendDat関数が実行可能かを問い合わせる
	,vmc1_bFp_v fp_existSendDat_		//送信が必要なデータの有無の確認
	,vmc1_bFp_v fp_can_iniSendDat_		//iniSendDatを実行して良いかの確認に呼ばれる
	,vmc1_vFp_pdid_pdsize fp_iniSendDat_//Dat送信確定時に呼ばれる　サイズを戻す
	,vmc1_vFp_uc fp_finSendDat_			//Dat送信終了時に呼ばれる　引数はエラーの有無
	,vmc1_bFp_v fp_can_send_			//sendを実行してよいかの確認に呼ばれる
	,vmc1_ucFp_v fp_send_);				//Datの中身送信時に呼ばれる
//VMC1を終了する
void vmc1_release(VMC1* pVMC1);
//VMC1を初期化する 引数は送信先のVMCID
void vmc1_initialize(VMC1* pVMC1,const unsigned char VMCID_[3],const unsigned char TVMCID_[3]);
//VMC1を終端化する
void vmc1_finalize(VMC1* pVMC1);
//受信データを投げ入れ可能か確認 0:不可,1:可
hmLib_boolian  vmc1_can_recv(VMC1* pVMC1);
//受信データを投げ入れる
void vmc1_recv(VMC1* pVMC1,unsigned char c_);
//送信データを呼び出し可能か確認 0:不可,1:可
hmLib_boolian  vmc1_can_send(VMC1* pVMC1);
//送信データを呼び出す
unsigned char vmc1_send(VMC1* pVMC1);
//受信を強制的に終了させる
void vmc1_force_end_recv(VMC1* pVMC1,unsigned char Err);
//送信を強制的に終了させる
void vmc1_force_end_send(VMC1* pVMC1,unsigned char Err);
//送信エラーを取得する
unsigned char vmc1_getSendErr(VMC1* pVMC1);
//送信エラーをクリアする
void vmc1_clearSendErr(VMC1* pVMC1);
//受信エラーを取得する
unsigned char vmc1_getRecvErr(VMC1* pVMC1);
//受信エラーをクリアする
void vmc1_clearRecvErr(VMC1* pVMC1);
#ifdef __cplusplus
}	//extern "C"{
#endif
#
#endif
