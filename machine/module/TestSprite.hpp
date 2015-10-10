#ifndef HMR_MACHINE_MODULE_SPRITE_INC
#define HMR_MACHINE_MODULE_SPRITE_INC 100
#
#include<XCBase/bytes.hpp>
#include<XCBase/future.hpp>
#include<XCBase/either.hpp>
#include<XC32/uart.hpp>
#include<homuraLib_v2/task.hpp>
#include<homuraLib_v2/machine/service/task.hpp>
#include"Sprite/command.hpp"
#include"Sprite/command_uart.hpp"
//Sprite.hpp��future/promise��int�ɒu���������肵��future��ˑ��ɂ�������
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
			//��A�̃R�}���h���V�[�P���X�����Ďg���₷�������ASprite�pUart
			//	�^�C���A�E�g�A�d���Ǘ��A�I�[�g���Z�b�g���͊Ǘ����Ă��Ȃ��̂Œ���
			template<typename uart_register_>
			class cSpriteUart {
				friend class testSpriteUart;
				typedef cSpriteUart<uart_register_> my_type;
			private:
				//Sprite�pUart
				sprite::command_uart<uart_register_> CommandUart;
				xc::unique_lock<sprite::command_uart<uart_register_>> CommandUartLock;
			public:
				typedef sprite::command_uart_status status_type;
				//Status�`�F�b�N�֐�
				status_type status()const{ return CommandUart.status(); }
				//Idle���ǂ����m�F�iIdle�łȂ��ƁA���ߌ`�̃��\�b�h�͎��s����j
				bool idle()const { return !CommandUart.is_command(); }

				//======�G���[�Ǘ�======
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
				//�J�����̉摜�T�C�Y
				sprite::commands::imagesize::type ImageSize;
				//�ʐ^�̃f�[�^�T�C�Y
				unsigned int DataSize;
				//�ʐ^�̃f�[�^�ǂݍ��݃J�E���^
				unsigned int DataCnt;
			private:
				//=== TakePicture�V�[�P���X ===
				//	setImageSize(ImageSize��ύX���̂�) 
				//	=> takePicture
				//	=> getDataSize
				//async_takePicture�pPromise
				struct sequence_takePicture{
					friend class testSpriteUart;
				public:
					typedef xc::either<sprite::picture_info, error_type> ans_type;
					//typedef xc::promise<ans_type> my_promise;
					//typedef typename my_promise::my_future my_future;
					typedef int my_future;
				private:
					my_type& Ref;
//					my_promise Promise;
					//xc::future<bool>::canceler Canceler;
					sprite::commands::ans_getDataSize Ans_getDataSize;
				public:
					sequence_takePicture(my_type& Ref_) :Ref(Ref_){}
					my_future  get_future(sprite::commands::imagesize::type ImageSize_){
						//Promise�����݂��g��ꂽ�܂܂̏ꍇ�͎��s
						//if(!Promise.can_get_future())return my_future();

						//future���s �񓯊������J�n
						my_future Future = 0;//Promise.get_future();

						//ImageSize_���`�F�b�N �ُ�l�Ȃ�160_120�ɐݒ�
						if (ImageSize_ != sprite::commands::imagesize::size_640_480 && ImageSize_ != sprite::commands::imagesize::size_320_240){
							ImageSize_ = sprite::commands::imagesize::size_160_120;
						}

						//���݂̃J������ImageSize�Ɠ���Ȃ�takePicture�A�Ⴆ�΂܂�getDataSize
						if (Ref.ImageSize != ImageSize_){
							//ImaseSize���قȂ�ꍇ�́AsetImageSize����
							Ref.ImageSize = ImageSize_;

							//setImageSize��񓯊����s & takePicture���s��\��
//							Canceler = Ref.CommandUart.async_command(sprite::commands::setImageSize(ImageSize_)).then(xc::ref(*this));
//							if(!Canceler){
//								Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
//							}
						}
						else{
							//takePicture��񓯊����s & getDataSize���s��\��
//							Canceler = Ref.CommandUart.async_command(sprite::commands::takePicture()).then(xc::ref(*this));
//							if(!Canceler){
//								Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
//							}
						}

						//future���s
						//	then_getDataSize���s���ɗ��s
						return Future;
					}
					void operator()(bool IsFail_){
						//������then���Ă΂ꂽ�̂ŁA�܂��o���Ă���canceler������
//						Canceler.clear();

						if (IsFail_){
//							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						switch(Ref.CommandUart.status().id()) {
						case sprite::commands::id::SetImageSize:
							//takePicture��񓯊����s & getDataSize���s��\��
//							Canceler = Ref.CommandUart.async_command(sprite::commands::takePicture()).then(xc::ref(*this));

//							if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return;
						case sprite::commands::id::TakePicture:
							//getDataSize��񓯊����s & TakePicture�V�[�P���X�̌�n�����s��\��
//							Canceler = Ref.CommandUart.async_command(sprite::commands::getDataSize(Ans_getDataSize)).then(xc::ref(*th////is));

//							if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return;
						case sprite::commands::id::GetDataSize:
							//Ans�̃f�[�^���L�^���Ă���
							Ref.DataSize = Ans_getDataSize.Size;

							//Promise���s
//							Promise.set_value(sprite::picture_info(Ref.DataSize, Ref.ImageSize));
							return;
						default:
//							Promise.set_value(error_type(error_category::InvalidCommand, Ref.CommandUart.status()));
							return;
						}

					}
					void cancel(){
						//���ݕێ����Ă���ꍇ�́Acancel�������s
//						if(Canceler)Canceler();

						//����get_future���Ȃ�A������
//						if(Promise.valid())Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
					}
				}Sequence_takePicture;
			private:
				//=== ReadPicture�V�[�P���X ===
				//	getData
				//	=> stop_takePicture (�ŏI�f�[�^���̂�)
				struct sequence_readPicture{
					friend class testSpriteUart;
				public:
					typedef xc::either<sprite::picture_data, error_type> ans_type;
//					typedef xc::promise<ans_type> my_promise;
//					typedef typename my_promise::my_future my_future;
					typedef int my_future;
				private:
					my_type& Ref;
//					my_promise Promise;
//					xc::future<bool>::canceler Canceler;
					sprite::commands::ans_getData Ans_getData;
					//�������݃o�b�t�@�̐擪�A�h���X
					unsigned char* BufPtr;
					unsigned int DataPos;
					unsigned int DataSize;
				private:
					sequence_readPicture(const sequence_readPicture&);
				public:
					sequence_readPicture(my_type& Ref_) :Ref(Ref_){}
					my_future get_future(unsigned char* BufPtr_, unsigned int BufSize_){
						//�ȉ��̏ꍇ�͎��s
						//	Promise�����݂��g��ꂽ�܂�
//						if(!Promise.can_get_future())return my_future();

						//future���s �񓯊������J�n
						my_future Future = 0;//Promise.get_future();

						//�o�b�t�@�擪�A�h���X���L�^
						BufPtr = BufPtr_;

						//�c�f�[�^�����o�b�t�@�̂ق����������Ƃ�
						if(BufSize_ < Ref.DataSize - Ref.DataCnt){
							//8�̔{���Ɋۂߍ���
							BufSize_ -= BufSize_ % 8;
						} else{
							//�c�f�[�^��S���ǂ�
							BufSize_ = Ref.DataSize - Ref.DataCnt;
						}

						//�o�b�t�@�ւ̏������ݓo�^
						Ans_getData.Itr = BufPtr_;
						Ans_getData.End = BufPtr_ + BufSize_;

						//setImageSize��񓯊����s & takePicture���s��\��
//						Canceler = Ref.CommandUart.async_command(sprite::commands::getData(Ref.DataCnt, BufSize_, Ans_getData)).then(xc::ref(*this));
//						if(!Canceler){
//							Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
							return Future;
					}
					void operator()(bool IsFail_){
						//������then���Ă΂ꂽ�̂ŁA�܂��o���Ă���canceler������
//						Canceler.clear();

						if(IsFail_){
							Ref.DataCnt = 0;
							Ref.DataSize = 0;

//							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						switch(Ref.CommandUart.status().id()){
						case sprite::commands::id::GetData:
							//���݈ʒu���L�^
							DataPos = Ref.DataCnt;
							DataSize = Ans_getData.Itr - BufPtr;

							//���ʂ��L�^
							Ref.DataCnt += DataSize;

							//�f�[�^���Ō�Ȃ�
							if (Ref.DataCnt >= Ref.DataSize){
								//end_takePicture��񓯊����s & ���̌�n�����s��\��
//								Canceler = Ref.CommandUart.async_command(sprite::commands::stop_takePicture()).then(xc::ref(*this));

//								if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));
								return;

							}

							//Promise���s
//							Promise.set_value(sprite::picture_data(DataPos, DataSize, false));
							return;
						case sprite::commands::id::StopTakePicture:
							Ref.DataCnt = 0;
							Ref.DataSize = 0;

							//Promise���s
//							Promise.set_value(sprite::picture_data(DataPos, DataSize, true));
							return;
						default:
							Ref.DataCnt = 0;
							Ref.DataSize = 0;
//							Promise.set_value(error_type(error_category::InvalidCommand, Ref.CommandUart.status()));
							return;
						}
					}
					void cancel(){
						//���ݕێ����Ă���ꍇ�́Acancel�������s
//						if (Canceler)Canceler();

						//����get_future���Ȃ�A������
//						if (Promise.valid()){
//							Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
//						}
					}
				}Sequence_readPicture;
			private:
				//=== Command Reset �V�[�P���X ===
				//	command_reset
				struct sequence_command_reset{
					friend class testSpriteUart;
				public:
					typedef xc::either<void, error_type> ans_type;
//					typedef xc::promise<ans_type> my_promise;
//					typedef typename my_promise::my_future my_future;
					typedef int my_future;
				private:
					my_type& Ref;
//					my_promise Promise;
//					xc::future<bool>::canceler Canceler;
				public:
					sequence_command_reset(my_type& Ref_) :Ref(Ref_){}
					my_future get_future(){
						//Promise�����݂��g��ꂽ�܂܂̏ꍇ�͎��s
//						if(!Promise.can_get_future())return my_future();

						//future���s �񓯊������J�n
						my_future Future = 0;//Promise.get_future();

						//setImageSize��񓯊����s & takePicture���s��\��
//						Canceler = Ref.CommandUart.async_command(sprite::commands::resetCamera()).then(xc::ref(*this));
//						if(!Canceler)Promise.set_value(error_type(error_category::FailRequestCommand, Ref.CommandUart.status()));

						return Future;
					}
					void operator()(bool IsFail_){
						//������then���Ă΂ꂽ�̂ŁA�܂��o���Ă���canceler������
//						Canceler.clear();

						if(IsFail_){
//							Promise.set_value(error_type(error_category::FailCommand, Ref.CommandUart.status()));
							return;
						}

						//Promise���s
//						Promise.set_value(ans_type());
					}
					void cancel(){
						//���ݕێ����Ă���ꍇ�́Acancel�������s
//						if (Canceler)Canceler();

						//����get_future���Ȃ�A������
//						if(Promise.valid())Promise.set_value(error_type(error_category::Canceled, Ref.CommandUart.status()));
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

					//�J�����̉摜�T�C�Y
					ImageSize=sprite::commands::imagesize::size_160_120;
					//�ʐ^�̃f�[�^�T�C�Y
					DataSize = 0;
					//�ʐ^�̃f�[�^�ǂݍ��݃J�E���^
					DataCnt = 0;

					return false;
				}
				void unlock(){
					//�������V�[�P���X���L�����Z��
					Sequence_takePicture.cancel();
					Sequence_readPicture.cancel();
					Sequence_command_reset.cancel();
					//���b�N����
					CommandUartLock.unlock();
				}
				bool is_lock()const{ return CommandUartLock; }
			public:
				//�ʐ^�B�e
				//	return:�B�e����Picture��picture_info
//				xc::future< xc::either<sprite::picture_info, error_type> > async_takePicture(sprite::commands::imagesize::type ImageSize_){
				int async_takePicture(sprite::commands::imagesize::type ImageSize_){
					//�ȉ��̏ꍇ�͎��s
					//	�A�C�h�����=���ߎ�t�\�łȂ�
					//	CommandUart�����݂��g��ꂽ�܂�
					//	���ǂݍ���Picture�c����
					if (!idle()
						|| CommandUart.is_command()
						|| can_readPicture()){
						return 0;//xc::future< xc::either<sprite::picture_info, error_type> >();
					}

					return Sequence_takePicture.get_future(ImageSize_);
				}
				//�ʐ^�f�[�^�����ݑ��݂��Ă��邩
				//	return:readPicture�\�ȎB�e�ς�Picture������?
				bool can_readPicture()const{ return DataCnt < DataSize; }
				//�ʐ^�f�[�^�ǂݏo��
				//	return:�擾����Data��picture_data
//				xc::future< xc::either<sprite::picture_data, error_type> > async_readPicture(unsigned char* BufPtr_, unsigned int BufSize_){
				int async_readPicture(unsigned char* BufPtr_, unsigned int BufSize_){
					//�ȉ��̏ꍇ�͎��s
					//	�A�C�h�����=���ߎ�t�\�łȂ�
					//	CommandUart�����݂��g��ꂽ�܂�
					//	�ǂݍ���Picture�����݂��Ȃ�
					//	Promise_readPicture�����݂��g��ꂽ�܂�
					if (!idle()
						|| CommandUart.is_command()
						|| !can_readPicture()){
						return 0;//xc::future< xc::either<sprite::picture_data, error_type> >();
					}

					return Sequence_readPicture.get_future(BufPtr_, BufSize_);
				}
				//���Z�b�g�v��
				//	return:���s�t���O
				//xc::future< xc::either<void, error_type> > async_command_reset(){
				int async_command_reset(){
					//�ȉ��̏ꍇ�͎��s
					//	�A�C�h�����=���ߎ�t�\�łȂ�
					//	CommandUart�����݂��g��ꂽ�܂�
					//	Promise_command_reset�����݂��g��ꂽ�܂�
					if (!idle()
						|| CommandUart.is_command()){
						return 0;//xc::future< xc::either<void, error_type> >();
					}

					return Sequence_command_reset.get_future();
				}
			};
			
			//�d���Ǘ��A�^�C���A�E�g�@�\�A�I�[�g���Z�b�g�@�\�t���̍��@�\Sprite�Ǘ��N���X
			template<typename uart_register_, typename power_pin_>
			class cSprite {
				friend class testSprite;
			public:
				typedef cSprite<uart_register_, power_pin_> my_type;
				typedef cSpriteUart<uart_register_> my_uart;
				typedef typename my_uart::status_type uart_status_type;
			public:
				//===== �ʒm�p�֐��^ =====
				typedef xc::bytes bytes;
				typedef typename bytes::size_type size_type;
				typedef xc::function<void(bytes&,size_type)> bytes_builder;
				struct picture_reader{
					virtual void take(sprite::picture_info PictureInfo_) = 0;
					virtual void read(sprite::picture_data PictureData_, xc::rvalue_reference<bytes> Data_) = 0;
				};
			public:
				//===== �X�e�[�^�X =====
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
				//======�G���[�Ǘ�======
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

				//Sprite�pUart
				my_uart SpriteUart;
				xc::unique_lock<cSpriteUart<uart_register_>> SpriteUartLock;

			public:
				//Status�`�F�b�N�֐�
				//	Power���Z�b�g����0xf0, ����ȊO�Ȃ�ACommandID��Ԃ�
				status_type status()const{ 
					return status_type(SpriteUart.status(), Sequence_power_reset);
				}
				//Idle���ǂ����m�F�iIdle�łȂ��ƁA���ߌ`�̃��\�b�h�͎��s����j
				bool idle()const { return SpriteUart.idle() && !static_cast<bool>(Sequence_power_reset); }

			private:
				//=== TakePicture & ReadPicture �V�[�P���X ===
				//	takePicture
				//	readPicture
				struct sequence_take_and_readPicture{
				private:
					typedef sequence_take_and_readPicture seq_type;
					typedef xc::either< void, error_type > ans_type;
//					typedef xc::promise<ans_type> my_promise;
//					typedef typename my_promise::my_future my_future;
					typedef int my_future;
				private:
					//���g���o���Ă���
					my_type& Ref;

					//���s���̂�error_type��Ԃ�
//					my_promise Promise;

					//�ǂ݂������f�[�^�̕ۊǏꏊ
					picture_reader* pPictureReader;

					//�ʐ^���B�e�f�[�^�T�C�Y
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

						//future�����s�ł��Ȃ���΁A���future�𔭍s
						//if(!Promise.can_get_future())return my_future();

						//future���s����
						my_future Future = 0;//Promise.get_future();

						//PictureReader���o���Ă���
						pPictureReader = &PictureReader_;

						Sequence_readPicture.initialize(BytesBuilder_, MaxSize_, HeadSpace_, FootSpace_);

						//TakePicture��v��
						Sequence_takePicture.async(ImageSize_);

						//Future��߂�
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
					//=== TakePicture �V�[�P���X ===
					//	takePicture
					//	=> sequence_readPicture�֘A��
					struct sequence_takePicture{
						typedef xc::either< sprite::picture_info, uart_error_type > uart_ans_type;
//						typedef xc::future< uart_ans_type > uart_future;
						typedef int uart_future;
//						typedef typename uart_future::canceler canceler;
					private:
						seq_type& Seq;
//						canceler Canceler;
						unsigned char WatchdogCnt;
					public:
						sequence_takePicture(seq_type& Seq_) :Seq(Seq_), WatchdogCnt(0){}
						void async(sprite::commands::imagesize::type ImageSize_){
							//wdt�L����
							WatchdogCnt = 3;

							//�R�}���h���s
//							Canceler = Seq.async_takePicture(ImageSize_).then(xc::ref(*this));
							Seq.async_takePicture(ImageSize_);

//							if(!Canceler){
//								WatchdogCnt = 0;
								Seq.throw_error(error_category::FailRequestUart);
//							}
						}
						void operator()(uart_ans_type UartAns){
//							Canceler.clear();

							//wdt������
							WatchdogCnt = 0;

							//�G���[��Ԃ��Ă����ꍇ
							if(!UartAns)Seq.throw_error(error_category::CatchUartError, UartAns.alternate());

							//�B�e���ʒʒm
							Seq.finish_take(*UartAns);
						}
						void task(){
							if(WatchdogCnt > 3) WatchdogCnt = 0;
							if(WatchdogCnt == 0)return;

							--WatchdogCnt;
							if(WatchdogCnt == 0){
//								if(Canceler)Canceler();

								Seq.throw_error(error_category::TimeOut);
							}
						}
						void cancel(){
							WatchdogCnt = 0;
//							if(Canceler)Canceler();
						}
					}Sequence_takePicture;

					//=== Read Picture �V�[�P���X ===
					//	readPicture
					struct sequence_readPicture{
						typedef xc::either< sprite::picture_data, uart_error_type > uart_ans_type;
//						typedef xc::future< uart_ans_type > uart_future;
						typedef int uart_future;
//						typedef typename uart_future::canceler canceler;
					private:
						seq_type& Seq;
//						canceler Canceler;
						unsigned char WatchdogCnt;

						bytes Bytes;

						//�ʒm�֐�
						bytes_builder BytesBuilder;
						unsigned int MaxSize;
						unsigned int HeadSpace;
						unsigned int FootSpace;
						unsigned int RequestingDataSize;	//bytes build ���N�G�X�g���̃T�C�Y 0�Ȃ�A�Ȃ�
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
							//�ʐ^�B�e�T�C�Y���m��
							if(RequestDataSize_ + HeadSpace + FootSpace> MaxSize){
								RequestDataSize_ = MaxSize - HeadSpace - FootSpace;
								RequestDataSize_ -= (RequestDataSize_ % 8);
							}

							//Bytes���m��
							BytesBuilder(Bytes, RequestDataSize_ + HeadSpace + FootSpace);
							if(Bytes.empty()){
								WatchdogCnt = 0;
								RequestingDataSize = RequestDataSize_;
								return;
							}
							RequestingDataSize = 0;

							//wdt�L����
							WatchdogCnt = 7;

							//�R�}���h���s
//							Canceler = Seq.async_readPicture(Bytes.begin() + HeadSpace, RequestDataSize_).then(xc::ref(*this));
							Seq.async_readPicture(Bytes.begin() + HeadSpace, RequestDataSize_);

//							if(!Canceler){
								WatchdogCnt = 0;
								Seq.throw_error(error_category::FailRequestUart);
//							}
						}
						void operator()(uart_ans_type UartAns){
//							Canceler.clear();

							//wdt������
							WatchdogCnt = 0;

							//�G���[��Ԃ��Ă����ꍇ
							if(!UartAns)Seq.throw_error(error_category::CatchUartError, UartAns.alternate());

							//�B�e���ʒʒm
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
//								if(Canceler)Canceler();

								Seq.throw_error(error_category::TimeOut);
							}
						}
						void cancel(){
							WatchdogCnt = 0;
//							if(Canceler)Canceler();
						}
					}Sequence_readPicture;

				private:
					void throw_error(error_category_type ErrorCategory_){
//						if(Promise.valid()){
							pPictureReader = 0;
//							Promise.set_value(error_type(ErrorCategory_, Ref.status()));
//						}
					}
					void throw_error(error_category_type ErrorCategory_, uart_error_type UartError_){
//						if(Promise.valid()){
							pPictureReader = 0;
//							Promise.set_value(error_type(ErrorCategory_, UartError_));
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

						//����f�̃f�[�^���܂����݂���ꍇ
						if(PictureData_.PicturePos + PictureData_.DataSize < PictureSize){
							//���̃f�[�^��M�\��
							Sequence_readPicture.async(PictureSize - (PictureData_.PicturePos + PictureData_.DataSize));
							return;
						}

						//�S�f�[�^�B�e�ς݂̎�
						pPictureReader = 0;
						PictureSize = 0;

//						if(Promise.valid()){
							//�Ō�̃f�[�^�Ȃ̂�Last�t���O���t���Ă��Ȃ������ꍇ
							if(!PictureData_.IsLast){
//								Promise.set_value(error_type(error_category::InvalidReadEnd, Ref.status()));
								return;
							}

//							Promise.set_value(ans_type());
//						}
					}
				}Sequence_take_and_readPicture;
				
				//=== Command Reset �V�[�P���X ===
				//	command_reset
				struct sequence_command_reset{
				private:
					typedef xc::either< void, error_type > ans_type;
//					typedef xc::promise<ans_type> my_promise;
//					typedef typename my_promise::my_future my_future;
					typedef int my_future;

					typedef xc::either< void, uart_error_type > uart_ans_type;
//					typedef xc::future< uart_ans_type > uart_future;
//					typedef typename uart_future::canceler uart_canceler;
					typedef int uart_future;
				private:
					//���g���o���Ă���
					my_type& Ref;

					//���s���̂�error_type��Ԃ�
//					my_promise Promise;

					//�L�����Z���[
//					uart_canceler Canceler;

					unsigned int WatchdogCnt;
				public:
					sequence_command_reset(my_type& Ref_) :Ref(Ref_), WatchdogCnt(0){}
				public:
					my_future get_future(){
						//future�����s�ł��Ȃ���΁A���future�𔭍s
//						if(!Promise.can_get_future())return my_future();

						//future���s����
						my_future Future = 0;//Promise.get_future();

						//wdt�L����
						WatchdogCnt = 3;

						//�R�}���h���s
//						Canceler = Ref.SpriteUart.async_command_reset().then(xc::ref(*this));
						Ref.SpriteUart.async_command_reset();

//						if(!Canceler){
							WatchdogCnt = 0;
//							if(Promise.valid())Promise.set_value(error_type(error_category::FailRequestUart, Ref.status()));
							//Seq.throw_error(error_category::FailRequestUart);
//						}

						return Future;
					}
					void operator()(uart_ans_type UartAns){
//						Canceler.clear();

						//wdt������
						WatchdogCnt = 0;

//						if(Promise.valid()){
						//�G���[��Ԃ��Ă����ꍇ
							if(!UartAns){
//								Promise.set_value(error_type(error_category::CatchUartError, UartAns.alternate()));
								return;
							}

						//Promise���s
//							Promise.set_value(ans_type());
//						}
					}
					void task(){
						if(WatchdogCnt > 3) WatchdogCnt = 0;
						if(WatchdogCnt == 0)return;

						--WatchdogCnt;
						if(WatchdogCnt == 0){
//							if(Canceler)Canceler();

//							Promise.set_value(error_type(error_category::TimeOut, Ref.status()));
						}
					}
					void cancel(){
						WatchdogCnt = 0;
//						if(Canceler)Canceler();

//						if(Promise.valid()){
//							Promise.set_value(error_type(error_category::Canceled, Ref.status()));
//						}
					}
				}Sequence_command_reset;
				
				//=== Power Reset �V�[�P���X ===
				struct sequence_power_reset{
				private:
					typedef xc::either< void, error_type > ans_type;
//					typedef xc::promise<ans_type> my_promise;
//					typedef typename my_promise::my_future my_future;
					typedef int my_future;
				private:
					//���g���o���Ă���
					my_type& Ref;

					//Sprite�̓d������
					power_pin_ PowerPin;
					xc::lock_guard<power_pin_> PowerPinLock;

					//���s���̂�error_type��Ԃ�
//					my_promise Promise;

					//Power���Z�b�g�p�J�E���^
					unsigned int PowerResetCnt;
				public:
					sequence_power_reset(my_type& Ref_)
						: Ref(Ref_)
						, PowerResetCnt(0)
						, PowerPin()
						, PowerPinLock(PowerPin){
					}
					my_future get_future(){
						//future�����s�ł��Ȃ���΁A���future�𔭍s
//						if(!Promise.can_get_future())return my_future();
						//���ł�AutoReset���Ȃ疳��
						if(PowerResetCnt>0)return my_future();

						//future���s����
						my_future Future = 0;//Promise.get_future();

						//�S�L�����Z��
						Ref.Sequence_take_and_readPicture.cancel();
						Ref.Sequence_command_reset.cancel();
						
						//�d���I�t
						Ref.SpriteUartLock.unlock();
						PowerPin(false);

						//5�b��ɍċN����\��
						PowerResetCnt=5;

						//future���s
						return 0;//Promise.get_future();
					}
					void cancel(){
						if(PowerResetCnt > 0){
							//���Z�b�g�J�E���^������
							PowerResetCnt = 1;

							//�d���񕜁A���W���[��������
							PowerPin(true);
						}
//						if(Promise.valid()){
							//Promise�𗚍s
//							Promise.set_value(error_type(error_category::Canceled,Ref.status()));
//						}
					}
					void task(){
						//AutoReset��
						if(PowerResetCnt>0){
							//PowerReset�̃J�E���g
							--PowerResetCnt;

							//1�b�O�ɓd����
							if(PowerResetCnt == 1){
								//�d���񕜁A���W���[��������
								PowerPin(true);
							}//Sprite�ݒ�
							else if(PowerResetCnt ==0){
								//���b�N
								Ref.SpriteUartLock.lock();

								//Promise�𗚍s
//								if(Promise.valid()){
//									Promise.set_value(ans_type());
//								}
							}
						}
					}
					operator bool()const{ return PowerResetCnt>0; }
				public:
					void power_on(){
						//�d��ON
						PowerPin(true);

						PowerResetCnt = 2;
					}
					void power_off(){
						//�d��OFF
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
				bool lock(){
					if (is_lock())return false;

					//�^�X�N�X�^�[�g
					service::task::start(SpriteTask, 2, 0);

					//�p���[���Z�b�g����āA�p���[����J�n
					Sequence_power_reset.power_on();

					IsLock = true;

					return false;
				}
				void unlock(){
					if(!is_lock())return;

					//�^�X�N�X�g�b�v
					service::task::stop(SpriteTask);

					//���s���V�[�P���X������΁A�L�����Z��
					Sequence_take_and_readPicture.cancel();
					Sequence_command_reset.cancel();
					Sequence_power_reset.cancel();

					Sequence_power_reset.power_off();

					SpriteUartLock.unlock();

					IsLock = false;
				}
				bool is_lock()const{ return IsLock; }
			public:
				//�ʐ^�B�e
				//	return:�B�e����Picture��picture_info
				/*xc::future< xc::either<void, error_type> >*/int async_take_and_readPicture(
					sprite::commands::imagesize::type ImageSize_, 
					picture_reader& PictureReader_,
					const bytes_builder& BytesBuilder_,
					unsigned int MaxSize_, 
					unsigned int HeadSpace_ = 0,
					unsigned int FootSpace_ = 0){
					if(Sequence_power_reset)return 0;//xc::future< xc::either<void, error_type> >();
					return Sequence_take_and_readPicture.get_future(ImageSize_, PictureReader_, BytesBuilder_, MaxSize_, HeadSpace_, FootSpace_);
				}
				//���Z�b�g�v��
				//	return:���s�t���O
				/*xc::future< xc::either<void, error_type> >*/int async_command_reset(){
					if(Sequence_power_reset)return 0;//xc::future< xc::either<void, error_type> >();
					return Sequence_command_reset.get_future();
				}
				//�p���[���Z�b�g�v��
				//	return:���s�t���O
				/*xc::future< xc::either<void, error_type> >*/int async_power_reset(){ return Sequence_power_reset.get_future(); }
			};
		}
	}
}
#
#endif
