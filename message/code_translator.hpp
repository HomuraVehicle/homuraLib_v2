#ifndef HMR_MESSAGE_CODETRANSLATOR_INC
#define HMR_MESSAGE_CODETRANSLATOR_INC 100
#
#include<algorithm>
#include<vector>
#include<homuraLib_v2/code/client.hpp>
#include"io_interface.hpp"
#include"data.hpp"
#include"exception.hpp"

#define HMR_MESSAGE_CODE_MAXSIZE 4096

namespace hmr {
	namespace message {
		class code_translator :public code::client{
		private:
			enum recvmode {
				recvmode_ID=0,
				recvmode_SIZE1=1,
				recvmode_SIZE2=2,
				recvmode_DATA=3,
				recvmode_IGNORE=4,
				recvmode_ERROR=5
			};
			enum sendmode {
				sendmode_ID=0,
				sendmode_SIZE1=1,
				sendmode_SIZE2=2,
				sendmode_DATA=3
			};
		private:
			typedef std::pair<unsigned char, io_interface*> element;
			typedef std::vector<element> containor;
			typedef containor::iterator iterator;
			typedef containor::const_iterator const_iterator;
		private:
			struct compare{
				bool operator()(const element& Element1, const element& Element2)const{
					return  Element1.first<Element2.first;
				}
			};
		private:
			containor pIOs;
			bool IsLock;
			//============== recv ===============
			unsigned char RecvMode;
			iterator RecvIO;
			unsigned int RecvCnt;
			shared_write_array RecvDataArray;
			unsigned char RecvDataID;
			data::error_type RecvDataError;
			//============== send ===============
			unsigned char SendMode;
			data SendData;
			iterator SendIO;
			unsigned int SendCnt;
			bool SendIncrement;
		public:
			//code初期化
			code_translator()
				: IsLock(false)
				, RecvMode(recvmode_ID)
				, RecvCnt(0)
				, RecvDataArray()
				, RecvDataID(data_id::null)
				, RecvDataError(error::null)
				, SendMode(sendmode_ID)
				, SendData()
				, SendCnt(0)
				, SendIncrement(false){
			}
		public:
			//======== code_translator regist functons ======
			bool open(unsigned char Ch_, io_interface& IO_){
				if(is_lock())return true;
				pIOs.push_back(element(Ch_,&IO_));
				return false;
			}
			void close() {
				if(is_lock())return;
				pIOs.clear();
			}
			void lock() {
				if(is_lock())return;

				if(pIOs.size()==0)return;

				std::stable_sort(pIOs.begin(), pIOs.end(), compare());
				SendIO=pIOs.begin();
				RecvIO=pIOs.begin();
				SendIncrement=true;
			}
			void unlock() {
				IsLock=false;
			}
			bool is_lock(){
				return IsLock;
			}
		public:
			//======== code_translator access functons ======
			std::pair<unsigned int, unsigned int> getRecvCntSize()const {
				if(RecvDataArray==false) {
					return std::pair<unsigned int, unsigned int>(0, RecvCnt);
				} else {
					std::pair<unsigned int, unsigned int>(RecvCnt,RecvDataArray.size());
				}
			}
			std::pair<unsigned int, unsigned int> getSendCntSize()const {
				if(SendData.getData()==false) {
					return std::pair<unsigned int, unsigned int>(0, 0);
				} else {
					std::pair<unsigned int, unsigned int>(SendCnt, SendData.size());
				}
			}
		public:
			//======== gate interface =======
			//すでに利用可能になっているか
			virtual bool is_open() {
				return pIOs.size()>0;
			}
			//sendできるか？
			virtual bool can_getc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//Dataが空でないなら、終了
				if(SendData==true) {
					if(SendMode==sendmode_DATA)return SendData.size() > SendCnt;
					else return 1;
				}

				//前回インクリメントされていなくて、flowingが落ちていれば、インクリメント
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				//Dataを読み出せないなら終了
				if(SendIO->second->can_get()==false)return 0;

				//Data読み出し 失敗したら終了
				if(SendIO->second->get(SendData))return 0;
				SendCnt=0;

				//空データならはじく
				if(SendData==false)return 0;

				//Data読み出しに成功すれば、OK
				SendIncrement=false;
				return 1;
			}
			//送信文字列を1byte取得する
			virtual unsigned char getc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//Dataが空なら、終了
				if(SendData==false) {
					SendMode=sendmode_ID;
					return 0xC0;
				}

				unsigned char c;
				//ID送信モード
				switch(SendMode) {
				case sendmode_ID:
					SendMode=sendmode_SIZE1;
					return SendData.getID();
				case sendmode_SIZE1:
					SendMode=sendmode_SIZE2;
					return (unsigned char)(SendData.size());
				case sendmode_SIZE2:
					if(SendData.size()==0) {
						SendMode=sendmode_ID;
						c=0;
						SendData.release();
					} else {
						SendMode=sendmode_DATA;
						c=(unsigned char)(SendData.size()>>8);
					}
					SendCnt=0;
					return c;
				case sendmode_DATA:
					c=SendData[(SendCnt)++];
					if(SendCnt==SendData.size()) {
						SendData.release();
						SendMode=sendmode_ID;
					}
					return c;
				default:
					SendMode=sendmode_ID;
					return 0xCC;
				}
			}
			//送信文字列がeof位置=Pac終端かどうかを検知する
			virtual bool flowing() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//前回インクリメントされていなくて、flowingが落ちていれば、インクリメント
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				return SendIO->second->flowing();
			}
			//recvできるか？(単にreturn 1)
			virtual bool can_putc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//RecvIOが無効なら、大歓迎（無視するけど）
				if(RecvIO==pIOs.end())return true;

				//エラー状態の時には、大歓迎（無視するけど）
				if(RecvMode==recvmode_ERROR)return true;

				//バッファ側が受信できるならOK
				return RecvIO->second->can_put();
			}
			//受信文字列を1byte与える
			virtual void putc(unsigned char c) {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//RecvIOが無効なら、大歓迎（無視するけど）
				if(RecvIO==pIOs.end())return;

				//データid受信時
				if(RecvMode==recvmode_ID) {
					//NullIDはつかえないので無視
					if(c==message::data_id::null) {
						//以後のデータはflushがあるまで信じられないのでモードを移行
						RecvMode=recvmode_ERROR;
						return;
					}

					//データIDを取得
					RecvDataID=c;
					RecvDataError=error::null;
					RecvDataArray.release();

					//IDに関するエラー登録
					if(data_id::is_special_id(RecvDataID)) RecvDataError|=error::unknown;

					//モードをサイズ受信モード(SIZE1)へ移行
					RecvMode=recvmode_SIZE1;
				}//データサイズ受信待1
				else if(RecvMode==recvmode_SIZE1) {
					//データサイズを取得
					RecvCnt=static_cast<unsigned char>(c);

					//モードをサイズ受信モード(SIZE2)へ移行
					RecvMode=recvmode_SIZE2;
				}//データサイズ受信待2
				else if(RecvMode==recvmode_SIZE2) {
					//データサイズを取得
					RecvCnt|=(static_cast<unsigned int>(static_cast<unsigned char>(c))<<8);

					//データサイズが規格上限を上回っていた場合は無視
					if(RecvCnt>HMR_MESSAGE_CODE_MAXSIZE) {
						//データを送る
						if(RecvIO->second->can_put()) {
							RecvDataError|=error::sizeover;
							RecvIO->second->put(data(RecvDataID, RecvDataArray, RecvDataError));
						}

						//以後のデータはflushがあるまで信じられないのでモードを移行
						RecvMode=recvmode_ERROR;
						return;
					}

					//データサイズが0の時
					if(RecvCnt==0) {
						//データを送る
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, RecvDataArray, RecvDataError));
						}

						//モードをID受信モードへ移行
						RecvMode=recvmode_ID;
						return;
					}

					//バッファを確保
					RecvDataArray = make_shared_array(RecvCnt);

					//データサイズ確保に失敗したとき
					if(RecvDataArray.size()!=RecvCnt) {
						//確保失敗エラー登録
						RecvDataError|=error::failnew;
						//以後、書き込みできないので、次のデータがあるまで無視
						RecvMode=recvmode_IGNORE;
						return;
					}

					//モードをPac受信モードへ移行
					RecvCnt=0;
					RecvMode=recvmode_DATA;
				}//データ受信中
				else if(RecvMode==recvmode_DATA) {
					//データ書き込み
					RecvDataArray[RecvCnt]=c;

					//書き込みサイズ上限に達した場合
					if(RecvCnt==RecvDataArray.size()) {
						//データを送る
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
						}
						RecvDataArray.release();
						
						//モードをID受信モードへ移行
						RecvMode=recvmode_ID;
						return;
					}
				}//データ無視中
				else if(RecvMode==recvmode_IGNORE) {
					--(RecvCnt);

					//全データ受信し終えたとき
					if(RecvCnt==0) {
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
						}
						RecvDataArray.release();

						//モードをID受信に移行
						RecvMode=recvmode_ID;
						return;
					}
				}
			}
			//flushする(eof化する=Pacを閉じる)
			virtual void flush() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//受信中のデータが存在する場合
				if(RecvMode==recvmode_DATA || RecvMode==recvmode_IGNORE) {
					//未受信完了エラー登録
					RecvDataError|=error::underflow;

					//データ送付
					if(RecvIO->second->can_put()) {
						RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
					}
					RecvDataArray.release();
				}

				//フラッシュ！
				RecvIO->second->flush();

				//モードをID受信に移行
				RecvMode=recvmode_ID;
			}
		public:
			//======= vcom functions ========
			//送信先chを取得する
			virtual unsigned char get_ch() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				//前回インクリメントされていなくて、flowingが落ちていれば、インクリメント
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				return SendIO->first;
			}
			//送信をキャンセルする
			virtual void cancel_get() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				SendMode=sendmode_ID;
				SendCnt=0;
			}
			//送信をスキップする
			virtual void skip_get() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				SendMode=sendmode_ID;
				SendCnt=0;
				SendData.release();
			}
			//受信するchを取得する
			virtual void put_ch(unsigned char Ch) {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				RecvIO=std::lower_bound(pIOs.begin(), pIOs.end(), element(Ch, 0), compare());
			}
			//受信をキャンセルする
			virtual void cancel_put() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lockされてなければ、自動ロック
				if(!is_lock())lock();

				RecvMode=recvmode_ID;
				RecvCnt=0;
				RecvDataArray.release();
			}
		};
	}
}
#
#endif
