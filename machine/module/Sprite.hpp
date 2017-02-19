#ifndef HMR_MACHINE_MODULE_SPRITE_INC
#define HMR_MACHINE_MODULE_SPRITE_INC 100
#
#include<XCBase/bytes.hpp>
#include<XCBase/future.hpp>
#include<XCBase/either.hpp>
#include<XC32/uart.hpp>
#include<homuraLib_v2/task.hpp>
#include"Sprite/command.hpp"
#include"Sprite/command_uart.hpp"
namespace hmr {
	namespace machine {
		namespace module {
			namespace sprite{
				namespace id = commands::id;
				namespace imagesize = commands::imagesize;
				struct picture_info{
					unsigned int PictureSize;
					sprite::commands::imagesize::type ImageSize;
				public:
					picture_info() :PictureSize(0), ImageSize(sprite::commands::imagesize::size_160_120){}
					picture_info(unsigned int PictureSize_,sprite::commands::imagesize::type ImageSize_)
						: PictureSize(PictureSize_)
						, ImageSize(ImageSize_){
					}
					bool valid()const { return PictureSize != 0; }
				};
				struct picture_data{
					unsigned int PicturePos;
					unsigned int DataSize;
					bool IsLast;
				public:
					picture_data()
						: PicturePos(0)
						, DataSize(0)
						, IsLast(false){
					}
					picture_data(unsigned int PicturePos_, unsigned int DataSize_, bool IsLast_)
						: PicturePos(PicturePos_)
						, DataSize(DataSize_)
						, IsLast(IsLast_){
					}
					bool valid()const { return DataSize != 0; }
				};
			}
			//一連のコマンドをシーケンス化して使いやすくした、Sprite用Uart
			//	タイムアウト、電源管理、オートリセット等は管理していないので注意
			template<typename uart_register_>
			class cSpriteUart {
				friend class testSpriteUart;
				typedef cSpriteUart<uart_register_> my_type;
			private:
				//Sprite用Uart
				sprite::command_uart<uart_register_> CommandUart;
				xc::unique_lock<sprite::command_uart<uart_register_>> CommandUartLock;
			public:
				typedef sprite::command_uart_status status_type;
				//Statusチェック関数
				status_type status()const{ return CommandUart.status(); }
				//Idleかどうか確認（Idleでないと、命令形のメソッドは失敗する）
				bool idle()const { return !CommandUart.is_command(); }

				//======エラー管理======
			public:
				struct error_category{
					enum type{
						null = 0x00,
						FailRequestCommand = 0x01,
						FailCommand = 0x02,
						InvalidCommand = 0x03,
						Canceled = 0x04
					};
				};
				typedef typename error_category::type error_category_type;
				struct error_type{
				private:
					error_category_type ErrorCaterogy;
					status_type Status;
				public:
					error_type() 
						: ErrorCaterogy(error_category::null)
						, Status(){
					}
					error_type(error_category_type ErrorCaterogy_, status_type Status_)
						: ErrorCaterogy(ErrorCaterogy_)
						, Status(Status_){
					}
					operator bool()const{ return ErrorCaterogy != error_category::null; }
					error_category_type category()const{ return ErrorCaterogy; }
					unsigned char mode()const{ return Status.mode(); }
					sprite::id::type command()const{ return Status.id(); }
				};

				//====== TakePicture ======
			private:
				//カメラの画像サイズ
				sprite::commands::imagesize::type ImageSize;
				//写真のデータサイズ
				unsigned int DataSize;
				//写真のデータ読み込みカウンタ
				unsigned int DataCnt;
			private:
				//=== TakePictureシーケンス ===
				//	setImageSize(ImageSizeを変更時のみ) 
				//	=> takePicture
				//	=> getDataSize
				//async_takePicture用Promise
				struct sequence_takePicture{
					friend class testSpriteUart;
				public:
					typedef xc::either<sprite::picture_info, error_type> ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;
				private:
					my_type& Ref;
					my_promise Promise;
					xc::future<bool>::canceler Canceler;
					sprite::commands::ans_getDataSize Ans_getDataSize;
				public:
					sequence_takePicture(my_type& Ref_) :Ref(Ref_){}
					my_future  get_future(sprite::commands::imagesize::type ImageSize_){
						//Promiseが現在も使われたままの場合は失敗
						if(!Promise.can_get_future())return my_future();

						//future発行 非同期処理開始
						my_future Future = Promise.get_future();

						//ImageSize_をチェック 異常値なら160_120に設定
						if (ImageSize_ != sprite::commands::imagesize::size_640_480 && ImageSize_ != sprite::commands::imagesize::size_320_240){
							ImageSize_ = sprite::commands::imagesize::size_160_120;
						}

						//現在のカメラのImageSizeと同一ならtakePicture、違えばまずgetDataSize
						if (Ref.ImageSize != ImageSize_){
							//ImaseSizeが異なる場合は、setImageSizeから
							Ref.ImageSize = ImageSize_;

							//setImageSizeを非同期実行 & takePicture実行を予約
							Canceler = Ref.CommandUart.async_command(sprite::commands::setImageSize(ImageSize_)).then(xc::ref(*this));
							if(!Canceler){
								Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							}
						}
						else{
							//takePictureを非同期実行 & getDataSize実行を予約
							Canceler = Ref.CommandUart.async_command(sprite::commands::takePicture()).then(xc::ref(*this));
							if(!Canceler){
								Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							}
						}

						//future発行
						//	then_getDataSize実行時に履行
						return Future;
					}
					void operator()(bool IsFail_){
						//正しくthenが呼ばれたので、まず覚えていたcancelerを消去
						Canceler.clear();

						if (IsFail_){
							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						switch(Ref.CommandUart.status().id()) {
						case sprite::commands::id::SetImageSize:
							//takePictureを非同期実行 & getDataSize実行を予約
							Canceler = Ref.CommandUart.async_command(sprite::commands::takePicture()).then(xc::ref(*this));

							if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return;
						case sprite::commands::id::TakePicture:
							//getDataSizeを非同期実行 & TakePictureシーケンスの後始末実行を予約
							Canceler = Ref.CommandUart.async_command(sprite::commands::getDataSize(Ans_getDataSize)).then(xc::ref(*this));

							if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return;
						case sprite::commands::id::GetDataSize:
							//Ansのデータを記録しておく
							Ref.DataSize = Ans_getDataSize.Size;

							//Promise履行
							Promise.set_value(sprite::picture_info(Ref.DataSize, Ref.ImageSize));
							return;
						default:
							Promise.set_value(error_type(error_category::InvalidCommand, Ref.CommandUart.status()));
							return;
						}

					}
					void cancel(){
						//現在保持している場合は、cancel処理実行
						if(Canceler)Canceler();

						//現在get_future中なら、無効化
						if(Promise.is_wait_value())Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
					}
				}Sequence_takePicture;
			private:
				//=== ReadPictureシーケンス ===
				//	getData
				//	=> stop_takePicture (最終データ時のみ)
				struct sequence_readPicture{
					friend class testSpriteUart;
				public:
					typedef xc::either<sprite::picture_data, error_type> ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;
				private:
					my_type& Ref;
					my_promise Promise;
					xc::future<bool>::canceler Canceler;
					sprite::commands::ans_getData Ans_getData;
					//書き込みバッファの先頭アドレス
					unsigned char* BufPtr;
					unsigned int DataPos;
					unsigned int DataSize;
				private:
					sequence_readPicture(const sequence_readPicture&);
				public:
					sequence_readPicture(my_type& Ref_) :Ref(Ref_){}
					my_future get_future(unsigned char* BufPtr_, unsigned int BufSize_){
						//以下の場合は失敗
						//	Promiseが現在も使われたまま
						if(!Promise.can_get_future())return my_future();

						//future発行 非同期処理開始
						my_future Future = Promise.get_future();

						//バッファ先頭アドレスを記録
						BufPtr = BufPtr_;

						//残データ数よりバッファのほうが小さいとき
						if(BufSize_ < Ref.DataSize - Ref.DataCnt){
							//8の倍数に丸め込み
							BufSize_ -= BufSize_ % 8;
						} else{
							//残データを全部読む
							BufSize_ = Ref.DataSize - Ref.DataCnt;
						}

						//バッファへの書き込み登録
						Ans_getData.Itr = BufPtr_;
						Ans_getData.End = BufPtr_ + BufSize_;

						//setImageSizeを非同期実行 & takePicture実行を予約
						Canceler = Ref.CommandUart.async_command(sprite::commands::getData(Ref.DataCnt, BufSize_, Ans_getData)).then(xc::ref(*this));
						if(!Canceler){
							Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return Future;
						}

						//future発行
						return Future;
					}
					void operator()(bool IsFail_){
						//正しくthenが呼ばれたので、まず覚えていたcancelerを消去
						Canceler.clear();

						if(IsFail_){
							Ref.DataCnt = 0;
							Ref.DataSize = 0;

							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						switch(Ref.CommandUart.status().id()){
						case sprite::commands::id::GetData:
							//現在位置を記録
							DataPos = Ref.DataCnt;
							DataSize = Ans_getData.Itr - BufPtr;

							//結果を記録
							Ref.DataCnt += DataSize;

							//データが最後なら
							if (Ref.DataCnt >= Ref.DataSize){
								//end_takePictureを非同期実行 & その後始末実行を予約
								Canceler = Ref.CommandUart.async_command(sprite::commands::stop_takePicture()).then(xc::ref(*this));

								if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
								return;

							}

							//Promise履行
							Promise.set_value(sprite::picture_data(DataPos, DataSize, false));
							return;
						case sprite::commands::id::StopTakePicture:
							Ref.DataCnt = 0;
							Ref.DataSize = 0;

							//Promise履行
							Promise.set_value(sprite::picture_data(DataPos, DataSize, true));
							return;
						default:
							Ref.DataCnt = 0;
							Ref.DataSize = 0;
							Promise.set_value(error_type(error_category::InvalidCommand, Ref.CommandUart.status()));
							return;
						}
					}
					void cancel(){
						//現在保持している場合は、cancel処理実行
						if (Canceler)Canceler();

						//現在get_future中なら、無効化
						if (Promise.is_wait_value()){
							Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
						}
					}
				}Sequence_readPicture;
			private:
				//=== Command Reset シーケンス ===
				//	command_reset
				struct sequence_command_reset{
					friend class testSpriteUart;
				public:
					typedef xc::either<void, error_type> ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;
				private:
					my_type& Ref;
					my_promise Promise;
					xc::future<bool>::canceler Canceler;
				public:
					sequence_command_reset(my_type& Ref_) :Ref(Ref_){}
					my_future get_future(){
						//Promiseが現在も使われたままの場合は失敗
						if(!Promise.can_get_future())return my_future();

						//future発行 非同期処理開始
						my_future Future = Promise.get_future();

						//setImageSizeを非同期実行 & takePicture実行を予約
						Canceler = Ref.CommandUart.async_command(sprite::commands::resetCamera()).then(xc::ref(*this));
						if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));

						return Future;
					}
					void operator()(bool IsFail_){
						//正しくthenが呼ばれたので、まず覚えていたcancelerを消去
						Canceler.clear();

						if(IsFail_){
							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						//Promise履行
						Promise.set_value(ans_type());
					}
					void cancel(){
						//現在保持している場合は、cancel処理実行
						if (Canceler)Canceler();

						//現在get_future中なら、無効化
						if(Promise.is_wait_value())Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
					}
				}Sequence_command_reset;
			public:
				cSpriteUart()
					: CommandUart()
					, CommandUartLock(CommandUart,true)
					, ImageSize(sprite::commands::imagesize::size_160_120)
					, DataSize(0)
					, DataCnt(0)
					, Sequence_takePicture(*this)
					, Sequence_readPicture(*this)
					, Sequence_command_reset(*this){
				}
			public:
				bool lock(){
					if (is_lock())return false;
					if (CommandUartLock.lock())return true;

					//カメラの画像サイズ
					ImageSize=sprite::commands::imagesize::size_160_120;
					//写真のデータサイズ
					DataSize = 0;
					//写真のデータ読み込みカウンタ
					DataCnt = 0;

					return false;
				}
				void unlock(){
					//処理中シーケンスをキャンセル
					Sequence_takePicture.cancel();
					Sequence_readPicture.cancel();
					Sequence_command_reset.cancel();
					//ロック解除
					CommandUartLock.unlock();
				}
				bool is_lock()const{ return CommandUartLock; }
			public:
				//写真撮影
				//	return:撮影したPictureのpicture_info
				xc::future< xc::either<sprite::picture_info, error_type> > async_takePicture(sprite::commands::imagesize::type ImageSize_){
					//以下の場合は失敗
					//	アイドル状態=命令受付可能でない
					//	CommandUartが現在も使われたまま
					//	未読み込みPicture残留時
					if (!idle()
						|| CommandUart.is_command()
						|| can_readPicture()){
						return xc::future< xc::either<sprite::picture_info, error_type> >();
					}

					return Sequence_takePicture.get_future(ImageSize_);
				}
				//写真データが現在存在しているか
				//	return:readPicture可能な撮影済みPictureが存在?
				bool can_readPicture()const{ return DataCnt < DataSize; }
				//写真データ読み出し
				//	return:取得したDataのpicture_data
				xc::future< xc::either<sprite::picture_data, error_type> > async_readPicture(unsigned char* BufPtr_, unsigned int BufSize_){
					//以下の場合は失敗
					//	アイドル状態=命令受付可能でない
					//	CommandUartが現在も使われたまま
					//	読み込みPictureが存在しない
					//	Promise_readPictureが現在も使われたまま
					if (!idle()
						|| CommandUart.is_command()
						|| !can_readPicture()){
						return xc::future< xc::either<sprite::picture_data, error_type> >();
					}

					return Sequence_readPicture.get_future(BufPtr_, BufSize_);
				}
				//リセット要求
				//	return:失敗フラグ
				xc::future< xc::either<void, error_type> > async_command_reset(){
					//以下の場合は失敗
					//	アイドル状態=命令受付可能でない
					//	CommandUartが現在も使われたまま
					//	Promise_command_resetが現在も使われたまま
					if (!idle()
						|| CommandUart.is_command()){
						return xc::future< xc::either<void, error_type> >();
					}

					return Sequence_command_reset.get_future();
				}
			};
			
			//電源管理、タイムアウト機能、オートリセット機能付きの高機能Sprite管理クラス
			template<typename uart_register_, typename power_pin_>
			class cSprite {
				friend class testSprite;
			public:
				typedef cSprite<uart_register_, power_pin_> my_type;
				typedef cSpriteUart<uart_register_> my_uart;
				typedef typename my_uart::status_type uart_status_type;
			public:
				//===== 通知用関数型 =====
				typedef xc::bytes bytes;
				typedef typename bytes::size_type size_type;
				typedef xc::function<void(bytes&,size_type)> bytes_builder;
				struct picture_reader{
					virtual void take(sprite::picture_info PictureInfo_) = 0;
					virtual void read(sprite::picture_data PictureData_, xc::rvalue_reference<bytes> Data_) = 0;
				};
			public:
				//===== ステータス =====
				struct status_type{
					typedef sprite::command_uart_status uart_status_type;
				public:
					typedef typename uart_status_type::id_type id_type;
					typedef typename uart_status_type::mode_type mode_type;
				private:
					uart_status_type UartStatus;
					bool IsPowerReset;
				public:
					status_type() :UartStatus(), IsPowerReset(false){}
					status_type(uart_status_type UartStatus_, bool IsPowerReset_) :UartStatus(UartStatus_), IsPowerReset(IsPowerReset_){}
					id_type id()const{ return UartStatus.id(); }
					mode_type mode()const{ return UartStatus.mode(); }
					uart_status_type uart_status()const{ return UartStatus; }
					bool isPowerReset()const{ return IsPowerReset; }
					unsigned char byte()const{
						return (0x80 * isPowerReset()) | (0x60 & (mode() << 5)) | (0x1f & id());
					}
				};
				//======エラー管理======
				struct error_category{
					enum type{
						null = 0x00,
						FailRequestUart = 0x01,
						CatchUartError = 0x02,
						TimeOut = 0x03,
						Canceled =0x04,
						FailBuildBytes = 0x05,
						InvalidReadEnd = 0x06
					};
				};
				typedef typename error_category::type error_category_type;
				typedef typename my_uart::error_type uart_error_type;
				typedef typename my_uart::error_category uart_error_category;
				typedef typename uart_error_category::type uart_error_category_type;
				struct error_type{
				private:
					error_category_type ErrorCaterogy;
					uart_error_type UartErrorType;
				public:
					error_type()
						: ErrorCaterogy(error_category::null)
						, UartErrorType(){
					}
					error_type(error_category_type ErrorCaterogy_, status_type Status_)
						: ErrorCaterogy(ErrorCaterogy_)
						, UartErrorType(uart_error_category::null, Status_.uart_status()){
					}
					error_type(error_category_type ErrorCaterogy_, uart_error_type UartErrorType_)
						: ErrorCaterogy(ErrorCaterogy_)
						, UartErrorType(UartErrorType_){
					}
					operator bool()const{ return ErrorCaterogy != error_category::null; }
					error_category_type category()const{ return ErrorCaterogy; }
					uart_error_category_type uart_category()const{ return UartErrorType.category(); }
					unsigned char uart_mode()const{ return UartErrorType.mode(); }
					sprite::id::type uart_command()const{ return UartErrorType.command(); }
					unsigned char byte_pos()const{
						return (0x60 & (uart_mode() << 5)) | (0x1f & uart_command());
					}
					unsigned char byte_category()const{
						return (0xF0 & (static_cast<unsigned char>(category()) << 4)) | (0x0F & static_cast<unsigned char>(uart_category()));
						//return (0xF0 & (static_cast<unsigned char>(category()) << 8)) | (0x0f & static_cast<unsigned char>(uart_category()));
					}
				};

			private:
				bool IsLock;

				//Sprite用Uart
				my_uart SpriteUart;
				xc::unique_lock<cSpriteUart<uart_register_>> SpriteUartLock;

				//Sprite用Task参照
				hmr::task::host_interface* pTask;
			public:
				//Statusチェック関数
				//	Powerリセット中は0xf0, それ以外なら、CommandIDを返す
				status_type status()const{ 
					return status_type(SpriteUart.status(), Sequence_power_reset);
				}
				//Idleかどうか確認（Idleでないと、命令形のメソッドは失敗する）
				bool idle()const { return SpriteUart.idle() && !static_cast<bool>(Sequence_power_reset); }

			private:
				//=== TakePicture & ReadPicture シーケンス ===
				//	takePicture
				//	readPicture
				struct sequence_take_and_readPicture{
				private:
					typedef sequence_take_and_readPicture seq_type;
					typedef xc::either< void, error_type > ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;
				private:
					//自身を覚えておく
					my_type& Ref;

					//失敗時のみerror_typeを返す
					my_promise Promise;

					//読みだしたデータの保管場所
					picture_reader* pPictureReader;

					//写真未撮影データサイズ
					unsigned int PictureSize;
				public:
					sequence_take_and_readPicture(my_type& Ref_)
						: Ref(Ref_)
						, Sequence_takePicture(*this)
						, Sequence_readPicture(*this)
						, pPictureReader(0)
						, PictureSize(0){
					}
				public:
					my_future get_future(
						sprite::commands::imagesize::type ImageSize_, 
						picture_reader& PictureReader_,
						const bytes_builder& BytesBuilder_, 
						unsigned int MaxSize_,
						unsigned int HeadSpace_,
						unsigned int FootSpace_){

						//futureが発行できなければ、空のfutureを発行
						if(!Promise.can_get_future())return my_future();

						//future発行処理
						my_future Future = Promise.get_future();

						//PictureReaderを覚えておく
						pPictureReader = &PictureReader_;

						Sequence_readPicture.initialize(BytesBuilder_, MaxSize_, HeadSpace_, FootSpace_);

						//TakePictureを要求
						Sequence_takePicture.async(ImageSize_);

						//Futureを戻す
						return Future;
					}
					void cancel(){
						Sequence_takePicture.cancel();
						Sequence_readPicture.cancel();
						throw_error(error_category::Canceled);
					}
					void task(){
						Sequence_takePicture.task();
						Sequence_readPicture.task();
					}
				private:
					//=== TakePicture シーケンス ===
					//	takePicture
					//	=> sequence_readPictureへ連絡
					struct sequence_takePicture{
						typedef xc::either< sprite::picture_info, uart_error_type > uart_ans_type;
						typedef xc::future< uart_ans_type > uart_future;
						typedef typename uart_future::canceler canceler;
					private:
						seq_type& Seq;
						canceler Canceler;
						unsigned char WatchdogCnt;
					public:
						sequence_takePicture(seq_type& Seq_) :Seq(Seq_), WatchdogCnt(0){}
						void async(sprite::commands::imagesize::type ImageSize_){
							//wdt有効化
							WatchdogCnt = 3;

							//コマンド実行
							Canceler = Seq.async_takePicture(ImageSize_).then(xc::ref(*this));
							if(!Canceler){
								WatchdogCnt = 0;
								Seq.throw_error(error_category::FailRequestUart);
							}
						}
						void operator()(uart_ans_type UartAns){
							Canceler.clear();

							//wdt無効化
							WatchdogCnt = 0;

							//エラーを返してきた場合
							if(!UartAns)Seq.throw_error(error_category::CatchUartError, UartAns.alternate());

							//撮影結果通知
							Seq.finish_take(*UartAns);
						}
						void task(){
							if(WatchdogCnt > 3) WatchdogCnt = 0;
							if(WatchdogCnt == 0)return;

							--WatchdogCnt;
							if(WatchdogCnt == 0){
								if(Canceler)Canceler();

								Seq.throw_error(error_category::TimeOut);
							}
						}
						void cancel(){
							WatchdogCnt = 0;
							if(Canceler)Canceler();
						}
					}Sequence_takePicture;

					//=== Read Picture シーケンス ===
					//	readPicture
					struct sequence_readPicture{
						typedef xc::either< sprite::picture_data, uart_error_type > uart_ans_type;
						typedef xc::future< uart_ans_type > uart_future;
						typedef typename uart_future::canceler canceler;
					private:
						seq_type& Seq;
						canceler Canceler;
						unsigned char WatchdogCnt;

						bytes Bytes;

						//通知関数
						bytes_builder BytesBuilder;
						unsigned int MaxSize;
						unsigned int HeadSpace;
						unsigned int FootSpace;
						unsigned int RequestingDataSize;	//bytes build リクエスト中のサイズ 0なら、なし
					public:
						void initialize(const bytes_builder& BytesBuilder_, unsigned int MaxSize_, unsigned int HeadSpace_, unsigned int FootSpace_){
							BytesBuilder = BytesBuilder_;
							MaxSize = MaxSize_;
							HeadSpace = HeadSpace_;
							FootSpace = FootSpace_;
						}
						bool is_initialized()const{ return BytesBuilder; }
					public:
						sequence_readPicture(seq_type& Seq_) 
							: Seq(Seq_)
							, WatchdogCnt(0)
							, MaxSize(0)
							, HeadSpace(0)
							, FootSpace(0)
							, RequestingDataSize(0){
						}
						void async(unsigned int RequestDataSize_){
							//写真撮影サイズを確定
							if(RequestDataSize_ + HeadSpace + FootSpace> MaxSize){
								RequestDataSize_ = MaxSize - HeadSpace - FootSpace;
								RequestDataSize_ -= (RequestDataSize_ % 8);
							}

							//Bytesを確保
							BytesBuilder(Bytes, RequestDataSize_ + HeadSpace + FootSpace);
							if(Bytes.empty()){
								WatchdogCnt = 0;
								RequestingDataSize = RequestDataSize_;
								return;
							}
							RequestingDataSize = 0;

							//wdt有効化
							WatchdogCnt = 7;

							//コマンド実行
							Canceler = Seq.async_readPicture(Bytes.begin() + HeadSpace, RequestDataSize_).then(xc::ref(*this));
							if(!Canceler){
								WatchdogCnt = 0;
								Seq.throw_error(error_category::FailRequestUart);
							}
						}
						void operator()(uart_ans_type UartAns){
							Canceler.clear();

							//wdt無効化
							WatchdogCnt = 0;

							//エラーを返してきた場合
							if(!UartAns)Seq.throw_error(error_category::CatchUartError, UartAns.alternate());

							//撮影結果通知
							Seq.finish_read(*UartAns, xc::move(Bytes));
						}
						void task(){
							if(RequestingDataSize){
								async(RequestingDataSize);
							}

							if(WatchdogCnt > 7) WatchdogCnt = 0;
							if(WatchdogCnt == 0)return;

							--WatchdogCnt;
							if(WatchdogCnt == 0){
								if(Canceler)Canceler();

								Seq.throw_error(error_category::TimeOut);
							}
						}
						void cancel(){
							WatchdogCnt = 0;
							if(Canceler)Canceler();
						}
					}Sequence_readPicture;

				private:
					void throw_error(error_category_type ErrorCategory_){
						if(Promise.is_wait_value()){
							pPictureReader = 0;
							Promise.set_value(error_type(ErrorCategory_, Ref.status()));
						}
					}
					void throw_error(error_category_type ErrorCategory_, uart_error_type UartError_){
						if(Promise.is_wait_value()){
							pPictureReader = 0;
							Promise.set_value(error_type(ErrorCategory_, UartError_));
						}
					}
					typename sequence_takePicture::uart_future async_takePicture(sprite::commands::imagesize::type ImageSize_){
						return Ref.SpriteUart.async_takePicture(ImageSize_);
					}
					typename sequence_readPicture::uart_future async_readPicture(unsigned char* BufPtr_, unsigned int BufSize_){
						return Ref.SpriteUart.async_readPicture(BufPtr_, BufSize_);
					}
					void finish_take(sprite::picture_info PictureInfo_){
						pPictureReader->take(PictureInfo_);

						PictureSize = PictureInfo_.PictureSize;
						Sequence_readPicture.async(PictureSize);
					}
					void finish_read(sprite::picture_data PictureData_, xc::rvalue_reference<xc::bytes> rrBytes_){
						pPictureReader->read(PictureData_, xc::move(rrBytes_));

						//未受診のデータがまだ存在する場合
						if(PictureData_.PicturePos + PictureData_.DataSize < PictureSize){
							//次のデータ受信予約
							Sequence_readPicture.async(PictureSize - (PictureData_.PicturePos + PictureData_.DataSize));
							return;
						}

						//全データ撮影済みの時
						pPictureReader = 0;
						PictureSize = 0;

						if(Promise.is_wait_value()){
							//最後のデータなのにLastフラグが付いていなかった場合
							if(!PictureData_.IsLast){
								Promise.set_value(error_type(error_category::InvalidReadEnd, Ref.status()));
								return;
							}

							Promise.set_value(ans_type());
						}
					}
				}Sequence_take_and_readPicture;
				
				//=== Command Reset シーケンス ===
				//	command_reset
				struct sequence_command_reset{
				private:
					typedef xc::either< void, error_type > ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;

					typedef xc::either< void, uart_error_type > uart_ans_type;
					typedef xc::future< uart_ans_type > uart_future;
					typedef typename uart_future::canceler uart_canceler;
				private:
					//自身を覚えておく
					my_type& Ref;

					//失敗時のみerror_typeを返す
					my_promise Promise;

					//キャンセラー
					uart_canceler Canceler;

					unsigned int WatchdogCnt;
				public:
					sequence_command_reset(my_type& Ref_) :Ref(Ref_), WatchdogCnt(0){}
				public:
					my_future get_future(){
						//futureが発行できなければ、空のfutureを発行
						if(!Promise.can_get_future())return my_future();

						//future発行処理
						my_future Future = Promise.get_future();

						//wdt有効化
						WatchdogCnt = 3;

						//コマンド実行
						Canceler = Ref.SpriteUart.async_command_reset().then(xc::ref(*this));
						if(!Canceler){
							WatchdogCnt = 0;
							if(Promise.is_wait_value())Promise.set_value(error_type(error_category::FailRequestUart, Ref.status()));
							//Seq.throw_error(error_category::FailRequestUart);
						}

						return Future;
					}
					void operator()(uart_ans_type UartAns){
						Canceler.clear();

						//wdt無効化
						WatchdogCnt = 0;

						if(Promise.is_wait_value()){
						//エラーを返してきた場合
							if(!UartAns){
								Promise.set_value(error_type(error_category::CatchUartError, UartAns.alternate()));
								return;
							}

						//Promise履行
							Promise.set_value(ans_type());
						}
					}
					void task(){
						if(WatchdogCnt > 3) WatchdogCnt = 0;
						if(WatchdogCnt == 0)return;

						--WatchdogCnt;
						if(WatchdogCnt == 0){
							if(Canceler)Canceler();

							Promise.set_value(error_type(error_category::TimeOut, Ref.status()));
						}
					}
					void cancel(){
						WatchdogCnt = 0;
						if(Canceler)Canceler();

						if(Promise.is_wait_value()){
							Promise.set_value(error_type(error_category::Canceled, Ref.status()));
						}
					}
				}Sequence_command_reset;
				
				//=== Power Reset シーケンス ===
				struct sequence_power_reset{
				private:
					typedef xc::either< void, error_type > ans_type;
					typedef xc::promise<ans_type> my_promise;
					typedef typename my_promise::my_future my_future;
				private:
					//自身を覚えておく
					my_type& Ref;

					//Spriteの電源制御
					power_pin_ PowerPin;
					xc::lock_guard<power_pin_> PowerPinLock;

					//失敗時のみerror_typeを返す
					my_promise Promise;

					//Powerリセット用カウンタ
					unsigned int PowerResetCnt;
				public:
					sequence_power_reset(my_type& Ref_)
						: Ref(Ref_)
						, PowerResetCnt(0)
						, PowerPin()
						, PowerPinLock(PowerPin){
					}
					my_future get_future(){
						//futureが発行できなければ、空のfutureを発行
						if(!Promise.can_get_future())return my_future();
						//すでにAutoReset中なら無視
						if(PowerResetCnt>0)return my_future();

						//future発行処理
						my_future Future = Promise.get_future();

						//全キャンセル
						Ref.Sequence_take_and_readPicture.cancel();
						Ref.Sequence_command_reset.cancel();
						
						//電源オフ
						Ref.SpriteUartLock.unlock();
						PowerPin(false);

						//5秒後に再起動を予約
						PowerResetCnt=5;

						//future発行
						return Promise.get_future();
					}
					void cancel(){
						if(PowerResetCnt > 0){
							//リセットカウンタ初期化
							PowerResetCnt = 1;

							//電源回復、モジュール初期化
							PowerPin(true);
						}
						if(Promise.is_wait_value()){
							//Promiseを履行
							Promise.set_value(error_type(error_category::Canceled,Ref.status()));
						}
					}
					void task(){
						//AutoReset中
						if(PowerResetCnt>0){
							//PowerResetのカウント
							--PowerResetCnt;

							//1秒前に電源回復
							if(PowerResetCnt == 1){
								//電源回復、モジュール初期化
								PowerPin(true);
							}//Sprite設定
							else if(PowerResetCnt ==0){
								//ロック
								Ref.SpriteUartLock.lock();

								//Promiseを履行
								if(Promise.is_wait_value()){
									Promise.set_value(ans_type());
								}
							}
						}
					}
					operator bool()const{ return PowerResetCnt>0; }
				public:
					void power_on(){
						//電源ON
						PowerPin(true);

						PowerResetCnt = 2;
					}
					void power_off(){
						//電源OFF
						PowerResetCnt = 0;
						PowerPin(0);
					}
				}Sequence_power_reset;

				//====== Task ======
				struct sprite_task :public task::client_interface{
				private:
					my_type& Ref;
				public:
					sprite_task(my_type& Ref_):Ref(Ref_){}
					duration operator()(duration Duration_){
						Ref.Sequence_take_and_readPicture.task();
						Ref.Sequence_command_reset.task();
						Ref.Sequence_power_reset.task();
						return 1;
					}
				}SpriteTask;

			public:
				cSprite()
					: IsLock(false)
					, SpriteUart()
					, SpriteUartLock(SpriteUart,true)
					, Sequence_take_and_readPicture(*this)
					, Sequence_command_reset(*this)
					, Sequence_power_reset(*this)
					, SpriteTask(*this){
				}
			public:
				void config(hmr::task::host_interface& Task_){
					pTask = &Task_;
				}
				bool lock(hmr::task::host_interface& Task_){
					config(Task_);
					return lock();
				}
				bool lock(){
					if (is_lock())return false;

					//タスクスタート
					pTask->start(SpriteTask, 2, 0);

					//パワーリセットを介して、パワー制御開始
					Sequence_power_reset.power_on();

					IsLock = true;

					return false;
				}
				void unlock(){
					if(!is_lock())return;

					//タスクストップ
					pTask->stop(SpriteTask);

					//実行中シーケンスがあれば、キャンセル
					Sequence_take_and_readPicture.cancel();
					Sequence_command_reset.cancel();
					Sequence_power_reset.cancel();

					Sequence_power_reset.power_off();

					SpriteUartLock.unlock();

					IsLock = false;
				}
				bool is_lock()const{ return IsLock; }
			public:
				//写真撮影
				//	return:撮影したPictureのpicture_info
				xc::future< xc::either<void, error_type> > async_take_and_readPicture(
					sprite::commands::imagesize::type ImageSize_, 
					picture_reader& PictureReader_,
					const bytes_builder& BytesBuilder_,
					unsigned int MaxSize_, 
					unsigned int HeadSpace_ = 0,
					unsigned int FootSpace_ = 0){
					if(Sequence_power_reset)return xc::future< xc::either<void, error_type> >();
					return Sequence_take_and_readPicture.get_future(ImageSize_, PictureReader_, BytesBuilder_, MaxSize_, HeadSpace_, FootSpace_);
				}
				//リセット要求
				//	return:失敗フラグ
				xc::future< xc::either<void, error_type> > async_command_reset(){
					if(Sequence_power_reset)return xc::future< xc::either<void, error_type> >();
					return Sequence_command_reset.get_future();
				}
				//パワーリセット要求
				//	return:失敗フラグ
				xc::future< xc::either<void, error_type> > async_power_reset(){ return Sequence_power_reset.get_future(); }
			};
		}
	}
}
#
#endif
