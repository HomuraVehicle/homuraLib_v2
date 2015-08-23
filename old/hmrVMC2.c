#ifndef HMR_VMC2_C_INC
#define HMR_VMC2_C_INC 102
#
#ifndef HMR_VMC2_INC
#	include"hmrVMC2.h"
#endif

#include<stdlib.h>

#define vmc2_putmode_IDLE	0	//�ҋ@��
#define vmc2_putmode_DATA	1	//��M��

#define vmc2_getmode_STRT	0
#define vmc2_getmode_DATA	1
#define vmc2_getmode_TRMN	2

static const unsigned char vmc2_PacStrt[4]={'#', '[', 0x00, ']'};
static const unsigned char vmc2_PacStrtSize=4;
static const unsigned char vmc2_PacTrmn[4]={'#', '|', 0x0d, 0x0a};
static const unsigned char vmc2_PacTrmnSize=4;

//vmc2������������
void vmc2_initialize(vmc2* pVMC2, vcom* pVCom, unsigned char Ch) {
	//VCom,Ch
	pVMC2->pVCom=pVCom;
	pVMC2->Ch=Ch;

	//��M���ϐ�
	pVMC2->RecvErr=vmc2_puterr_NULL;
	pVMC2->RecvMode=vmc2_putmode_IDLE;
	pVMC2->RecvCnt=0;
	pVMC2->RecvCh=0xFF;

	//���M���ϐ�
	pVMC2->SendErr=vmc2_geterr_NULL;
	pVMC2->SendMode=vmc2_getmode_STRT;
	pVMC2->SendCnt=0;
}
//vmc2���I�[������
void vmc2_finalize(vmc2* pVMC2) {
	//��M���[�h�̂܂܂̏ꍇ
	if(pVMC2->RecvMode==vmc2_putmode_DATA) {
		vcom1_flush(pVMC2->pVCom);
		pVMC2->RecvMode=vmc2_putmode_IDLE;
		pVMC2->RecvCnt=0;
	}
	//���M���[�h�̂܂܂̏ꍇ
	if(pVMC2->SendMode==vmc2_getmode_DATA) {
		vcom1_cancel_get(pVMC2->pVCom);
		pVMC2->SendMode=vmc2_getmode_STRT;
		pVMC2->SendCnt=0;
	}
}
//�t���b�V������i�����������Ȃ��j
void vmc2_flush(vmc2* pVMC2) { return; }
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian vmc2_can_putc(vmc2* pVMC2) { 
	if(pVMC2->RecvMode==vmc2_putmode_DATA) {
		return vcom1_can_putc(pVMC2->pVCom);
	}
	return 1;
}
//��M�f�[�^�𓊂������
void vmc2_putc(vmc2* pVMC2, unsigned char c) {
	unsigned char cnt;

	//��M�ł��Ȃ��̂ɓ������ꂽ��G���[
	if(!vmc2_can_putc(pVMC2)) {
		pVMC2->RecvErr=vmc2_puterr_INVALID_PUTC;
	}

	//���M���[�h(vmc2_RecvPacStrt�҂�)
	if(pVMC2->RecvMode==vmc2_putmode_IDLE) {
		//CH������M����Ch�ƈ�v
		if(pVMC2->RecvCnt==2 && (c&0x0F)==pVMC2->Ch) {
			++(pVMC2->RecvCnt);

			//�����Ch���󂯎��
			pVMC2->RecvCh=((c>>4)&0x0F);
		}//��CH������M����Strt�ƈ�v
		else if(pVMC2->RecvCnt!=2 && c==vmc2_PacStrt[pVMC2->RecvCnt]) {
			++(pVMC2->RecvCnt);

			//���ׂĂ�STRT����M
			if(pVMC2->RecvCnt==vmc2_PacStrtSize) {
				//���[�h��Pac��M���[�h�ֈڍs
				pVMC2->RecvMode=vmc2_putmode_DATA;
				pVMC2->RecvCnt=0;

				//��Mch��ʒm
				vcom1_put_ch(pVMC2->pVCom,pVMC2->RecvCh);
			}
		}//STRT�͕s��v�����擪STRT�Ƃ͈�v
		else if(c==vmc2_PacStrt[0]) {
			pVMC2->RecvCnt=1;
		}//����ȊO
		else {
			pVMC2->RecvCnt=0;
		}
	}//��M���[�h(PacTrmn�҂�)
	else {
		//��M������putmodeCnt�Ԗڂ�PacTrmn�����ƈ�v
		if(vmc2_PacTrmn[pVMC2->RecvCnt]==c) {
			//�J�E���^�[���Z
			++(pVMC2->RecvCnt);

			//�Svmc2_RecvPacTrmn��M
			if(pVMC2->RecvCnt==vmc2_PacTrmnSize) {
				//�I���ʒm
				vcom1_flush(pVMC2->pVCom);

				//���[�h��Pac��M���[�h�ֈڍs
				pVMC2->RecvMode=vmc2_putmode_IDLE;
				pVMC2->RecvCnt=0;
			}
		}//��M������0�Ԗڂ�PacTrmn�����ƈ�v
		else if(vmc2_PacTrmn[0]==c) {
			for(cnt=0; cnt<pVMC2->RecvCnt;++cnt) {
				//��M�ł��Ȃ��ꍇ
				if(vcom1_can_putc(pVMC2->pVCom)==0) {
					//�G���[����
					pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
					//��M���X�g�b�v
					vcom1_flush(pVMC2->pVCom);
					//���[�h��߂�
					pVMC2->RecvMode=vmc2_putmode_IDLE;
					pVMC2->RecvCnt=0;
					return;
				}

				//�f�[�^�������n��
				vcom1_putc(pVMC2->pVCom, vmc2_PacTrmn[cnt]);
			}
			//�J�E���^�[��1��(1�����͈�v��������)
			pVMC2->RecvCnt=1;
		}//��M������PacTrmn�����ƕs��v
		else{
			for(cnt=0; cnt<pVMC2->RecvCnt; ++cnt) {
				//��M�ł��Ȃ��ꍇ
				if(vcom1_can_putc(pVMC2->pVCom)==0) {
					//�G���[����
					pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
					//��M���X�g�b�v
					vcom1_flush(pVMC2->pVCom);
					//���[�h��߂�
					pVMC2->RecvMode=vmc2_putmode_IDLE;
					pVMC2->RecvCnt=0;
					return;
				}

				//�f�[�^�������n��
				vcom1_putc(pVMC2->pVCom, vmc2_PacTrmn[cnt]);
			}
			//�J�E���^�[���Z�b�g
			pVMC2->RecvCnt=0;

			//��M�ł��Ȃ��ꍇ
			if(vcom1_can_putc(pVMC2->pVCom)==0) {
				//�G���[����
				pVMC2->RecvErr=vmc2_puterr_RECVREJECTED;
				//��M���X�g�b�v
				vcom1_flush(pVMC2->pVCom);
				//���[�h��߂�
				pVMC2->RecvMode=vmc2_putmode_IDLE;
				pVMC2->RecvCnt=0;
				return;
			}
			
			//�f�[�^�������n��
			vcom1_putc(pVMC2->pVCom, c);

		}
	}
}
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian vmc2_can_getc(vmc2* pVMC2) {
	if(pVMC2->SendMode==vmc2_getmode_STRT&&pVMC2->SendCnt==0) {
		//����M�ł���f�[�^�����݂��邩
		return vcom1_can_getc(pVMC2->pVCom);
	}else if(pVMC2->SendMode==vmc2_getmode_DATA) {
		//���M�ςݏ�ԃf�[�^�����ɂ���ꍇ�́A����TRMN�̂͂��Ȃ̂�1
		if(vcom1_flowing(pVMC2->pVCom)==0 && pVMC2->SendCnt!=0)return 1;
		return vcom1_can_getc(pVMC2->pVCom);
	}

	return 1;
}
//���M�f�[�^���Ăяo��
unsigned char vmc2_getc(vmc2* pVMC2) {
	unsigned char c;

	//getc�ł��Ȃ��Ƃ��ɂ́C�G���[
	if(!vmc2_can_getc(pVMC2)) {
		pVMC2->SendCnt=vmc2_geterr_INVALID_GETC;
		return 0xC0;
	}

	//�񑗐M���[�h(vmc2_SendPacStrt�҂�)
	if(pVMC2->SendMode==vmc2_getmode_STRT) {
		//CH�t�F�[�Y�̎�
		if(pVMC2->SendCnt==2) {
			c = ((pVMC2->Ch)<<4)|(vcom1_get_ch(pVMC2->pVCom)&0x0F);
			++(pVMC2->SendCnt);
		}//�Œ蕶���񑗐M�t�F�[�Y
		else{
			c = vmc2_PacStrt[pVMC2->SendCnt];
			++(pVMC2->SendCnt);

			//PacStrt�𑗂�I�������A���M�p�ɏ�����
			if(pVMC2->SendCnt>=4) {
				pVMC2->SendMode=vmc2_getmode_DATA;
				pVMC2->SendCnt=0;
			}
		}
	}//Data���M���[�h
	else if(pVMC2->SendMode==vmc2_getmode_DATA) {
		//�f�[�^���I�������ꍇ�ATRMN�Ɉڍs����
		if(vcom1_flowing(pVMC2->pVCom)==0 && pVMC2->SendCnt!=0) {
			//TRMN���M
			pVMC2->SendMode=vmc2_getmode_TRMN;
			c=vmc2_PacTrmn[0];
			pVMC2->SendCnt=1;
		}//send�Ɏ��s����ꍇ�͋����I��
		else if(vcom1_can_getc(pVMC2->pVCom)==0) {
			//�G���[����
			pVMC2->SendErr=vmc2_geterr_SENDREJECTED;

			//vcom�㏈��
			vcom1_cancel_get(pVMC2->pVCom);

			//TRMN���������M
			pVMC2->SendMode=vmc2_getmode_TRMN;
			c=vmc2_PacTrmn[0];
			pVMC2->SendCnt=1;
		} else {
			//�f�[�^�𑗐M
			c=vcom1_getc(pVMC2->pVCom);
			pVMC2->SendCnt=1;
		}
	}//TRMN�𑗂�
	else if(pVMC2->SendMode==vmc2_getmode_TRMN) {
		//vmc2_SendPacTrmn���M
		c=vmc2_PacTrmn[pVMC2->SendCnt];
		++(pVMC2->SendCnt);

		//PacTrmn�𑗂�I�������A���M�p�ɏ�����
		if(pVMC2->SendCnt>=vmc2_PacTrmnSize) {
			pVMC2->SendMode=vmc2_getmode_STRT;
			pVMC2->SendCnt=0;
			pVMC2->SendErr=0;
		}
	}
	return c;
}
//���M�f�[�^��eof�ʒu�łȂ������m�F����
hmLib_boolian vmc2_flowing(vmc2* pVMC2) {
	return !(pVMC2->SendMode==vmc2_getmode_STRT&&pVMC2->SendCnt==0);
}
//��M�������I�ɏI��������
void vmc2_force_end_get(vmc2* pVMC2) {
	if(pVMC2->RecvMode==vmc2_putmode_IDLE)return;
	//�f�[�^��M���Ȃ��������Ƃɂ���
	vcom1_flush(pVMC2->pVCom);

	//vmc2����݂���n��
	pVMC2->RecvMode=vmc2_putmode_IDLE;
	pVMC2->RecvCnt=0;
}
//���M�������I�ɏI��������
void vmc2_force_end_put(vmc2* pVMC2) {
	if(pVMC2->SendMode==vmc2_getmode_STRT)return;

	//�f�[�^���M���Ȃ��������Ƃɂ���
	vcom1_cancel_get(pVMC2->pVCom);

	pVMC2->SendMode=vmc2_getmode_STRT;
	pVMC2->SendCnt=0;
}
//���M�G���[���擾����
unsigned char vmc2_error_of_get(vmc2* pVMC2) { return pVMC2->SendErr; }
//���M�G���[���N���A����
void vmc2_clear_error_of_get(vmc2* pVMC2) { pVMC2->SendErr=0; }
//��M�G���[���擾����
unsigned char vmc2_error_of_put(vmc2* pVMC2) { return pVMC2->RecvErr; }
//��M�G���[���N���A����
void vmc2_clear_error_of_put(vmc2* pVMC2) { pVMC2->RecvErr=0; }
#
#endif
