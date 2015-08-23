#ifndef HMR_VCOM_VCOM_INC
#define HMR_VCOM_VCOM_INC 100
#
/*---vcom---
gateもどきの文字列送受信用フォーマット
*/
namespace hmr {
	class gate {
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
	};
}
#
#endif
