#ifndef HMR_VCOMDATA_INC
#define HMR_VCOMDATA_INC 101
#
#include<hmLib_v3_06/type.h>
#include<hmLib_v3_06/cstring.h>
/*---vcom---
Volcanic Communication Data Interface.
・ほむら用のデータ送受信用共通プロトコル
	各モジュールは、CH0-7の固有チャンネルを持つ
	各送受信データは、送信元CH[1], 識別ID[1], DataSize[2],Data[DataSize]を持っている
	識別IDは0x00-0xEFの値をとる。0xF0-0xFFはvcom制御に予約されており、独自使用は不可。
	DataSizeは、0～4096byteの範囲内
・vcomは、共通プロトコル準拠の通信規格から、データの送受信を行う。
・この共通プロトコルに準じている通信規格は、vcomを使って通信を行う
	vcomが保持する関数ポインタを用いて、通信が行われる。

=== vcom ===
v1_01/140110 hmIto
	IDにかかわる規約を改定
v1_00/131220 hmIto
	VMC1をもとに、一通りの機能を完成
*/
#ifdef __cplusplus
extern "C"{
#endif
	typedef hmLib_uint8 vcom_ch_t;
	typedef hmLib_uint8 vcom_id_t;
	typedef hmLib_uint8 vcom_err_t;
	typedef hmLib_cstring_size_t vcom_size_t;

	typedef struct {
		vcom_ch_t Ch;				//受信時の送信元Ch/送信時の送信先Ch
		vcom_id_t ID;				//Data識別ID
		vcom_err_t Err;			//DataErrコード
		hmLib_u16Fp_v Accessible;	//Dataアクセス可能領域
		hmLib_cstring Data;			//Data自体 Sizeも含まれている
	}vcom_data;

//vcom_data型が許容する最大データサイズを定義
#define vcom_data_MAXSIZE			4096

//vcom_data予約ID一覧
#define vcom_data_id_is_valid(id)	((unsigned char)(id)<0xF0)
#define vcom_data_id_NULL	0xFF	//Null ID
#define vcom_data_id_EOF	0xFE	//End of File識別用ID

//vcom_dataエラーコード一覧（重複可）
#define vcom_data_error_NULL		0x00		//Dataにエラーがない
#define vcom_data_error_STRANGER	0x01		//Dataの送信元CHがおかしい
#define vcom_data_error_UNKNOWN		0x02		//Dataの識別IDがおかしい
#define vcom_data_error_FAILNEW		0x04		//Dataの確保に失敗した
#define vcom_data_error_SIZEOVER	0x08		//DataのSizeが大きすぎる
#define vcom_data_error_OVERFLOW	0x10		//DataがSizeに比べて多すぎる
#define vcom_data_error_UNDERFLOW	0x20		//DataがSizeに比べて少なすぎる
#define vcom_data_error_BROKEN		0x40		//Dataが壊れている

	//フォーマット
	void vcom_data_format(vcom_data* Data_);
	//コンストラクト済みかどうか(最初にformatされていない場合は、動作不定)
	hmLib_boolian vcom_data_is_construct(vcom_data* Data);
	//コンストラクト
	void vcom_data_construct(vcom_data* Data, hmLib_cstring* mStr, vcom_ch_t Ch, vcom_id_t ID, vcom_err_t Err, hmLib_u16Fp_v Accessible);
	//ディストラクト(最初にformatもconstructもされていない場合は、動作不定)
	void vcom_data_destruct(vcom_data* Data);
	//eofデータにする
	void vcom_data_set_eof(vcom_data* Data_);
	//eofデータかどうかを判別する
	hmLib_boolian vcom_data_eof(vcom_data* Data_);
	//move
	void vcom_data_move(vcom_data* From, vcom_data* To);
	//swap
	void vcom_data_swap(vcom_data* Data1, vcom_data* Data2);

#ifdef __cplusplus
}
#endif
#
#endif
