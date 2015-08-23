#ifndef HMR_CODE_VIC1_CPP_INC
#define HMR_CODE_VIC1_CPP_INC 201
#
/*
=== code::vic1 ===
v2_01/140622 hmIto
	open/close�֐��Œl��Ԃ��Ȃ��p�X�������������C��
*/
#ifndef HMR_CRC8_INC
#	include<homuraLib_v2/crc8.h>
#endif
#ifndef HMR_CODE_CLIENT_INC
#	include"client.hpp"
#endif
#ifndef HMR_CODE_VIC1_INC
#	include"vic1.hpp"
#endif

namespace hmr {
	namespace code {
		//========== vic1::buf ===========
		//�o�b�t�@������
		vic1::buffer::buffer(vic1* VIC1_, client* pClient_, unsigned char* BufBegin, unsigned char* BufEnd)
			: VIC1(VIC1_)
			,pClient(pClient_){
			Begin=BufBegin;
			End=BufEnd;
			VComItr=BufBegin;
			GateItr=BufBegin;
		}
		//�o�b�t�@������
		vic1::buffer::buffer(vic1* VIC1_, unsigned char* BufBegin, unsigned char* BufEnd)
			: VIC1(VIC1_)
			, pClient(0) {
			Begin=BufBegin;
			End=BufEnd;
			VComItr=BufBegin;
			GateItr=BufBegin;
		}
		//�o�b�t�@�I�[��
		vic1::buffer::~buffer() {
			Begin=0;
			End=0;
			VComItr=0;
			GateItr=0;
			//�f�[�^��M���Ȃ��������Ƃɂ���
			pClient->flush();
			//�f�[�^���M���Ȃ��������Ƃɂ���
			pClient->cancel_get();
			pClient=0;
		}
		//��[�ł�������[
		void vic1::buffer::load() {
			//��M�\�ł��������s
			while(pClient->can_getc()) {
				//���ۂɎ�M
				*((VComItr)++) = pClient->getc();

				//�o�b�t�@�������ς��ɂȂ����ꍇ�A���邢��vcom����̑��M���r�؂ꂽ�ꍇ
				if(VComItr+1 == End || pClient->flowing()==0) {
					//crc8���v�Z���āA�����ɏ�������
					*(VComItr) = crc8_puts(0x00, Begin, VComItr - Begin);
					++(VComItr);
					break;
				}
			}
		}
		bool vic1::buffer::open(client* pClient_) {
			pClient=pClient_;
			return false;
		}
		bool vic1::buffer::close() {
			pClient=0;
			return false;
		}
		//���łɗ��p�\�ɂȂ��Ă��邩
		bool vic1::buffer::is_open() {return pClient!=0;}
		//�o�b�t�@�֏������񂾃f�[�^������
		void vic1::buffer::flush() {
			unsigned char CRC8;

			//���łɎ��s���Ă���Ζ���
			if(VIC1->Fail==1)return;

			//����������M���Ă��Ȃ���Ζ���
			if(GateItr==Begin) {
				//���s�������Ƃ�ʒm
				VIC1->Fail=1;
				//Iterator�������ʒu��
				VComItr=Begin;
				GateItr=Begin;

				return;
			}

			//1byte�߂�
			--(GateItr);

			//GateItr�܂ł̃`�F�b�N�T�������Z
			CRC8=crc8_puts(0x00, Begin, GateItr - Begin);

			//�`�F�b�N�T���Ɏ��s���Ă���ꍇ�͔j��
			if(*(GateItr)!=CRC8) {
				//���s�������Ƃ�ʒm
				VIC1->Fail=1;
				//Iterator�������ʒu��
				VComItr=Begin;
				GateItr=Begin;

				return;
			}

			//�f�[�^�Z�[�u
			//VCom�ɏ������������
			VComItr = Begin;
			while(VComItr!= GateItr) {
				if(pClient->can_putc()==0) {
					//�G���[����
					VIC1->Err=error_SAVE_REJECTED;
					break;
				}
				pClient->putc(*(VComItr++));
			}

			//Iterator�������ʒu��
			VComItr=Begin;
			GateItr=Begin;
		}
		//�o�b�t�@�ɏ������݉\��
		bool vic1::buffer::can_putc() {
			//�o�b�t�@�ɋ󂫂�����ꍇ
			//	���M�ƈ����flush����܂łǂꂪCheckSum���킩��Ȃ��̂ŁA
			//	1byte�����Ă����K�v���Ȃ��_�ɒ���
			return GateItr < End;
		}
		//�o�b�t�@�֏�������
		void vic1::buffer::putc(unsigned char c) {
			//�������݉\�łȂ��ꍇ�A�G���[����
			if(!can_putc()) {
				//�G���[����
				VIC1->Err=error_OVERFLOW;
				//���s�������Ƃ�ʒm
				VIC1->Fail=1;
				//Iterator�������ʒu��
				VComItr=Begin;
				GateItr=Begin;
			}

			//�f�[�^��������
			*((GateItr)++)=c;
		}
		//�o�b�t�@�ւ���̓ǂݏo������؂�ʒu�ɂ��邩
		bool vic1::buffer::flowing() {
			//�������ݍς݃o�b�t�@�����܂œǂݏo���Ă��Ȃ�
			//���邢�́A�o�b�t�@�����܂ŏ�������ł��炸���f�[�^�͌p�����Ă���ivcom�����M�]�n������j
			return GateItr < VComItr
				|| (VComItr < End && pClient->flowing());
		}
		//�o�b�t�@����̓ǂݏo���\��
		bool vic1::buffer::can_getc() {
			//�������ݍς݃o�b�t�@�����܂œǂݏo���Ă��Ȃ�
			//���邢�́Avcom�����M�\�ł���
			return GateItr != VComItr
				|| pClient->can_getc();
		}
		//�o�b�t�@����̓ǂݏo��
		unsigned char vic1::buffer::getc() {
			//�o�b�t�@�ւ̕�[�]�n������ꍇ
			if(VComItr != End && pClient->flowing()) {
				//��[�ł�������[
				load();
			}//��[�]�n���Ȃ����S�đ��M�������� => �V�K�o�b�t�@������ꍇ
			else if(GateItr == VComItr) {
				//Iterator�������ʒu��
				GateItr=Begin;
				VComItr=Begin;

				//��[�ł�������[
				load();
			}

			//�܂��c���Ă���ꍇ
			if(GateItr < VComItr) {
				return *((GateItr)++);
			} else {
				//�G���[����
				VIC1->Err=error_OVERREAD;
				return 0;
			}
		}
		//�o�b�t�@�ւ̏�������/�ǂݍ��݂��I�����A�o�b�t�@���N���A����
		void vic1::buffer::clear() {
			GateItr=Begin;
			VComItr=Begin;
		}
		//�o�b�t�@�ւ̓ǂݍ��݂��Ȃ��������Ƃɂ���
		void vic1::buffer::cancel_get() { GateItr=Begin; }
		//�o�b�t�@�̓����f�[�^�����ׂĔj�����Avcom�ɂ�Packet�I����ʒm
		void vic1::buffer::errorflush() {
			GateItr=Begin;
			VComItr=Begin;
			pClient->flush();
		}
		//vcom���̂��f�[�^�������Ă��邩
		bool vic1::buffer::vcom_can_getc() { return pClient->can_getc(); }
		//vcom���̂��f�[�^�������Ă��邩
		bool vic1::buffer::vcom_flowing() { return pClient->flowing(); }
		//vcom���̂��f�[�^�������Ă��邩
		void vic1::buffer::vcom_skip_get() { return pClient->skip_get(); }
		//vcom���̂��f�[�^�������Ă��邩
		unsigned char vic1::buffer::vcom_get_ch() { return pClient->get_ch(); }

		//========== vic1::gate ===========
		//-------- Mode�J��4�֐� ----------------
		//�R�}���h���M������
		void vic1::inform_send() {
			//ActiveCmd�ɓo�^
			ActiveCmd=Send;

			switch(Mode) {
			case mode_IDLE:
				//�Ђ܂ɂȂ������A�b�������l�������ꍇ
				if(NextTergetCh!=ch_NULL) {
					TergetCh=NextTergetCh;
					NextTergetCh=ch_NULL;
					SendCnt=0;
					Send=cmd_WHAT;

				}//�{���ɒP�ɉɂȏꍇ
				else {
					SendCnt=0;
					Send=cmd_NULL;
				}
				break;
			case mode_RESEND:
				//com���瑗�M�f�[�^���擾�\�ȏꍇ
				if(Buffer.can_getc()) {
					//�f�[�^���M�J�n
					Mode=mode_SEND;
					SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
					Send=cmd_DATA;
				} else {
					//Stop���M
					Mode=mode_STOP;
					Send=cmd_STOP;
					SendCnt=0;
				}
				break;
			default:
				//���M���I���đҋ@��Ԃɓ���
				SendCnt=0;
				Send=cmd_NULL;
				break;
			}
		}
		//�������̃R�}���h��M����
		void vic1::inform_recv() {
			RecvCnt=0;
			//�Ђ܂ɂ��Ă���Ƃ�
			if(Mode==mode_IDLE) {
				//���肩��̃R�}���h���e�ŕ���
				switch(Recv) {
					//���肪Start�v�������Ă����ꍇ
				case cmd_START:
					//Ack��Ԃ��A���g��Recv�ɂ���
					Mode=mode_RECV;
					Buffer.clear();
					TergetCh=RecvCh;
					SendCnt=0;
					Send=cmd_ACK;
					break;
				case cmd_ERR:
					//��������
					break;
				default:
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					TergetCh=RecvCh;
					SendCnt=0;
					Send=cmd_ERR;
				}
				return;
			}

			//�ʐM���ɑ��肪�m��Ȃ��l����ʐM�������ꍇ
			if(RecvCh!=TergetCh) {
				//Start�v���������ꍇ�A�؂�ւ��\��
				if(Recv==cmd_START && NextTergetCh>RecvCh) {
					NextTergetCh=RecvCh;
					return;
				}
				//����ȊO�Ȃ�A����
			}

			//�Ԏ��������̂ŁA�ЂƂ܂��^�C���A�E�g���Z�b�g
			pTimeout->restart();
			//�Ԏ��������̂ŁA�^�C���A�E�g�̃J�E���g��߂�
			TimeOutCnt=0;

			//�G���[�̏ꍇ�͖ⓚ���p��IDLE��
			if(Recv == cmd_ERR) {
				//���ɘb�������鑊�肪�o�^����Ă����ꍇ
				if(NextTergetCh!=ch_NULL) {
					//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
					Mode=mode_IDLE;
					pTimeout->disable();
					TergetCh=NextTergetCh;
					NextTergetCh=ch_NULL;
					SendCnt=0;
					Send=cmd_WHAT;
				}//���̗\����Ȃ��ꍇ
				else {
					//���[�h��Idle�ɂ��A�ق�
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_NULL;
				}
			}//Recv��
			else if(Mode==mode_RECV) {
				//�f�[�^�𑗂��Ă����ꍇ
				switch(Recv) {
				case cmd_START:
					//ACK���M
					SendCnt=0;
					Send=cmd_ACK;
					break;
				case cmd_DATA:
					//�t���b�V��
					Buffer.flush();

					//��M�ɐ������Ă��邩�m�F
					if(Fail==0) {
						//ACK���M
						SendCnt=0;
						Send=cmd_ACK;
					} else {
						//�t���O�����낷
						Fail=0;
						//NACK���M
						SendCnt=0;
						Send=cmd_NACK;
					}
					break;
				case cmd_STOP:
					//�p�P�b�g�I���ʒm
					Buffer.errorflush();

					//com���瑗�M�f�[�^���擾�\�ȏꍇ(�܂�Ԃ��Ȃ̂�flowing�͋C�ɂ��Ȃ�)
					if(Buffer.can_getc()) {
						//����M����ւ��v��
						Mode=mode_START;
						pTimeout->restart();
						pTimeout->enable();
						SendCnt=0;
						Send=cmd_NACK;
					}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
					else {
						//�I���󂯓���
						Mode=mode_IDLE;
						SendCnt=0;
						Send=cmd_ACK;
					}
					break;
				default:
					//�p�P�b�g�I���ʒm
					Buffer.errorflush();

					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					break;
				}

				//ReecvMode�̎��ɂ́A�f�[�^���o�b�t�@�ɂ���Ă��܂��Ă���̂ŁA���Z�b�g�B
				Buffer.clear();
				Fail=0;
			}//Start�R�}���h�ɑ΂���ԓ�
			else if(Mode==mode_START && (ActiveCmd==cmd_START || ActiveCmd==cmd_NACK)) {
				switch(Recv) {
					//���肪��������ꍇ
				case cmd_ACK:
					//com���瑗�M�f�[�^���擾�\�ȏꍇ
					if(Buffer.can_getc()) {
						//���M���������ꍇ
						if(Send!=cmd_NULL) {
							//ESC����U���M
							Mode=mode_RESEND;
							SendCnt=0;
							Send=cmd_ESC;
						}//�����M�\�ȏꍇ
						else {
							//DATA�𑗐M
							Mode=mode_SEND;
							SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
							Send=cmd_DATA;
						}
					}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
					else {
						//Stop���M
						Mode=mode_STOP;
						SendCnt=0;
						Send=cmd_STOP;
					}
					break;
					//���肪���ۂ��Ă����ꍇ
				case cmd_NACK:
					//���ɘb�������鑊�肪�o�^����Ă����ꍇ
					if(NextTergetCh!=ch_NULL) {
						//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
						Mode=mode_IDLE;
						pTimeout->disable();
						TergetCh=NextTergetCh;
						NextTergetCh=ch_NULL;
						SendCnt=0;
						Send=cmd_WHAT;
					}//���̗\����Ȃ��ꍇ
					else {
						//���[�h��Idle�ɂ��A�ق�
						Mode=mode_IDLE;
						SendCnt=0;
						Send=cmd_NULL;
					}
					break;
					//���肪Start�v�����d�˂Ă����ꍇ
				case cmd_START:
					//�����莩���̕����傫��ch�̂Ƃ�
					if(RecvCh < Ch) {
						//Ack��Ԃ��A���g��Recv�ɂ���
						Mode=mode_RECV;
						pTimeout->disable();
						SendCnt=0;
						Send=cmd_ACK;
					}
					//�������ꍇ�͖���
					break;
					//����ȊO�̂��Ƃ������Ă���ꍇ
				default:
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
					break;
				}
			}//DATA�R�}���h�ɑ΂���ԓ�
			else if(Mode==mode_SEND && ActiveCmd==cmd_DATA) {
				switch(Recv) {
					//���肪��M�ɐ������Ă����ꍇ
				case cmd_ACK:
					//com���瑗�M�f�[�^���擾�\�ȏꍇ�iDATA�Ɍp����������ꍇ�Ɍ���j
					if(Buffer.vcom_can_getc() && Buffer.vcom_flowing()) {
						//���M���������ꍇ
						if(Send!=cmd_NULL) {
							//ESC����U���M
							Mode=mode_RESEND;
							SendCnt=0;
							Send=cmd_ESC;
						}//�����M�\�ȏꍇ
						else {
							//DATA�𑗐M
							Mode=mode_SEND;
							SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
							Send=cmd_DATA;
						}
					}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
					else {
						//Stop���M
						Mode=mode_STOP;
						SendCnt=0;
						Send=cmd_STOP;
					}
					break;
					//���肪��M�Ɏ��s���Ă����ꍇ
				case cmd_NACK:
					//�o�b�t�@�̓ǂݏo���ʒu��������
					Buffer.cancel_get();
					//���M���������ꍇ
					if(Send!=cmd_NULL) {
						//ESC����U���M
						Mode=mode_RESEND;
						SendCnt=0;
						Send=cmd_ESC;
					}//�����M�\�ȏꍇ
					else {
						//DATA�𑗐M
						Mode=mode_SEND;
						SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
						Send=cmd_DATA;
					}
					break;
				default:
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
				}
			}//Stop�R�}���h�ɑ΂���ԓ�
			else if(Mode==mode_STOP && ActiveCmd==cmd_STOP) {
				switch(Recv) {
					//���肪��������ꍇ
				case cmd_ACK:
					//���ɘb�������鑊�肪�o�^����Ă����ꍇ
					if(NextTergetCh!=ch_NULL) {
						//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
						Mode=mode_IDLE;
						pTimeout->disable();
						TergetCh=NextTergetCh;
						NextTergetCh=ch_NULL;
						SendCnt=0;
						Send=cmd_WHAT;
					}//���̗\����Ȃ��ꍇ
					else {
						//���[�h��Idle�ɂ��A�ق�
						Mode=mode_IDLE;
						pTimeout->disable();
						SendCnt=0;
						Send=cmd_NULL;
					}
					break;
					//���肪���ۂ��Ă����ꍇ
				case cmd_NACK:
					//���g�͎�M���[�h�ɂ��A����Ɏ���̈ӎv��`����
					Mode=mode_RECV;
					Buffer.clear();	//�o�b�t�@���N���A���Ď�M�ɔ�����
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ACK;
					break;
				default:
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					pTimeout->disable();
					SendCnt=0;
					Send=cmd_ERR;
				}
			} else {
				//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
				Mode=mode_IDLE;
				pTimeout->disable();
				SendCnt=0;
				Send=cmd_ERR;
			}
		}
		//�f�[�^�̗L�����m�F���āAIDLE����START�֑J�ڂ���֐�
		bool vic1::checkData() {
			//IDLE�ł���A���M���R�}���h���Ȃ��A���M�������f�[�^������A�̂R�������������ꂽ�ꍇ�̂ݔ���
			if(Mode==mode_IDLE && Send==cmd_NULL && Buffer.vcom_can_getc()) {
				//ch�擾���Astart���[�h�ֈڍs
				TergetCh=Buffer.vcom_get_ch();
				Mode=mode_START;
				pTimeout->restart();
				pTimeout->enable();
				SendCnt=0;
				Send=cmd_START;
				return hmLib_true;
			}
			return hmLib_false;
		}
		//timeout�����[�h���ꕔ�ς��Ă���ł�����
		//vic1��timeout�R�}���h�đ��֐�
		void vic1::timeout() {
			if(Mode==mode_START) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					Buffer.vcom_skip_get();
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					return;
				}
				Send=cmd_START;
				SendCnt=0;
			} else if(Mode==mode_SEND) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					Buffer.vcom_skip_get();
					//ESC����U���M
					Mode=mode_RESEND;
					SendCnt=0;
					Send=cmd_ESC;
					return;
				}
				//ESC����U���M
				Mode=mode_RESEND;
				SendCnt=0;
				Send=cmd_ESC;
			} else if(Mode==mode_STOP) {
				if(TimeOutCnt>=MaxTimeOutNum) {
					TimeOutCnt=0;
					//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
					Mode=mode_IDLE;
					SendCnt=0;
					Send=cmd_ERR;
					return;
				}		Send=cmd_STOP;
				SendCnt=0;
			}
			++(TimeOutCnt);
		}

		//========== vic1 ��{�֐� ==========
		//vic1������������ �����́A�����̃`�����l���ԍ�(0x00-0x0F)�A����̃`�����l���ԍ��A�^���鑗��M�p�Œ蒷�o�b�t�@�A�h���X
		vic1::vic1(client& rClient_
			, const unsigned char Ch_
			, unsigned char* BufBegin
			, unsigned char* BufEnd
			, timeout_functions& Timeout_)
			: Buffer(this, &rClient_, BufBegin, BufEnd)
			, pTimeout(&Timeout_) {
			ActiveCmd=cmd_NULL;
			Ch=Ch_;
			Err=error_NULL;
			Fail=0x00;
			TimeOutCnt=0;

			Mode=mode_IDLE;
			NextTergetCh=ch_NULL;
			TergetCh=ch_NULL;
			Send=cmd_NULL;
			SendCnt=0;

			Recv=cmd_NULL;
			RecvCh=cmd_NULL;
			RecvCnt=0;
			//	maintask_start(task, 0, 0, 0);
		}		
		//vic1������������
		vic1::vic1(const unsigned char Ch_
			, unsigned char* BufBegin
			, unsigned char* BufEnd
			, timeout_functions& Timeout_)
			: Buffer(this, BufBegin, BufEnd)
			, pTimeout(&Timeout_) {
			ActiveCmd=cmd_NULL;
			Ch=Ch_;
			Err=error_NULL;
			Fail=0x00;
			TimeOutCnt=0;

			Mode=mode_IDLE;
			NextTergetCh=ch_NULL;
			TergetCh=ch_NULL;
			Send=cmd_NULL;
			SendCnt=0;

			Recv=cmd_NULL;
			RecvCh=cmd_NULL;
			RecvCnt=0;
		}

		//vic1���I�[������
		vic1::~vic1() {
			pTimeout->disable();

			Ch=ch_NULL;
			TergetCh=ch_NULL;
			NextTergetCh=ch_NULL;

			Mode=mode_IDLE;
			Err=0;

			ActiveCmd=cmd_NULL;
			Send=cmd_NULL;
			SendCnt=0;
			Recv=cmd_NULL;
			RecvCnt=0;
			RecvCh=ch_NULL;

			//	maintask_stop(task);
		}
		bool vic1::open(client& rClient_) {
			return Buffer.open(&rClient_);
		}
		bool vic1::close() {
			return Buffer.close();
		}
		//========== gate interface =================
		//��M�f�[�^1byte�ڔ��ʊ֐�
		void vic1::putc_1byte(unsigned char c) {
			//��������Ch���H
			if(Ch==(c&0x0F)) {
				RecvCh=(c>>4);
				++(RecvCnt);
			}//��L�ȊO�Ȃ�A���Ȃ��Ƃ������ɂƂ��ăR�}���h�ł͂Ȃ�
			else {
				//��M���Ȃ�f�[�^���������Ă���
				if(Mode==mode_RECV) {
					Buffer.putc(c);
				}
			}
		}
		//��M�f�[�^2byte�ڔ��ʊ֐�
		void vic1::putc_2byte(unsigned char c1, unsigned char c2) {
			//���ʐM���̑���̏ꍇ
			if(RecvCh==TergetCh) {
				//�Ƃɂ����R�����g�͕����Ă���
				Recv=c2;
				++(RecvCnt);
			}//�m��Ȃ��l����Start�R�}���h�������ꍇ
			else if(c2==cmd_START) {
				//����������Ă���
				Recv=c2;
				++(RecvCnt);
			}//��L�ȊO�Ȃ�A���Ȃ��Ƃ������ɂƂ��ăR�}���h�ł͂Ȃ�
			else {
				//�J�E���^��߂�
				RecvCnt=0;

				//��M���Ȃ�f�[�^���������Ă���
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}
				putc_1byte(c2);
			}
		}
		//��M�f�[�^3byte�ڔ��ʊ֐�
		void vic1::putc_3byte(unsigned char c1, unsigned char c2, unsigned char c3) {
			//Trmn1
			if(c3==0x0d) {
				++(RecvCnt);
			}//��L�ȊO�Ȃ�A���Ȃ��Ƃ�1byte�ڂ̕����͎����ɂƂ��ăR�}���h�ł͂Ȃ�
			else {
				//�J�E���^��߂�
				RecvCnt=0;

				//��M���Ȃ�f�[�^���������Ă���
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}
				putc_1byte(c2);

				if(RecvCnt==0) {
					putc_1byte(c3);
				} else if(RecvCnt==1) {
					putc_2byte(c2, c3);
				}
			}
		}
		//��M�f�[�^3byte�ڔ��ʊ֐�
		void vic1::putc_4byte(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4) {
			//Trmn1
			if(c4==0x0a) {
				//�R�}���h��M�����A����
				inform_recv();
			}//��L�ȊO�Ȃ�A���Ȃ��Ƃ�1byte�ڂ̕����͎����ɂƂ��ăR�}���h�ł͂Ȃ�
			else {
				//�J�E���^��߂�
				RecvCnt=0;

				//��M���Ȃ�f�[�^���������Ă���
				if(Mode==mode_RECV) {
					Buffer.putc(c1);
				}

				putc_1byte(c2);

				if(RecvCnt==0) {
					putc_1byte(c3);

					if(RecvCnt==0) {
						putc_1byte(c4);
					} else if(RecvCnt==1) {
						putc_2byte(c3, c4);
					}

				} else if(RecvCnt==1) {
					putc_2byte(c2, c3);

					if(RecvCnt==0) {
						putc_1byte(c4);
					} else if(RecvCnt==1) {
						putc_2byte(c3, c4);
					} else if(RecvCnt==2) {
						putc_3byte(c2, c3, c4);
					}
				}

			}
		}
		//���łɗ��p�\�ɂȂ��Ă��邩
		bool vic1::is_open() {return Buffer.is_open();}
		//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
		bool vic1::can_putc() {
			//��M�f�[�^�͏�Ɏ󂯓���\�i�ǂݎ̂Ă邩�ǂ����͕ʁj
			return hmLib_true;
		}
		//��M�f�[�^�𓊂������
		void vic1::putc(unsigned char c) {
			if(RecvCnt==0) {
				putc_1byte(c);
			} else if(RecvCnt==1) {
				putc_2byte(((RecvCh)<<4)|(Ch), c);
			} else if(RecvCnt==2) {
				putc_3byte(((RecvCh)<<4)|(Ch), Recv, c);
			} else {
				putc_4byte(((RecvCh)<<4)|(Ch), Recv, 0x0d, c);
			}
		}
		//��M�f�[�^���t���b�V������igate�݊��p�AVIC���ł̏����͂Ȃ��j
		void vic1::flush() {}
		//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
		bool vic1::can_getc() {
			//���M���[�h�ɑJ�ڂł���Ƃ̒ʍ�������΁A�J��
			if(checkData())return hmLib_true;

			//���M���[�h�ŁA���M�f�[�^���S�đ��M�������Ă��Ȃ��ꍇ
			if(Send==cmd_DATA && SendCnt==0xFF)return Buffer.can_getc();

			//�R�}���h���M���̏ꍇ
			if(Send!=cmd_NULL)return hmLib_true;

			return hmLib_false;
		}
		//���M�f�[�^���Ăяo��
		unsigned char vic1::getc() {
			unsigned char c;

			//���M�ł��Ȃ��ꍇ�̓G���[
			if(!can_getc()) {
				Err=error_INVALID_GETC;
				return 0xC0;
			}

			//�f�[�^���M���[�h�ŁA���f�[�^�𑗂�I���Ă��Ȃ��ꍇ
			if(Send==cmd_DATA && SendCnt==0xFF) {
				//�f�[�^���M���̏ꍇ�́A�f�[�^�𑗐M����
				c=Buffer.getc();

				//���M���ׂ��f�[�^�𑗂�I�����ꍇ�́ASendCnt��߂�
				if(Buffer.flowing()==0) {
					//SendCnt�𐳂����l�ɖ߂�
					SendCnt=0;
				}
			} else {
				//SendCnt�����Ă�����A�G���[����
				if(SendCnt>3) {
					Err=error_INCORRECT_SENDCNT;
					Send=cmd_ERR;
					SendCnt=0;
				}

				//�R�}���h���M���̏ꍇ�́A�R�}���h��ǂ�
				if(SendCnt==0) {
					c=((Ch<<4) | TergetCh);
					//���M�J�E���^��i�߂�
					++(SendCnt);
				} else if(SendCnt==1) {
					switch(Send) {
					case cmd_ACK:
						c=cmd_ACK;
						break;
					case cmd_NACK:
						c=cmd_NACK;
						break;
					case cmd_START:
						c=cmd_START;
						break;
					case cmd_STOP:
						c=cmd_STOP;
						break;
					case cmd_DATA:
						c=cmd_DATA;
						break;
					default:
						Send=cmd_ERR;
						c=cmd_ERR;
						break;
					}
					//���M�J�E���^��i�߂�
					++(SendCnt);
				} else if(SendCnt==2) {
					c=0x0d;
					//���M�J�E���^��i�߂�
					++(SendCnt);
				} else if(SendCnt==3) {
					inform_send();
					c=0x0a;
				}

			}
			return c;
		}
		//���M�f�[�^�𗬂ꑱ���Ă��邩�m�F�iVIC�̏ꍇ�́A�R�}���h������0,����ȊO��1�j
		bool vic1::flowing() { return Send!=cmd_NULL; }

		//============ vic functions ================
		//�ʐM�������I�ɏI��������
		void vic1::force_end() {
			Mode=mode_IDLE;
			Send=cmd_ERR;
			SendCnt=0;
			Recv=cmd_NULL;
			RecvCnt=0;
			ActiveCmd=cmd_NULL;
			Fail=0;
			TergetCh=ch_NULL;
			NextTergetCh=ch_NULL;

		}

	}
}
#
#endif
