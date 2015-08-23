#ifndef HMR_CODE_VMC2_CPP_INC
#define HMR_CODE_VMC2_CPP_INC 200
#
#ifndef HMR_CODE_VMC2_INC
#	include"vmc2.hpp"
#endif

namespace hmr {
	namespace code {
		const unsigned char vmc2::PacStrt[4]={'#', '[', 0x00, ']'};
		const unsigned char vmc2::PacStrtSize=4;
		const unsigned char vmc2::PacTrmn[4]={'#', '|', 0x0d, 0x0a};
		const unsigned char vmc2::PacTrmnSize=4;

		//vmc2������������
		vmc2::vmc2(unsigned char Ch_) :Ch(Ch_) {}
		vmc2::vmc2(unsigned char Ch_, client& rVCom1) :Ch(Ch_) {open(rVCom1);}
		//vmc2���I�[������
		vmc2::~vmc2() {close();}
		//VCom1���J��
		bool vmc2::open(client& rVCom1) {
			if(is_open())return true;

			pVCom1=&rVCom1;

			//��M���ϐ�
			RecvErr=puterr_NULL;
			RecvMode=putmode_IDLE;
			RecvCnt=0;
			RecvCh=0xFF;

			//���M���ϐ�
			SendErr=geterr_NULL;
			SendMode=getmode_STRT;
			SendCnt=0;

			return false;
		}
		//VCom1�����
		bool vmc2::close() {
			if(!is_open())return true;

			//��M���[�h�̂܂܂̏ꍇ
			if(RecvMode==putmode_DATA) {
				pVCom1->flush();
				RecvMode=putmode_IDLE;
				RecvCnt=0;
			}
			//���M���[�h�̂܂܂̏ꍇ
			if(SendMode==getmode_DATA) {
				pVCom1->cancel_get();
				SendMode=getmode_STRT;
				SendCnt=0;
			}

			pVCom1=0;
		}
		//���łɗ��p�\�ɂȂ��Ă��邩
		bool vmc2::is_open() { return pVCom1!=0; }
		//�t���b�V������i�����������Ȃ��j
		void vmc2::flush() { return; }
		//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
		bool vmc2::can_putc() {
			if(RecvMode==putmode_DATA) {
				return pVCom1->can_putc();
			}
			return 1;
		}
		//��M�f�[�^�𓊂������
		void vmc2::putc(unsigned char c) {
			unsigned char cnt;

			//��M�ł��Ȃ��̂ɓ������ꂽ��G���[
			if(!can_putc()) {
				RecvErr=puterr_INVALID_PUTC;
			}

			//���M���[�h(RecvPacStrt�҂�)
			if(RecvMode==putmode_IDLE) {
				//CH������M����Ch�ƈ�v
				if(RecvCnt==2 && (c&0x0F)==Ch) {
					++(RecvCnt);

					//�����Ch���󂯎��
					RecvCh=((c>>4)&0x0F);
				}//��CH������M����Strt�ƈ�v
				else if(RecvCnt!=2 && c==PacStrt[RecvCnt]) {
					++(RecvCnt);

					//���ׂĂ�STRT����M
					if(RecvCnt==PacStrtSize) {
						//���[�h��Pac��M���[�h�ֈڍs
						RecvMode=putmode_DATA;
						RecvCnt=0;

						//��Mch��ʒm
						pVCom1->put_ch(RecvCh);
					}
				}//STRT�͕s��v�����擪STRT�Ƃ͈�v
				else if(c==PacStrt[0]) {
					RecvCnt=1;
				}//����ȊO
				else {
					RecvCnt=0;
				}
			}//��M���[�h(PacTrmn�҂�)
			else {
				//��M������putmodeCnt�Ԗڂ�PacTrmn�����ƈ�v
				if(PacTrmn[RecvCnt]==c) {
					//�J�E���^�[���Z
					++(RecvCnt);

					//�SRecvPacTrmn��M
					if(RecvCnt==PacTrmnSize) {
						//�I���ʒm
						pVCom1->flush();

						//���[�h��Pac��M���[�h�ֈڍs
						RecvMode=putmode_IDLE;
						RecvCnt=0;
					}
				}//��M������0�Ԗڂ�PacTrmn�����ƈ�v
				else if(PacTrmn[0]==c) {
					for(cnt=0; cnt<RecvCnt; ++cnt) {
						//��M�ł��Ȃ��ꍇ
						if(pVCom1->can_putc()==0) {
							//�G���[����
							RecvErr=puterr_RECVREJECTED;
							//��M���X�g�b�v
							pVCom1->flush();
							//���[�h��߂�
							RecvMode=putmode_IDLE;
							RecvCnt=0;
							return;
						}

						//�f�[�^�������n��
						pVCom1->putc(PacTrmn[cnt]);
					}
					//�J�E���^�[��1��(1�����͈�v��������)
					RecvCnt=1;
				}//��M������PacTrmn�����ƕs��v
				else {
					for(cnt=0; cnt<RecvCnt; ++cnt) {
						//��M�ł��Ȃ��ꍇ
						if(pVCom1->can_putc()==0) {
							//�G���[����
							RecvErr=puterr_RECVREJECTED;
							//��M���X�g�b�v
							pVCom1->flush();
							//���[�h��߂�
							RecvMode=putmode_IDLE;
							RecvCnt=0;
							return;
						}

						//�f�[�^�������n��
						pVCom1->putc(PacTrmn[cnt]);
					}
					//�J�E���^�[���Z�b�g
					RecvCnt=0;

					//��M�ł��Ȃ��ꍇ
					if(pVCom1->can_putc()==0) {
						//�G���[����
						RecvErr=puterr_RECVREJECTED;
						//��M���X�g�b�v
						pVCom1->flush();
						//���[�h��߂�
						RecvMode=putmode_IDLE;
						RecvCnt=0;
						return;
					}

					//�f�[�^�������n��
					pVCom1->putc(c);

				}
			}
		}
		//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
		bool vmc2::can_getc() {
			if(SendMode==getmode_STRT&&SendCnt==0) {
				//����M�ł���f�[�^�����݂��邩
				return pVCom1->can_getc();
			} else if(SendMode==getmode_DATA) {
				//���M�ςݏ�ԃf�[�^�����ɂ���ꍇ�́A����TRMN�̂͂��Ȃ̂�1
				if(pVCom1->flowing()==0 && SendCnt!=0)return 1;
				return pVCom1->can_getc();
			}

			return 1;
		}
		//���M�f�[�^���Ăяo��
		unsigned char vmc2::getc() {
			unsigned char c;

			//getc�ł��Ȃ��Ƃ��ɂ́C�G���[
			if(!can_getc()) {
				SendCnt=geterr_INVALID_GETC;
				return 0xC0;
			}

			//�񑗐M���[�h(SendPacStrt�҂�)
			if(SendMode==getmode_STRT) {
				//CH�t�F�[�Y�̎�
				if(SendCnt==2) {
					c = ((Ch)<<4)|(pVCom1->get_ch()&0x0F);
					++(SendCnt);
				}//�Œ蕶���񑗐M�t�F�[�Y
				else {
					c = PacStrt[SendCnt];
					++(SendCnt);

					//PacStrt�𑗂�I�������A���M�p�ɏ�����
					if(SendCnt>=4) {
						SendMode=getmode_DATA;
						SendCnt=0;
					}
				}
			}//Data���M���[�h
			else if(SendMode==getmode_DATA) {
				//�f�[�^���I�������ꍇ�ATRMN�Ɉڍs����
				if(pVCom1->flowing()==0 && SendCnt!=0) {
					//TRMN���M
					SendMode=getmode_TRMN;
					c=PacTrmn[0];
					SendCnt=1;
				}//send�Ɏ��s����ꍇ�͋����I��
				else if(pVCom1->can_getc()==0) {
					//�G���[����
					SendErr=geterr_SENDREJECTED;

					//vcom�㏈��
					pVCom1->cancel_get();

					//TRMN���������M
					SendMode=getmode_TRMN;
					c=PacTrmn[0];
					SendCnt=1;
				} else {
					//�f�[�^�𑗐M
					c=pVCom1->getc();
					SendCnt=1;
				}
			}//TRMN�𑗂�
			else if(SendMode==getmode_TRMN) {
				//SendPacTrmn���M
				c=PacTrmn[SendCnt];
				++(SendCnt);

				//PacTrmn�𑗂�I�������A���M�p�ɏ�����
				if(SendCnt>=PacTrmnSize) {
					SendMode=getmode_STRT;
					SendCnt=0;
					SendErr=0;
				}
			}
			return c;
		}
		//���M�f�[�^��eof�ʒu�łȂ������m�F����
		bool vmc2::flowing() {
			return !(SendMode==getmode_STRT&&SendCnt==0);
		}
		//��M�������I�ɏI��������
		void vmc2::force_end_get() {
			if(RecvMode==putmode_IDLE)return;
			//�f�[�^��M���Ȃ��������Ƃɂ���
			pVCom1->flush();

			//vmc2����݂���n��
			RecvMode=putmode_IDLE;
			RecvCnt=0;
		}
		//���M�������I�ɏI��������
		void vmc2::force_end_put() {
			if(SendMode==getmode_STRT)return;

			//�f�[�^���M���Ȃ��������Ƃɂ���
			pVCom1->cancel_get();

			SendMode=getmode_STRT;
			SendCnt=0;
		}
	}
}
#
#endif
