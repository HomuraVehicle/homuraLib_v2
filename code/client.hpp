#ifndef HMR_CODE_CLIENT_INC
#define HMR_CODE_CLIENT_INC 200
#
#include<homuraLib_v2/gate.hpp>
/*--- code::client ---
code形式の通信を依頼するためのclientクラスを定義している

v1_00/140413 hmIto
	cpp化
=== code ===
v1_04/140302 hmIto
	skip_get関数を追加
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
namespace hmr {
	namespace code {
		class client :public gate {
		public:
			//======== gate interface =======
			//すでに利用可能になっているか
			virtual bool is_open()=0;
			//sendできるか？
			virtual bool can_getc()=0;
			//送信文字列を1byte取得する
			virtual unsigned char getc()=0;
			//送信文字列がeof位置=Pac終端かどうかを検知する
			virtual bool flowing()=0;
			//recvできるか？(単にreturn 1)
			virtual bool can_putc()=0;
			//受信文字列を1byte与える
			virtual void putc(unsigned char c)=0;
			//flushする(eof化する=Pacを閉じる)
			virtual void flush()=0;
		public:
			//======= vcom functions ========
			//送信先chを取得する
			virtual unsigned char get_ch()=0;
			//送信をキャンセルする
			virtual void cancel_get()=0;
			//送信をスキップする
			virtual void skip_get()=0;
			//受信するchを取得する
			virtual void put_ch(unsigned char Ch)=0;
			//受信をキャンセルする
			virtual void cancel_put()=0;
		};
	}
}
#
#endif
