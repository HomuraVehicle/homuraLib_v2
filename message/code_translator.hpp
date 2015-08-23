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
			//code������
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
			//���łɗ��p�\�ɂȂ��Ă��邩
			virtual bool is_open() {
				return pIOs.size()>0;
			}
			//send�ł��邩�H
			virtual bool can_getc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//Data����łȂ��Ȃ�A�I��
				if(SendData==true) {
					if(SendMode==sendmode_DATA)return SendData.size() > SendCnt;
					else return 1;
				}

				//�O��C���N�������g����Ă��Ȃ��āAflowing�������Ă���΁A�C���N�������g
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				//Data��ǂݏo���Ȃ��Ȃ�I��
				if(SendIO->second->can_get()==false)return 0;

				//Data�ǂݏo�� ���s������I��
				if(SendIO->second->get(SendData))return 0;
				SendCnt=0;

				//��f�[�^�Ȃ�͂���
				if(SendData==false)return 0;

				//Data�ǂݏo���ɐ�������΁AOK
				SendIncrement=false;
				return 1;
			}
			//���M�������1byte�擾����
			virtual unsigned char getc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//Data����Ȃ�A�I��
				if(SendData==false) {
					SendMode=sendmode_ID;
					return 0xC0;
				}

				unsigned char c;
				//ID���M���[�h
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
			//���M������eof�ʒu=Pac�I�[���ǂ��������m����
			virtual bool flowing() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//�O��C���N�������g����Ă��Ȃ��āAflowing�������Ă���΁A�C���N�������g
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				return SendIO->second->flowing();
			}
			//recv�ł��邩�H(�P��return 1)
			virtual bool can_putc() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//RecvIO�������Ȃ�A�劽�}�i�������邯�ǁj
				if(RecvIO==pIOs.end())return true;

				//�G���[��Ԃ̎��ɂ́A�劽�}�i�������邯�ǁj
				if(RecvMode==recvmode_ERROR)return true;

				//�o�b�t�@������M�ł���Ȃ�OK
				return RecvIO->second->can_put();
			}
			//��M�������1byte�^����
			virtual void putc(unsigned char c) {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//RecvIO�������Ȃ�A�劽�}�i�������邯�ǁj
				if(RecvIO==pIOs.end())return;

				//�f�[�^id��M��
				if(RecvMode==recvmode_ID) {
					//NullID�͂����Ȃ��̂Ŗ���
					if(c==message::data_id::null) {
						//�Ȍ�̃f�[�^��flush������܂ŐM�����Ȃ��̂Ń��[�h���ڍs
						RecvMode=recvmode_ERROR;
						return;
					}

					//�f�[�^ID���擾
					RecvDataID=c;
					RecvDataError=error::null;
					RecvDataArray.release();

					//ID�Ɋւ���G���[�o�^
					if(data_id::is_special_id(RecvDataID)) RecvDataError|=error::unknown;

					//���[�h���T�C�Y��M���[�h(SIZE1)�ֈڍs
					RecvMode=recvmode_SIZE1;
				}//�f�[�^�T�C�Y��M��1
				else if(RecvMode==recvmode_SIZE1) {
					//�f�[�^�T�C�Y���擾
					RecvCnt=static_cast<unsigned char>(c);

					//���[�h���T�C�Y��M���[�h(SIZE2)�ֈڍs
					RecvMode=recvmode_SIZE2;
				}//�f�[�^�T�C�Y��M��2
				else if(RecvMode==recvmode_SIZE2) {
					//�f�[�^�T�C�Y���擾
					RecvCnt|=(static_cast<unsigned int>(static_cast<unsigned char>(c))<<8);

					//�f�[�^�T�C�Y���K�i����������Ă����ꍇ�͖���
					if(RecvCnt>HMR_MESSAGE_CODE_MAXSIZE) {
						//�f�[�^�𑗂�
						if(RecvIO->second->can_put()) {
							RecvDataError|=error::sizeover;
							RecvIO->second->put(data(RecvDataID, RecvDataArray, RecvDataError));
						}

						//�Ȍ�̃f�[�^��flush������܂ŐM�����Ȃ��̂Ń��[�h���ڍs
						RecvMode=recvmode_ERROR;
						return;
					}

					//�f�[�^�T�C�Y��0�̎�
					if(RecvCnt==0) {
						//�f�[�^�𑗂�
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, RecvDataArray, RecvDataError));
						}

						//���[�h��ID��M���[�h�ֈڍs
						RecvMode=recvmode_ID;
						return;
					}

					//�o�b�t�@���m��
					RecvDataArray = make_shared_array(RecvCnt);

					//�f�[�^�T�C�Y�m�ۂɎ��s�����Ƃ�
					if(RecvDataArray.size()!=RecvCnt) {
						//�m�ێ��s�G���[�o�^
						RecvDataError|=error::failnew;
						//�Ȍ�A�������݂ł��Ȃ��̂ŁA���̃f�[�^������܂Ŗ���
						RecvMode=recvmode_IGNORE;
						return;
					}

					//���[�h��Pac��M���[�h�ֈڍs
					RecvCnt=0;
					RecvMode=recvmode_DATA;
				}//�f�[�^��M��
				else if(RecvMode==recvmode_DATA) {
					//�f�[�^��������
					RecvDataArray[RecvCnt]=c;

					//�������݃T�C�Y����ɒB�����ꍇ
					if(RecvCnt==RecvDataArray.size()) {
						//�f�[�^�𑗂�
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
						}
						RecvDataArray.release();
						
						//���[�h��ID��M���[�h�ֈڍs
						RecvMode=recvmode_ID;
						return;
					}
				}//�f�[�^������
				else if(RecvMode==recvmode_IGNORE) {
					--(RecvCnt);

					//�S�f�[�^��M���I�����Ƃ�
					if(RecvCnt==0) {
						if(RecvIO->second->can_put()) {
							RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
						}
						RecvDataArray.release();

						//���[�h��ID��M�Ɉڍs
						RecvMode=recvmode_ID;
						return;
					}
				}
			}
			//flush����(eof������=Pac�����)
			virtual void flush() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//��M���̃f�[�^�����݂���ꍇ
				if(RecvMode==recvmode_DATA || RecvMode==recvmode_IGNORE) {
					//����M�����G���[�o�^
					RecvDataError|=error::underflow;

					//�f�[�^���t
					if(RecvIO->second->can_put()) {
						RecvIO->second->put(data(RecvDataID, hmLib::move(static_cast<shared_read_array>(RecvDataArray)), RecvDataError));
					}
					RecvDataArray.release();
				}

				//�t���b�V���I
				RecvIO->second->flush();

				//���[�h��ID��M�Ɉڍs
				RecvMode=recvmode_ID;
			}
		public:
			//======= vcom functions ========
			//���M��ch���擾����
			virtual unsigned char get_ch() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				//�O��C���N�������g����Ă��Ȃ��āAflowing�������Ă���΁A�C���N�������g
				if(!SendIncrement & !SendIO->second->flowing()) {
					++SendIO;
					if(SendIO == pIOs.end())SendIO = pIOs.begin();
					SendIncrement=true;
				}

				return SendIO->first;
			}
			//���M���L�����Z������
			virtual void cancel_get() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				SendMode=sendmode_ID;
				SendCnt=0;
			}
			//���M���X�L�b�v����
			virtual void skip_get() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				SendMode=sendmode_ID;
				SendCnt=0;
				SendData.release();
			}
			//��M����ch���擾����
			virtual void put_ch(unsigned char Ch) {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
				if(!is_lock())lock();

				RecvIO=std::lower_bound(pIOs.begin(), pIOs.end(), element(Ch, 0), compare());
			}
			//��M���L�����Z������
			virtual void cancel_put() {
				hmr_assert(is_open(), exceptions::not_opend());

				//Lock����ĂȂ���΁A�������b�N
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
