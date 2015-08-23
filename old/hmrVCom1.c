#ifndef HMR_VCOM1_C_INC
#define HMR_VCOM1_C_INC 104
#
#ifndef HMR_VCOM1_INC
#	include"hmrVCom1.h"
#endif
#define vcom1_recvmode_ID		0
#define vcom1_recvmode_SIZE1	1
#define vcom1_recvmode_SIZE2	2
#define vcom1_recvmode_DATA		3
#define vcom1_recvmode_IGNORE	4
#define vcom1_recvmode_ERROR	5

#define vcom1_sendmode_ID		0
#define vcom1_sendmode_SIZE1	1
#define vcom1_sendmode_SIZE2	2
#define vcom1_sendmode_DATA		3

//vcom1������
void vcom1_initialize(vcom1* pVCom1,
	vcom1_bFp_v Fp_can_move_push,
	vcom1_vFp_pdata Fp_move_push,
	vcom1_bFp_v Fp_can_move_pop,
	vcom1_vFp_pdata Fp_move_pop,
	vcom1_bFp_pstr_strsize Fp_cstring_construct) {
	pVCom1->RecvCh=0x00;
	pVCom1->RecvCnt=0;
	vcom_data_format(&(pVCom1->RecvData));
	pVCom1->RecvMode=vcom1_recvmode_ID;

	pVCom1->SendCnt=0;
	vcom_data_format(&(pVCom1->SendData));
	pVCom1->SendEof=1;
	pVCom1->SendMode=vcom1_sendmode_ID;

	pVCom1->_fp_can_move_push=Fp_can_move_push;
	pVCom1->_fp_move_push=Fp_move_push;
	pVCom1->_fp_can_move_pop=Fp_can_move_pop;
	pVCom1->_fp_move_pop=Fp_move_pop;
	pVCom1->_fp_cstring_construct=Fp_cstring_construct;
}
//vcom1�I�[��
void vcom1_finalize(vcom1* pVCom1) {
	pVCom1->RecvCh=0x00;
	pVCom1->RecvCnt=0;
	vcom_data_destruct(&(pVCom1->RecvData));
	pVCom1->RecvMode=vcom1_recvmode_ID;

	pVCom1->SendCnt=0;
	vcom_data_destruct(&(pVCom1->SendData));
	pVCom1->SendEof=1;
	pVCom1->SendMode=vcom1_sendmode_ID;

	pVCom1->_fp_can_move_push=0;
	pVCom1->_fp_move_push=0;
	pVCom1->_fp_can_move_pop=0;
	pVCom1->_fp_move_pop=0;
	pVCom1->_fp_cstring_construct=0;
}

//======== gate interface =======
//send�ł��邩�H
hmLib_boolian vcom1_can_getc(vcom1* pVCom) {
	//Data����łȂ��Ȃ�A�I��
	if(vcom_data_is_construct(&(pVCom->SendData))) {
		if(pVCom->SendMode==vcom1_sendmode_DATA && pVCom->SendData.Accessible!=0)return pVCom->SendData.Accessible() > pVCom->SendCnt;
		else return 1;
	}
	//Data��ǂݏo���Ȃ��Ȃ�I��
	if(pVCom->_fp_can_move_pop()==0)return 0;

	//Data�ǂݏo��
	pVCom->_fp_move_pop(&(pVCom->SendData));
	pVCom->SendCnt=0;

	//��f�[�^�Ȃ�͂���
	if(!vcom_data_is_construct(&(pVCom->SendData)))return 0;

	//eof�Ȃ�A�t���O�𗧂ĂĎ������ɍs��
	if(vcom_data_eof(&(pVCom->SendData))) {
		vcom_data_destruct(&pVCom->SendData);
		pVCom->SendEof=1;
		return vcom1_can_getc(pVCom);
	}

	//Data�ǂݏo���ɐ�������΁AOK
	return 1;
}
//���M�������1byte�擾����
unsigned char vcom1_getc(vcom1* pVCom) {
	unsigned char c;

	//EOF�t���O���O��
	pVCom->SendEof=0;

	//Data����Ȃ�A�I��
	if(vcom1_can_getc(pVCom)==0) {
		pVCom->SendMode=vcom1_sendmode_ID;
		return 0xC0;
	}

	//ID���M���[�h
	switch(pVCom->SendMode) {
	case vcom1_sendmode_ID:
		pVCom->SendMode=vcom1_sendmode_SIZE1;
		return pVCom->SendData.ID;
	case vcom1_sendmode_SIZE1:
		pVCom->SendMode=vcom1_sendmode_SIZE2;
		return (unsigned char)(cstring_size(&(pVCom->SendData.Data)));
	case vcom1_sendmode_SIZE2:
		if(cstring_size(&(pVCom->SendData.Data))==0) {
			pVCom->SendMode=vcom1_sendmode_ID;
			c=(unsigned char)(cstring_size(&(pVCom->SendData.Data))>>8);
			vcom_data_destruct(&(pVCom->SendData));
			pVCom->SendMode=vcom1_sendmode_ID;
		} else {
			pVCom->SendMode=vcom1_sendmode_DATA;
			c=(unsigned char)(cstring_size(&(pVCom->SendData.Data))>>8);
		}
		pVCom->SendCnt=0;
		return c;
	case vcom1_sendmode_DATA:
		c=cstring_getc(&(pVCom->SendData.Data), (pVCom->SendCnt)++);
		if(pVCom->SendCnt==cstring_size(&(pVCom->SendData.Data))) {
			vcom_data_destruct(&(pVCom->SendData));
			pVCom->SendMode=vcom1_sendmode_ID;
		}
		return c;
	default:
		pVCom->SendMode=vcom1_sendmode_ID;
		return 0;
	}
}
//���M������eof�ʒu=Pac�I�[���ǂ��������m����
hmLib_boolian vcom1_flowing(vcom1* pVCom) {
	vcom1_can_getc(pVCom);
	return pVCom->SendEof==0;
}
//recv�ł��邩�H(�P��return 1)
hmLib_boolian vcom1_can_putc(vcom1* pVCom) {
	//�G���[��Ԃ̎��ɂ́A�劽�}
	if(pVCom->RecvMode==vcom1_recvmode_ERROR)return 1;

	//�o�b�t�@������M�ł���Ȃ�OK
	return pVCom->_fp_can_move_push();
}
//��M�������1byte�^����
void vcom1_putc(vcom1* pVCom, unsigned char c) {
	//�f�[�^id��M��
	if(pVCom->RecvMode==vcom1_recvmode_ID) {
		//NullID�͂����Ȃ��̂Ŗ���
		if(c==vcom_data_id_NULL) {
			//�Ȍ�̃f�[�^��flush������܂ŐM�����Ȃ��̂Ń��[�h���ڍs
			pVCom->RecvMode=vcom1_recvmode_ERROR;
			return;
		}

		//�f�[�^ID���擾
		pVCom->RecvData.ID=c;
		pVCom->RecvData.Ch=pVCom->RecvCh;
		pVCom->RecvData.Err=vcom_data_error_NULL;
		pVCom->RecvData.Accessible=0;

		//ID�Ɋւ���G���[�o�^
		if(!vcom_data_id_is_valid(c))pVCom->RecvData.Err|=vcom_data_error_UNKNOWN;

		//Ch�Ɋւ���G���[�o�^
		if(pVCom->RecvCh>7)pVCom->RecvData.Err|=vcom_data_error_STRANGER;

		//���[�h���T�C�Y��M���[�h(SIZE1)�ֈڍs
		pVCom->RecvMode=vcom1_recvmode_SIZE1;
	}//�f�[�^�T�C�Y��M��1
	else if(pVCom->RecvMode==vcom1_recvmode_SIZE1) {
		//�f�[�^�T�C�Y���擾
		pVCom->RecvCnt=(vcom_size_t)(c);

		//���[�h���T�C�Y��M���[�h(SIZE2)�ֈڍs
		pVCom->RecvMode=vcom1_recvmode_SIZE2;
	}//�f�[�^�T�C�Y��M��2
	else if(pVCom->RecvMode==vcom1_recvmode_SIZE2) {
		//�f�[�^�T�C�Y���擾
		pVCom->RecvCnt|=((vcom_size_t)(c)<<8);

		//�f�[�^�T�C�Y���K�i����������Ă����ꍇ�͖���
		if(pVCom->RecvCnt>vcom_data_MAXSIZE) {
			//�f�[�^�𑗂�
			if(pVCom->_fp_can_move_push()) {
				//�G���[�t���ő��t
				pVCom->RecvData.Err|=vcom_data_error_SIZEOVER;

				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}

			//�Ȍ�̃f�[�^��flush������܂ŐM�����Ȃ��̂Ń��[�h���ڍs
			pVCom->RecvMode=vcom1_recvmode_ERROR;
			return;
		}

		//�f�[�^�T�C�Y��0�̎�
		if(pVCom->RecvCnt==0) {
			//�f�[�^�𑗂�
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}

			//���[�h��ID��M���[�h�ֈڍs
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}

		//�o�b�t�@���m��
		pVCom->_fp_cstring_construct(&(pVCom->RecvData.Data), pVCom->RecvCnt);

		//�f�[�^�T�C�Y�m�ۂɎ��s�����Ƃ�
		if(cstring_size(&(pVCom->RecvData.Data)) != pVCom->RecvCnt) {
			//�m�ێ��s�G���[�o�^
			pVCom->RecvData.Err|=vcom_data_error_FAILNEW;
			//�Ȍ�A�������݂ł��Ȃ��̂ŁA���̃f�[�^������܂Ŗ���
			pVCom->RecvMode=vcom1_recvmode_IGNORE;
			return;
		}

		//���[�h��Pac��M���[�h�ֈڍs
		pVCom->RecvCnt=0;
		pVCom->RecvMode=vcom1_recvmode_DATA;
	}//�f�[�^��M��
	else if(pVCom->RecvMode==vcom1_recvmode_DATA) {
		//�f�[�^��������
		cstring_putc(&(pVCom->RecvData.Data), (pVCom->RecvCnt)++, c);

		//�������݃T�C�Y����ɒB�����ꍇ
		if(pVCom->RecvCnt==cstring_size(&(pVCom->RecvData.Data))) {
			//�f�[�^�𑗂�
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}//���t�ł��Ȃ��ꍇ�́A�j������
			else {
				vcom_data_destruct(&(pVCom->RecvData));
			}

			//���[�h��ID��M���[�h�ֈڍs
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}
	}//�f�[�^������
	else if(pVCom->RecvMode==vcom1_recvmode_IGNORE) {
		--(pVCom->RecvCnt);

		//�S�f�[�^��M���I�����Ƃ�
		if(pVCom->RecvCnt==0) {
			if(pVCom->_fp_can_move_push()) {
				pVCom->_fp_move_push(&(pVCom->RecvData));
				vcom_data_format(&(pVCom->RecvData));
			}//���t�ł��Ȃ��ꍇ�́A�j������
			else {
				vcom_data_destruct(&(pVCom->RecvData));
			}

			//���[�h��ID��M�Ɉڍs
			pVCom->RecvMode=vcom1_recvmode_ID;
			return;
		}
	}
}
//flush����(eof������=Pac�����)
void vcom1_flush(vcom1* pVCom) {
	//��M���̃f�[�^�����݂���ꍇ
	if(pVCom->RecvMode==vcom1_recvmode_DATA || pVCom->RecvMode==vcom1_recvmode_IGNORE) {
		//����M�����G���[�o�^
		pVCom->RecvData.Err|=vcom_data_error_UNDERFLOW;

		//�f�[�^���t
		if(pVCom->_fp_can_move_push()) {
			pVCom->_fp_move_push(&(pVCom->RecvData));
			vcom_data_format(&(pVCom->RecvData));
		}//���t�ł��Ȃ��ꍇ�́A�j������
		else {
			vcom_data_destruct(&(pVCom->RecvData));
		}
	}

	//�f�[�^�������t(���ӁF���s����ƁAPacTrmn���o�^����Ȃ�)
	if(pVCom->_fp_can_move_push()) {
		vcom_data_set_eof(&(pVCom->RecvData));

		pVCom->_fp_move_push(&(pVCom->RecvData));
		vcom_data_format(&(pVCom->RecvData));
	}

	//���[�h��ID��M�Ɉڍs
	pVCom->RecvMode=vcom1_recvmode_ID;
}

//======= vcom functions ========
//���M��ch���擾����
unsigned char vcom1_get_ch(vcom1* pVCom) {
	if(vcom1_can_getc(pVCom)==0)return 0xDD;

	return pVCom->SendData.Ch;
}
//���M���L�����Z������
void vcom1_cancel_get(vcom1* pVCom) {
	pVCom->SendMode=vcom1_sendmode_ID;
	pVCom->SendCnt=0;
}
//���M���X�L�b�v����
void vcom1_skip_get(vcom1* pVCom) {
	pVCom->SendMode=vcom1_sendmode_ID;
	pVCom->SendCnt=0;
	vcom_data_destruct(&pVCom->SendData);
}
//��M����ch���擾����
void vcom1_put_ch(vcom1* pVCom, unsigned char Ch) {
	pVCom->RecvCh=Ch;
}
//��M���L�����Z������
void vcom1_cancel_put(vcom1* pVCom) {
	pVCom->RecvMode=vcom1_recvmode_ID;
	pVCom->RecvCnt=0;
	vcom_data_destruct(&pVCom->RecvData);
}
#
#endif
