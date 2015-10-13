#ifndef HMR_VMC1X_C_INC
#define HMR_VMC1X_C_INC 102
#
#ifndef HMR_VMC1x_INC
#	include"hmrVMC1x.h"
#endif

#include<stdlib.h>

#define vmc1x_putmode_IDLE	0	//�ҋ@��
#define vmc1x_putmode_DATA	1	//��M��

#define vmc1x_getmode_STRT	0
#define vmc1x_getmode_DATA	1
#define vmc1x_getmode_TRMN	2

//static const unsigned char vmc1x_PacStrt[4]={'#', '[', 0x00, ']'};
//static const unsigned char vmc1x_PacStrtSize=4;
static const unsigned char vmc1x_PacTrmn[4]={'#', '#', 0x0d, 0x0a};
static const unsigned char vmc1x_PacTrmnSize=4;

//vmc1x������������
void vmc1x_initialize(vmc1x* pVMC1x, vcom* pVCom, const unsigned char VMCID_[3], const unsigned char TVMCID_[3]) {
	//VCom,Ch
	pVMC1x->pVCom=pVCom;
	pVMC1x->VMCID[0]=VMCID_[0];
	pVMC1x->VMCID[1]=VMCID_[1];
	pVMC1x->VMCID[2]=VMCID_[2];
	pVMC1x->TVMCID[0]=TVMCID_[0];
	pVMC1x->TVMCID[1]=TVMCID_[1];
	pVMC1x->TVMCID[2]=TVMCID_[2];

	//��M���ϐ�
	pVMC1x->RecvErr=vmc1x_puterr_NULL;
	pVMC1x->RecvMode=vmc1x_putmode_IDLE;
	pVMC1x->RecvCnt=0;
	pVMC1x->RecvCh=0xFF;

	//���M���ϐ�
	pVMC1x->SendErr=vmc1x_geterr_NULL;
	pVMC1x->SendMode=vmc1x_getmode_STRT;
	pVMC1x->SendCnt=0;
}
//vmc1x���I�[������
void vmc1x_finalize(vmc1x* pVMC1x) {
	//��M���[�h�̂܂܂̏ꍇ
	if(pVMC1x->RecvMode==vmc1x_putmode_DATA) {
		vcom1_flush(pVMC1x->pVCom);
		pVMC1x->RecvMode=vmc1x_putmode_IDLE;
		pVMC1x->RecvCnt=0;
	}
	//���M���[�h�̂܂܂̏ꍇ
	if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		vcom1_cancel_get(pVMC1x->pVCom);
		pVMC1x->SendMode=vmc1x_getmode_STRT;
		pVMC1x->SendCnt=0;
	}
}
//�t���b�V������i�����������Ȃ��j
void vmc1x_flush(vmc1x* pVMC1x) { return; }
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian vmc1x_can_putc(vmc1x* pVMC1x) { 
	if(pVMC1x->RecvMode==vmc1x_putmode_DATA) {
		return vcom1_can_putc(pVMC1x->pVCom);
	}
	return 1;
}
//��M�f�[�^�𓊂������
void vmc1x_putc(vmc1x* pVMC1x, unsigned char c) {
	unsigned char cnt;
	//���M���[�h(vmc1x_RecvPacStrt�҂�)
	if(pVMC1x->RecvMode==vmc1x_putmode_IDLE) {
		//�擪���ʋL���̎�M��
		if(pVMC1x->RecvCnt==0) {
			if(c=='#') {
				++(pVMC1x->RecvCnt);
			}
		}//�擪���ʋL���̎�M�� ID�m�F��
		else {
			//ID����v
			if(c==pVMC1x->VMCID[pVMC1x->RecvCnt-1]) {
				++(pVMC1x->RecvCnt);
				if(pVMC1x->RecvCnt==4) {
					//���[�h��Pac��M���[�h�ֈڍs
					pVMC1x->RecvMode=vmc1x_putmode_DATA;
					pVMC1x->RecvCnt=0;
				}
			}//ID�͕s��v�����擪���ʋL������M
			else if(c=='#') {
				pVMC1x->RecvCnt=1;
			}//����ȊO
			else {
				pVMC1x->RecvCnt=0;
			}
		}
	}//��M���[�h(PacTrmn�҂�)
	else {
		//��M������putmodeCnt�Ԗڂ�PacTrmn�����ƈ�v
		if(vmc1x_PacTrmn[pVMC1x->RecvCnt]==c) {
			//�J�E���^�[���Z
			++(pVMC1x->RecvCnt);

			//�Svmc1x_RecvPacTrmn��M
			if(pVMC1x->RecvCnt==vmc1x_PacTrmnSize) {
				//�I���ʒm
				vcom1_flush(pVMC1x->pVCom);

				//���[�h��Pac��M���[�h�ֈڍs
				pVMC1x->RecvMode=vmc1x_putmode_IDLE;
				pVMC1x->RecvCnt=0;
			}
		}//��M������0�Ԗڂ�PacTrmn�����ƈ�v
		else if(vmc1x_PacTrmn[0]==c) {
			for(cnt=0; cnt<pVMC1x->RecvCnt;++cnt) {
				//��M�ł��Ȃ��ꍇ
				if(vcom1_can_putc(pVMC1x->pVCom)==0) {
					//�G���[����
					pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
					//��M���X�g�b�v
					vcom1_flush(pVMC1x->pVCom);
					//���[�h��߂�
					pVMC1x->RecvMode=vmc1x_putmode_IDLE;
					pVMC1x->RecvCnt=0;
					return;
				}

				//�f�[�^�������n��
				vcom1_putc(pVMC1x->pVCom, vmc1x_PacTrmn[cnt]);
			}
			//�J�E���^�[��1��(1�����͈�v��������)
			pVMC1x->RecvCnt=1;
		}//��M������PacTrmn�����ƕs��v
		else{
			for(cnt=0; cnt<pVMC1x->RecvCnt; ++cnt) {
				//��M�ł��Ȃ��ꍇ
				if(vcom1_can_putc(pVMC1x->pVCom)==0) {
					//�G���[����
					pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
					//��M���X�g�b�v
					vcom1_flush(pVMC1x->pVCom);
					//���[�h��߂�
					pVMC1x->RecvMode=vmc1x_putmode_IDLE;
					pVMC1x->RecvCnt=0;
					return;
				}

				//�f�[�^�������n��
				vcom1_putc(pVMC1x->pVCom, vmc1x_PacTrmn[cnt]);
			}
			//�J�E���^�[���Z�b�g
			pVMC1x->RecvCnt=0;

			//��M�ł��Ȃ��ꍇ
			if(vcom1_can_putc(pVMC1x->pVCom)==0) {
				//�G���[����
				pVMC1x->RecvErr=vmc1x_puterr_RECVREJECTED;
				//��M���X�g�b�v
				vcom1_flush(pVMC1x->pVCom);
				//���[�h��߂�
				pVMC1x->RecvMode=vmc1x_putmode_IDLE;
				pVMC1x->RecvCnt=0;
				return;
			}
			
			//�f�[�^�������n��
			vcom1_putc(pVMC1x->pVCom, c);

		}
	}
}
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian vmc1x_can_getc(vmc1x* pVMC1x) {
	if(pVMC1x->SendMode==vmc1x_getmode_STRT&&pVMC1x->SendCnt==0) {
		//����M�ł���f�[�^�����݂��邩
		return vcom1_can_getc(pVMC1x->pVCom);
	}else if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		//���M�ςݏ�ԃf�[�^�����ɂ���ꍇ�́A����TRMN�̂͂��Ȃ̂�1
		if(vcom1_flowing(pVMC1x->pVCom)==0 && pVMC1x->SendCnt!=0)return 1;
		return vcom1_can_getc(pVMC1x->pVCom);
	}

	return 1;
}
//���M�f�[�^���Ăяo��
unsigned char vmc1x_getc(vmc1x* pVMC1x) {
	unsigned char c=0x00;
	//�񑗐M���[�h(vmc1x_SendPacStrt�҂�)
	if(pVMC1x->SendMode==vmc1x_getmode_STRT) {
		//�ŏ��̕����̏ꍇ
		if(pVMC1x->SendCnt==0) {
			c='#';
			++(pVMC1x->SendCnt);
		//ID���t�t�F�[�Y
		} else {
			c=pVMC1x->TVMCID[pVMC1x->SendCnt-1];
			++(pVMC1x->SendCnt);
			//PacStrt�𑗂�I�������A���M�p�ɏ�����
			if(pVMC1x->SendCnt>=4) {
				pVMC1x->SendMode=vmc1x_getmode_DATA;
				pVMC1x->SendCnt=0;
			}
		}
	}//Data���M���[�h
	else if(pVMC1x->SendMode==vmc1x_getmode_DATA) {
		//�f�[�^���I�������ꍇ�ATRMN�Ɉڍs����
		if(vcom1_flowing(pVMC1x->pVCom)==0 && pVMC1x->SendCnt!=0) {
			//TRMN���M
			pVMC1x->SendMode=vmc1x_getmode_TRMN;
			c=vmc1x_PacTrmn[0];
			pVMC1x->SendCnt=1;
		}//send�Ɏ��s����ꍇ�͋����I��
		else if(vcom1_can_getc(pVMC1x->pVCom)==0) {
			//�G���[����
			pVMC1x->SendErr=vmc1x_geterr_SENDREJECTED;

			//vcom�㏈��
			vcom1_cancel_get(pVMC1x->pVCom);

			//TRMN���������M
			pVMC1x->SendMode=vmc1x_getmode_TRMN;
			c=vmc1x_PacTrmn[0];
			pVMC1x->SendCnt=1;
		} else {
			//�f�[�^�𑗐M
			c=vcom1_getc(pVMC1x->pVCom);
			pVMC1x->SendCnt=1;
		}
	}//TRMN�𑗂�
	else if(pVMC1x->SendMode==vmc1x_getmode_TRMN) {
		//vmc1x_SendPacTrmn���M
		c=vmc1x_PacTrmn[pVMC1x->SendCnt];
		++(pVMC1x->SendCnt);

		//PacTrmn�𑗂�I�������A���M�p�ɏ�����
		if(pVMC1x->SendCnt>=vmc1x_PacTrmnSize) {
			pVMC1x->SendMode=vmc1x_getmode_STRT;
			pVMC1x->SendCnt=0;
			pVMC1x->SendErr=0;
		}
	}
	return c;
}
//���M�f�[�^��eof�ʒu�łȂ������m�F����
hmLib_boolian vmc1x_flowing(vmc1x* pVMC1x) {
	return !(pVMC1x->SendMode==vmc1x_getmode_STRT&&pVMC1x->SendCnt==0);
}
//��M�������I�ɏI��������
void vmc1x_force_end_recv(vmc1x* pVMC1x) {
	if(pVMC1x->RecvMode==vmc1x_putmode_IDLE)return;
	//�f�[�^��M���Ȃ��������Ƃɂ���
	vcom1_flush(pVMC1x->pVCom);

	//vmc1x����݂���n��
	pVMC1x->RecvMode=vmc1x_putmode_IDLE;
	pVMC1x->RecvCnt=0;
}
//���M�������I�ɏI��������
void vmc1x_force_end_send(vmc1x* pVMC1x) {
	if(pVMC1x->SendMode==vmc1x_getmode_STRT)return;

	//�f�[�^���M���Ȃ��������Ƃɂ���
	vcom1_cancel_get(pVMC1x->pVCom);

	pVMC1x->SendMode=vmc1x_getmode_STRT;
	pVMC1x->SendCnt=0;
}
//���M�G���[���擾����
unsigned char vmc1x_error_of_get(vmc1x* pVMC1x) { return pVMC1x->SendErr; }
//���M�G���[���N���A����
void vmc1x_clear_error_of_get(vmc1x* pVMC1x) { pVMC1x->SendErr=0; }
//��M�G���[���擾����
unsigned char vmc1x_error_of_put(vmc1x* pVMC1x) { return pVMC1x->RecvErr; }
//��M�G���[���N���A����
void vmc1x_clear_error_of_put(vmc1x* pVMC1x) { pVMC1x->RecvErr=0; }
#
#endif
