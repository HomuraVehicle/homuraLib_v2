#ifndef HMR_IO_IODRIVERINTERFACE_INC
#define HMR_IO_IODRIVERINTERFACE_INC 100

/*--- homuraLib_v2::io::iodriver_interface ---
=== homuraLib_v2::io::iodriver_interface ===
v1_00/140707 hmIto
	起動、停止制御のみに制限して、作成
===IODriver::ItfIODriver===
v1_00/140126 hmIto
cExclusiveWorkIODriverを追加
非スレッド関数でIOを排他的に駆動するタイプのIODriver
排他的なので、複数IOの同時駆動はできない
void operator()(void)関数を定期的に実行してやる必要がある
v1_00/140118 amby
作成開始

設計
　	通信を開始するとTaskのオペレータを定期的に呼ぶ
	　
	 */

#include <memory>
#include <functional>
namespace hmr {
	namespace io{
		//IODriverの基底クラス
		class iodriver_interface {
		public:
			typedef unsigned int ch_t;
		public://--- iodriver_interface ---
			// start関数 : Ch で指定されたものの通信を開始する
			virtual bool start(ch_t Ch_)=0;
			// Ch指定stop関数 : Ch で指定されたものの通信を終了する
			virtual void stop(ch_t Ch_)=0;
			// stop関数 : 動いているものをすべて止めてしまう
			virtual void stop()=0;
			// Ch指定is_start関数 : Ch で指定されたものが通信中かを返す
			virtual bool is_start(ch_t Ch_)const=0;
			// is_start関数 : いずれかが通信中かを返す
			virtual bool is_start()const=0;
		};
	}
}
#
#endif
