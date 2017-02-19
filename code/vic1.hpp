#ifndef HMR_CODE_VIC1_INC
#define HMR_CODE_VIC1_INC 200
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
v2_00/140413 hmIto
	C++化
v1_02/140302 hmIto
	タイムアウト回数に上限を設け、オーバーした場合は相応の措置をとるように修正
	例えば、存在しない相手宛のStartを無限に繰り返してしまう問題を解決
v1_01/140115 hmIto
	チェックサムにcrc8を採用
v1_00/131224 hmIto
	リリース
*/

#define MaxTimeOutNum 5
//ch
#define ch_NULL		0xFF

#include<memory>
#include<hmLib_v3_06/type.h>
#include<homuraLib_v2/gate.hpp>
#include"client.hpp"
namespace hmr {
	namespace code {
		//vic1構造体の宣言
		class vic1 :public gate{
		public:
			enum error {
				error_NULL=0x00,
				error_SAVE_REJECTED=0x10,
				error_INCORRECT_SENDCNT=0x20,
				error_OVERFLOW=0x30,
				error_OVERREAD=0x40,
				error_INVALID_GETC=0x50
			};
		private:
			enum mode {
				mode_IDLE=0x00,
				mode_START=0x10,
				mode_SEND=0x20,
				mode_RESEND=0x21,
				mode_STOP=0x30,
				mode_RECV=0x40
			};
			enum cmd {
				cmd_ACK=0x00,
				cmd_NACK=0x10,
				cmd_START=0x20,
				cmd_STOP=0x30,
				cmd_DATA=0x40,
				cmd_ESC=0x50,
				cmd_WHAT=0x60,
				cmd_ERR=0xF0,
				cmd_NULL=0xFF
			};
		private:
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

		public:
			class timeout_functions {
			public:
				virtual void restart()=0;
				virtual void enable()=0;
				virtual void disable()=0;
			};
		private:
			timeout_functions* pTimeout;
		private:
			class buffer :public gate{
			private:
				vic1* VIC1;
				//送受信用インターフェース
				client* pClient;
				//送受信共有バッファ
				unsigned char* Begin;
				unsigned char* End;
				//送受信共有バッファのイテレータ
				unsigned char* GateItr;
				//VCom側のイテレータ
				unsigned char* VComItr;
			public:
				//バッファ初期化
				buffer(vic1* VIC1_, unsigned char* BufBegin, unsigned char* BufEnd);
				//バッファ初期化
				buffer(vic1* VIC1_,client* pClient_, unsigned char* BufBegin, unsigned char* BufEnd);
				//バッファ終端化
				~buffer();
			public:
				bool open(client* pClient_);
				bool close();
			private:
				//補充できる限り補充
				void load();
			public:
				//========== vic1::buf ===========
				//すでに利用可能になっているか
				virtual bool is_open();
				//バッファに書き込み可能か
				virtual bool can_putc();
				//バッファへ書き込み
				virtual void putc(unsigned char c);
				//バッファへ書き込んだデータを処理
				virtual void flush();
				//バッファからの読み出し可能か
				virtual bool can_getc();
				//バッファからの読み出し
				virtual unsigned char getc();
				//バッファへからの読み出しが区切り位置にいるか
				virtual bool flowing();
			public:
				//========== 
				//バッファへの読み込みをなかったことにする
				void cancel_get();
				//バッファの内部データをすべて破棄し、vcomにもPacket終了を通知
				void errorflush();
				//バッファへの書き込み/読み込みを終了し、バッファをクリアする
				void clear();
				//vcom自体がデータを持っているか
				bool vcom_can_getc();
				//vcom自体がflowingはどうなっているか
				bool vcom_flowing();
				//vcom自体にデータ取得をスキップさせる
				void vcom_skip_get();
				//vcom自体がflowingはどうなっているか
				unsigned char vcom_get_ch();
			};
			buffer Buffer;
		public:
			//vic1を初期化する
			//	BufEnd - BufBegin - 1 が最大データサイズ長となる
			//	最大データサイズ長は通信するすべてのデバイス間で同一である必要がある
			//	timeout関数の呼び出しタイミング通告用関数群を引き渡す
			vic1(client& rClient_
				, const unsigned char Ch
				, unsigned char* BufBegin
				, unsigned char* BufEnd
				, timeout_functions& timeout_);
			//vic1を初期化する
			//	BufEnd - BufBegin - 1 が最大データサイズ長となる
			//	最大データサイズ長は通信するすべてのデバイス間で同一である必要がある
			//	timeout関数の呼び出しタイミング通告用関数群を引き渡す
			vic1(const unsigned char Ch
				, unsigned char* BufBegin
				, unsigned char* BufEnd
				, timeout_functions& timeout_);			//vic1を終端化する
			~vic1();
		public:
			bool open(client& rClient_);
			bool close();
		private:
			//========== vic1::gate ===========
			//-------- Mode遷移4関数 ----------------
			//コマンド送信完了時
			void inform_send();
			//自分宛のコマンド受信完了
			void inform_recv();
			//データの有無を確認して、IDLEからSTARTへ遷移する関数
			bool checkData();
		public:
			//timeoutもモードを一部変えているでござる
			//	vic1のtimeoutコマンド再送関数
			void timeout();
		public:
			//========== gate interface =================
			//すでに利用可能になっているか
			virtual bool is_open();
			//受信データを投げ入れ可能か確認 0:不可,1:可
			virtual bool can_putc();
			//受信データを投げ入れる
			virtual void putc(unsigned char c_);
			//受信データをフラッシュする（gate互換用、VIC内での処理はない）
			virtual void flush();
			//送信データを呼び出し可能か確認 0:不可,1:可
			virtual bool can_getc();
			//送信データを呼び出す
			virtual unsigned char getc();
			//送信データを流れ続けているか確認（VICの場合は、データシーケンス末尾が0、それ以外は1）
			virtual bool flowing();
		public:
			//============ vic functions ================
			//通信を強制的に終了させる
			void force_end();
			//エラーを取得する
			unsigned char error() { return Err; }
			//エラーをクリアする
			void clear_error() { Err=error_NULL; }
		private:
			//受信データ1byte目判別関数
			void putc_1byte(unsigned char c);
			//受信データ2byte目判別関数
			void putc_2byte(unsigned char c1, unsigned char c2);
			//受信データ3byte目判別関数
			void putc_3byte(unsigned char c1, unsigned char c2, unsigned char c3);
			//受信データ3byte目判別関数
			void putc_4byte(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4);
		};
	}
}
#
#endif
