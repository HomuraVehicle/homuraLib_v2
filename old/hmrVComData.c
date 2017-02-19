#ifndef HMR_VCOMDATA_C_INC
#define HMR_VCOMDATA_C_INC 101
#
#ifndef HMR_VCOMDATA_INC
#	include"hmrVComData.h"
#endif
//フォーマット
void vcom_data_format(vcom_data* Data) {
	Data->Ch=0x0F;
	Data->ID=vcom_data_id_NULL;
	Data->Err=vcom_data_error_NULL;
	Data->Accessible=0;
	cstring_format(&(Data->Data));
}
//コンストラクト済みかどうか(最初にformatされていない場合は、動作不定)
hmLib_boolian vcom_data_is_construct(vcom_data* Data) { return Data->ID!=vcom_data_id_NULL; }
//コンストラクト
void vcom_data_construct(vcom_data* Data, hmLib_cstring* mStr, vcom_ch_t Ch, vcom_id_t ID, vcom_err_t Err, hmLib_u16Fp_v Accessible) {
	Data->Accessible=Accessible;
	Data->Ch=Ch;
	Data->Err=Err;
	Data->ID=ID;
	cstring_move(mStr, &(Data->Data));
}
//ディストラクト(最初にformatもconstructもされていない場合は、動作不定)
void vcom_data_destruct(vcom_data* Data) {
	if(vcom_data_is_construct(Data)==0)return;
	Data->Accessible=0;
	Data->Ch=0x0F;
	Data->Err=0;
	Data->ID=vcom_data_id_NULL;
	cstring_destruct(&(Data->Data));
}
//eofデータにする
void vcom_data_set_eof(vcom_data* Data) {
	Data->ID=vcom_data_id_EOF;
}
//eofデータかどうかを判別する
hmLib_boolian vcom_data_eof(vcom_data* Data) {
	return Data->ID==vcom_data_id_EOF;
}
//move
void vcom_data_move(vcom_data* From, vcom_data* To) {
	To->Ch=From->Ch;
	To->ID=From->ID;
	To->Err=From->Err;
	To->Accessible=From->Accessible;
	cstring_move(&(From->Data), &(To->Data));

	vcom_data_format(From);
}
//swap
void vcom_data_swap(vcom_data* Data1, vcom_data* Data2) {
	static vcom_data Tmp;
	vcom_data_move(Data1, &Tmp);
	vcom_data_move(Data2, Data1);
	vcom_data_move(&Tmp, Data2);
}
#
#endif
