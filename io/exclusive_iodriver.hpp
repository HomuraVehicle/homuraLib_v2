#ifndef HMR_IODRIVER_EXCULUSIVEIODRIVER_INC
#define HMR_IODRIVER_EXCULUSIVEIODRIVER_INC 100

/*--- homura IO Driver ---
=== homuraLib_v2::io::exclusive_iodriver ===
v1_00/140707 hmIto
	Picからも使えるように機能制限
===IODriver::ExclusiveFunctionalIODriver===
v1_00/140126 hmIto
cExclusiveFunctionalIODriverを追加
	非スレッド関数でIOを排他的に駆動するタイプのIODriver
	排他的なので、複数IOの同時駆動はできない
	void operator()(void)関数を定期的に実行してやる必要がある
 */

#include <map>
#include<homuraLib_config/allocator.hpp>
#include "io_interface.hpp"
#include "iodriver_interface.hpp"

namespace hmr {
	namespace io {
		//IOを排他的に駆動するタイプのIODriver
		class exclusive_iodriver:public iodriver_interface{
		private:
			typedef std::map<ch_t, io_interface&,std::less<ch_t>,allocator> io_map;
			typedef std::pair<ch_t, io_interface&> io_pair;
		private:
			io_map TaskMap;
			ch_t TaskCh;
		public:
			exclusive_iodriver() :TaskCh(0) {}
		public://--- iodriver_interface ---
			// start関数 : Ch で指定されたものの通信を開始する
			virtual bool start(ch_t Ch_) {
				auto itr=TaskMap.find(Ch_);

				// そのChが存在しなければ、エラーを返す
				if(itr == std::end(TaskMap)) return true;
				itr->second.start();

				//Ch登録
				TaskCh=Ch_;

				return false;
			}
			// Ch指定stop関数 : Ch で指定されたものの通信を終了する
			virtual void stop(ch_t Ch_) {
				if(TaskCh!=Ch_)return;
				stop();
			}
			// stop関数 : 動いているものをすべて止めてしまう
			virtual void stop() {
				//現在タスクが駆動していなければ、エラーを返す
				if(TaskCh==0)return;

				//タスクをクリア
				auto itr=TaskMap.find(TaskCh);

				// そのChが存在しなければ、エラーを返す
				if(itr == std::end(TaskMap)) return;

				//Taskを止める
				itr->second.stop();
				TaskCh=0;
			}
			// Ch指定is_start関数 : Ch で指定されたものが通信中かを返す
			virtual bool is_start(ch_t Ch_)const {
				return TaskCh==Ch_;
			}
			// is_start関数 : いずれかが通信中かを返す
			virtual bool is_start()const {
				return TaskCh>0;
			}
		public:
			// iotaskをレジストしていく
			bool regist(ch_t Ch_, io_interface& rIO_) {
				//0Chは占有
				if(Ch_==0)return true;

				//登録済みのChなら、エラーを返す
				if(TaskMap.find(Ch_) != std::end(TaskMap))return true;

				//TaskMapに登録
				TaskMap.insert(io_pair(Ch_, rIO_));

				return false;
			}
			//現在のチャンネルを取得
			ch_t getCh()const { return TaskCh; }
		};
	}
}
#
#endif
