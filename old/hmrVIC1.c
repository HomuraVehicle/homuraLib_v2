#ifndef HMR_VIC1_C_INC
#define HMR_VIC1_C_INC 102
#
#ifndef CRC8_INC
#	include"crc8.h"
#endif
#ifndef HMR_VIC1_INC
#	include"hmrVIC1.h"
#endif
#include<stdlib.h>

#define vic1_MaxTimeOutNum 5
//ch
#define vic1_ch_NULL		0xFF

//Cmd
#define vic1_cmd_ACK		0x00
#define vic1_cmd_NACK		0x10
#define vic1_cmd_START		0x20
#define vic1_cmd_STOP		0x30
#define vic1_cmd_DATA		0x40
#define vic1_cmd_ESC		0x50
#define vic1_cmd_WHAT		0x60
#define vic1_cmd_ERR		0xF0
#define vic1_cmd_NULL		0xFF

//Mode
#define vic1_mode_IDLE		0x00
#define vic1_mode_START		0x10
#define vic1_mode_SEND		0x20
#define vic1_mode_RESEND	0x21
#define vic1_mode_STOP		0x30
#define vic1_mode_RECV		0x40

//========== vic1::buf ===========
//��[�ł�������[
void x_vic1_buf_load(vic1* pVIC1) {
	//��M�\�ł��������s
	while(vcom1_can_getc(pVIC1->pVCom)){
		//���ۂɎ�M
		*((pVIC1->_buf_VComItr)++) = vcom1_getc(pVIC1->pVCom);

		//�o�b�t�@�������ς��ɂȂ����ꍇ�A���邢��vcom����̑��M���r�؂ꂽ�ꍇ
		if(pVIC1->_buf_VComItr+1 == pVIC1->_buf_End || vcom1_flowing(pVIC1->pVCom)==0) {
			//crc8���v�Z���āA�����ɏ�������
			*(pVIC1->_buf_VComItr) = crc8_puts(0x00,pVIC1->_buf_Begin,pVIC1->_buf_VComItr - pVIC1->_buf_Begin);
                        ++(pVIC1->_buf_VComItr);
			break;
		}
	}
}
//�o�b�t�@������
void _vic1_buf_initialize(vic1* pVIC1, unsigned char* BufBegin, unsigned char* BufEnd) {
	pVIC1->_buf_Begin=BufBegin;
	pVIC1->_buf_End=BufEnd;
	pVIC1->_buf_VComItr=BufBegin;
	pVIC1->_buf_GateItr=BufBegin;
}
//�o�b�t�@�I�[��
void _vic1_buf_finalize(vic1* pVIC1) {
	pVIC1->_buf_Begin=0;
	pVIC1->_buf_End=0;
	pVIC1->_buf_VComItr=0;
	pVIC1->_buf_GateItr=0;
}
//�o�b�t�@�֏������񂾃f�[�^������
void _vic1_buf_flush(vic1* pVIC1) {
	unsigned char CRC8;

	//���łɎ��s���Ă���Ζ���
	if(pVIC1->Fail==1)return;
	
	//����������M���Ă��Ȃ���Ζ���
	if(pVIC1->_buf_GateItr==pVIC1->_buf_Begin) {
		//���s�������Ƃ�ʒm
		pVIC1->Fail=1;
		//Iterator�������ʒu��
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;

		return;
	}

	//1byte�߂�
	--(pVIC1->_buf_GateItr);

	//GateItr�܂ł̃`�F�b�N�T�������Z
	CRC8=crc8_puts(0x00, pVIC1->_buf_Begin, pVIC1->_buf_GateItr - pVIC1->_buf_Begin);

	//�`�F�b�N�T���Ɏ��s���Ă���ꍇ�͔j��
	if(*(pVIC1->_buf_GateItr)!=CRC8) {
		//���s�������Ƃ�ʒm
		pVIC1->Fail=1;
		//Iterator�������ʒu��
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;

		return;
	}

	//�f�[�^�Z�[�u
	//VCom�ɏ������������
	pVIC1->_buf_VComItr = pVIC1->_buf_Begin;
	while(pVIC1->_buf_VComItr!= pVIC1->_buf_GateItr) {
		if(vcom1_can_putc(pVIC1->pVCom)==0) {
			//�G���[����
			pVIC1->Err=vic1_error_SAVE_REJECTED;
			break;
		}
		vcom1_putc(pVIC1->pVCom, *(pVIC1->_buf_VComItr++));
	}

	//Iterator�������ʒu��
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
}
//�o�b�t�@�̓����f�[�^�����ׂĔj�����Avcom�ɂ�Packet�I����ʒm
void _vic1_buf_errorflush(vic1* pVIC1) {
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
	vcom1_flush(pVIC1->pVCom);
}
//�o�b�t�@�ɏ������݉\��
hmLib_boolian _vic1_buf_can_putc(vic1* pVIC1) {
	//�o�b�t�@�ɋ󂫂�����ꍇ
	//	���M�ƈ����flush����܂łǂꂪCheckSum���킩��Ȃ��̂ŁA
	//	1byte�����Ă����K�v���Ȃ��_�ɒ���
	return pVIC1->_buf_GateItr < pVIC1->_buf_End;
}
//�o�b�t�@�֏�������
void _vic1_buf_putc(vic1* pVIC1, unsigned char c) {
	//�������݉\�łȂ��ꍇ�A�G���[����
	if(!_vic1_buf_can_putc(pVIC1)) {
		//�G���[����
		pVIC1->Err=vic1_error_OVERFLOW;
		//���s�������Ƃ�ʒm
		pVIC1->Fail=1;
		//Iterator�������ʒu��
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	}

	//�f�[�^��������
	*((pVIC1->_buf_GateItr)++)=c;
}
//�o�b�t�@�ւ���̓ǂݏo������؂�ʒu�ɂ��邩
hmLib_boolian _vic1_buf_flowing(vic1* pVIC1) {
	//�������ݍς݃o�b�t�@�����܂œǂݏo���Ă��Ȃ�
	//���邢�́A�o�b�t�@�����܂ŏ�������ł��炸���f�[�^�͌p�����Ă���ivcom�����M�]�n������j
	return pVIC1->_buf_GateItr < pVIC1->_buf_VComItr
		|| (pVIC1->_buf_VComItr < pVIC1->_buf_End && vcom1_flowing(pVIC1->pVCom));
}
//�o�b�t�@����̓ǂݏo���\��
hmLib_boolian _vic1_buf_can_getc(vic1* pVIC1) {
	//�������ݍς݃o�b�t�@�����܂œǂݏo���Ă��Ȃ�
	//���邢�́Avcom�����M�\�ł���
	return pVIC1->_buf_GateItr != pVIC1->_buf_VComItr
		|| vcom1_can_getc(pVIC1->pVCom);
}
//�o�b�t�@����̓ǂݏo��
unsigned char _vic1_buf_getc(vic1* pVIC1) {
	//�o�b�t�@�ւ̕�[�]�n������ꍇ
	if(pVIC1->_buf_VComItr != pVIC1->_buf_End && vcom1_flowing(pVIC1->pVCom)){
		//��[�ł�������[
		x_vic1_buf_load(pVIC1);
	}//��[�]�n���Ȃ����S�đ��M�������� => �V�K�o�b�t�@������ꍇ
	else if(pVIC1->_buf_GateItr == pVIC1->_buf_VComItr) {
		//Iterator�������ʒu��
		pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
		pVIC1->_buf_VComItr=pVIC1->_buf_Begin;

		//��[�ł�������[
		x_vic1_buf_load(pVIC1);
	}

	//�܂��c���Ă���ꍇ
	if(pVIC1->_buf_GateItr < pVIC1->_buf_VComItr) {
		return *((pVIC1->_buf_GateItr)++);
	} else {
		//�G���[����
		pVIC1->Err=vic1_error_OVERREAD;
		return 0;
	}
}
//�o�b�t�@�ւ̏�������/�ǂݍ��݂��I�����A�o�b�t�@���N���A����
void _vic1_buf_clear(vic1* pVIC1) {
	pVIC1->_buf_GateItr=pVIC1->_buf_Begin;
	pVIC1->_buf_VComItr=pVIC1->_buf_Begin;
}
//�o�b�t�@�ւ̓ǂݍ��݂��Ȃ��������Ƃɂ���
void _vic1_buf_cancel_get(vic1* pVIC1) { pVIC1->_buf_GateItr=pVIC1->_buf_Begin; }

//========== vic1::gate ===========
//-------- Mode�J��4�֐� ----------------
//�R�}���h���M������
void _vic1_inform_send(vic1* pVIC1) {
	//ActiveCmd�ɓo�^
	pVIC1->ActiveCmd=pVIC1->Send;

	switch(pVIC1->Mode) {
	case vic1_mode_IDLE:
		//�Ђ܂ɂȂ������A�b�������l�������ꍇ
		if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
			pVIC1->TergetCh=pVIC1->NextTergetCh;
			pVIC1->NextTergetCh=vic1_ch_NULL;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_WHAT;

		}//�{���ɒP�ɉɂȏꍇ
		else {
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_NULL;
		}
		break;
	case vic1_mode_RESEND:
		//com���瑗�M�f�[�^���擾�\�ȏꍇ
		if(_vic1_buf_can_getc(pVIC1)) {
			//�f�[�^���M�J�n
			pVIC1->Mode=vic1_mode_SEND;
			pVIC1->SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
			pVIC1->Send=vic1_cmd_DATA;
		} else {
			//Stop���M
			pVIC1->Mode=vic1_mode_STOP;
			pVIC1->Send=vic1_cmd_STOP;
			pVIC1->SendCnt=0;
		}
		break;
	default:
		//���M���I���đҋ@��Ԃɓ���
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_NULL;
		break;
	}
}
//�������̃R�}���h��M����
void _vic1_inform_recv(vic1* pVIC1) {
	pVIC1->RecvCnt=0;
	//�Ђ܂ɂ��Ă���Ƃ�
	if(pVIC1->Mode==vic1_mode_IDLE) {
		//���肩��̃R�}���h���e�ŕ���
		switch(pVIC1->Recv) {
			//���肪Start�v�������Ă����ꍇ
		case vic1_cmd_START:
			//Ack��Ԃ��A���g��Recv�ɂ���
			pVIC1->Mode=vic1_mode_RECV;
			_vic1_buf_clear(pVIC1);
			pVIC1->TergetCh=pVIC1->RecvCh;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		case vic1_cmd_ERR:
			//��������
			break;
		default:
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->TergetCh=pVIC1->RecvCh;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
		return;
	}

	//�ʐM���ɑ��肪�m��Ȃ��l����ʐM�������ꍇ
	if(pVIC1->RecvCh!=pVIC1->TergetCh) {
		//Start�v���������ꍇ�A�؂�ւ��\��
		if(pVIC1->Recv==vic1_cmd_START && pVIC1->NextTergetCh>pVIC1->RecvCh) {
			pVIC1->NextTergetCh=pVIC1->RecvCh;
			return;
		}
		//����ȊO�Ȃ�A����
	}

	//�Ԏ��������̂ŁA�ЂƂ܂��^�C���A�E�g���Z�b�g
	pVIC1->Fp_timeout_restart();
	//�Ԏ��������̂ŁA�^�C���A�E�g�̃J�E���g��߂�
	pVIC1->TimeOutCnt=0;

	//�G���[�̏ꍇ�͖ⓚ���p��IDLE��
	if(pVIC1->Recv == vic1_cmd_ERR){
		//���ɘb�������鑊�肪�o�^����Ă����ꍇ
		if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
			//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->TergetCh=pVIC1->NextTergetCh;
			pVIC1->NextTergetCh=vic1_ch_NULL;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_WHAT;
		}//���̗\����Ȃ��ꍇ
		else {
			//���[�h��Idle�ɂ��A�ق�
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_NULL;
		}
	}//Recv��
	else if(pVIC1->Mode==vic1_mode_RECV) {
		//�f�[�^�𑗂��Ă����ꍇ
		switch(pVIC1->Recv) {
		case vic1_cmd_START:
			//ACK���M
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		case vic1_cmd_DATA:
			//�t���b�V��
			_vic1_buf_flush(pVIC1);

			//��M�ɐ������Ă��邩�m�F
			if(pVIC1->Fail==0) {
				//ACK���M
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			} else {
				//�t���O�����낷
				pVIC1->Fail=0;
				//NACK���M
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NACK;
			}
			break;
		case vic1_cmd_STOP:
			//�p�P�b�g�I���ʒm
			_vic1_buf_errorflush(pVIC1);

			//com���瑗�M�f�[�^���擾�\�ȏꍇ(�܂�Ԃ��Ȃ̂�flowing�͋C�ɂ��Ȃ�)
			if(_vic1_buf_can_getc(pVIC1)) {
				//����M����ւ��v��
				pVIC1->Mode=vic1_mode_START;
				pVIC1->Fp_timeout_restart();
				pVIC1->Fp_timeout_enable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NACK;
			}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
			else {
				//�I���󂯓���
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			}
			break;
		default:
			//�p�P�b�g�I���ʒm
			_vic1_buf_errorflush(pVIC1);

			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			break;
		}

		//ReecvMode�̎��ɂ́A�f�[�^���o�b�t�@�ɂ���Ă��܂��Ă���̂ŁA���Z�b�g�B
		_vic1_buf_clear(pVIC1);
		pVIC1->Fail=0;
	}//Start�R�}���h�ɑ΂���ԓ�
	else if(pVIC1->Mode==vic1_mode_START && (pVIC1->ActiveCmd==vic1_cmd_START || pVIC1->ActiveCmd==vic1_cmd_NACK)) {
		switch(pVIC1->Recv) {
		//���肪��������ꍇ
		case vic1_cmd_ACK:
			//com���瑗�M�f�[�^���擾�\�ȏꍇ
			if(_vic1_buf_can_getc(pVIC1)) {
				//���M���������ꍇ
				if(pVIC1->Send!=vic1_cmd_NULL) {
					//ESC����U���M
					pVIC1->Mode=vic1_mode_RESEND;
					pVIC1->SendCnt=0;
					pVIC1->Send=vic1_cmd_ESC;
				}//�����M�\�ȏꍇ
				else {
					//DATA�𑗐M
					pVIC1->Mode=vic1_mode_SEND;
					pVIC1->SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
					pVIC1->Send=vic1_cmd_DATA;
				}
			}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
			else {
				//Stop���M
				pVIC1->Mode=vic1_mode_STOP;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_STOP;
			}
			break;
			//���肪���ۂ��Ă����ꍇ
		case vic1_cmd_NACK:
			//���ɘb�������鑊�肪�o�^����Ă����ꍇ
			if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
				//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->TergetCh=pVIC1->NextTergetCh;
				pVIC1->NextTergetCh=vic1_ch_NULL;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_WHAT;
			}//���̗\����Ȃ��ꍇ
			else {
				//���[�h��Idle�ɂ��A�ق�
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NULL;
			}
			break;
			//���肪Start�v�����d�˂Ă����ꍇ
		case vic1_cmd_START:
			//�����莩���̕����傫��ch�̂Ƃ�
			if(pVIC1->RecvCh < pVIC1->Ch) {
				//Ack��Ԃ��A���g��Recv�ɂ���
				pVIC1->Mode=vic1_mode_RECV;
				pVIC1->Fp_timeout_disable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ACK;
			}
			//�������ꍇ�͖���
			break;
			//����ȊO�̂��Ƃ������Ă���ꍇ
		default:
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			break;
		}
	}//DATA�R�}���h�ɑ΂���ԓ�
	else if(pVIC1->Mode==vic1_mode_SEND && pVIC1->ActiveCmd==vic1_cmd_DATA) {
		switch(pVIC1->Recv) {
			//���肪��M�ɐ������Ă����ꍇ
		case vic1_cmd_ACK:
			//com���瑗�M�f�[�^���擾�\�ȏꍇ�iDATA�Ɍp����������ꍇ�Ɍ���j
			if(vcom1_can_getc(pVIC1->pVCom) && vcom1_flowing(pVIC1->pVCom)) {
				//���M���������ꍇ
				if(pVIC1->Send!=vic1_cmd_NULL) {
					//ESC����U���M
					pVIC1->Mode=vic1_mode_RESEND;
					pVIC1->SendCnt=0;
					pVIC1->Send=vic1_cmd_ESC;
				}//�����M�\�ȏꍇ
				else {
					//DATA�𑗐M
					pVIC1->Mode=vic1_mode_SEND;
					pVIC1->SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
					pVIC1->Send=vic1_cmd_DATA;
				}
			}//com���瑗�M�f�[�^���擾�ł��Ȃ��ꍇ
			else {
				//Stop���M
				pVIC1->Mode=vic1_mode_STOP;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_STOP;
			}
			break;
			//���肪��M�Ɏ��s���Ă����ꍇ
		case vic1_cmd_NACK:
			//�o�b�t�@�̓ǂݏo���ʒu��������
			_vic1_buf_cancel_get(pVIC1);
			//���M���������ꍇ
			if(pVIC1->Send!=vic1_cmd_NULL) {
				//ESC����U���M
				pVIC1->Mode=vic1_mode_RESEND;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_ESC;
			}//�����M�\�ȏꍇ
			else {
				//DATA�𑗐M
				pVIC1->Mode=vic1_mode_SEND;
				pVIC1->SendCnt=0xFF;		//Data���M�̏ꍇ�̂݁ASendCnt��ff�ɃZ�b�g����K�v������
				pVIC1->Send=vic1_cmd_DATA;
			}
			break;
		default:
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
	}//Stop�R�}���h�ɑ΂���ԓ�
	else if(pVIC1->Mode==vic1_mode_STOP && pVIC1->ActiveCmd==vic1_cmd_STOP) {
		switch(pVIC1->Recv) {
		//���肪��������ꍇ
		case vic1_cmd_ACK:
			//���ɘb�������鑊�肪�o�^����Ă����ꍇ
			if(pVIC1->NextTergetCh!=vic1_ch_NULL) {
				//WHAT�R�}���h��ԑ����A������IDLE�ɂȂ�
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->TergetCh=pVIC1->NextTergetCh;
				pVIC1->NextTergetCh=vic1_ch_NULL;
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_WHAT;
			}//���̗\����Ȃ��ꍇ
			else {
				//���[�h��Idle�ɂ��A�ق�
				pVIC1->Mode=vic1_mode_IDLE;
				pVIC1->Fp_timeout_disable();
				pVIC1->SendCnt=0;
				pVIC1->Send=vic1_cmd_NULL;
			}
			break;
		//���肪���ۂ��Ă����ꍇ
		case vic1_cmd_NACK:
			//���g�͎�M���[�h�ɂ��A����Ɏ���̈ӎv��`����
			pVIC1->Mode=vic1_mode_RECV;
			_vic1_buf_clear(pVIC1);	//�o�b�t�@���N���A���Ď�M�ɔ�����
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ACK;
			break;
		default:
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->Fp_timeout_disable();
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
		}
	} else {
		//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
		pVIC1->Mode=vic1_mode_IDLE;
		pVIC1->Fp_timeout_disable();
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_ERR;
	}
}
//�f�[�^�̗L�����m�F���āAIDLE����START�֑J�ڂ���֐�
hmLib_boolian _vic1_checkData(vic1* pVIC1) {
	//IDLE�ł���A���M���R�}���h���Ȃ��A���M�������f�[�^������A�̂R�������������ꂽ�ꍇ�̂ݔ���
	if(pVIC1->Mode==vic1_mode_IDLE && pVIC1->Send==vic1_cmd_NULL && vcom1_can_getc(pVIC1->pVCom)) {
		//ch�擾���Astart���[�h�ֈڍs
		pVIC1->TergetCh=vcom1_get_ch(pVIC1->pVCom);
		pVIC1->Mode=vic1_mode_START;
		pVIC1->Fp_timeout_restart();
		pVIC1->Fp_timeout_enable();	//START,REDATA,DATA,STOP�͍đ��@�\�t��
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_START;
		return hmLib_true;
	}
	return hmLib_false;
}
//timeout�����[�h���ꕔ�ς��Ă���ł�����
//vic1��timeout�R�}���h�đ��֐�
void vic1_timeout(vic1* pVIC1) {
	if(pVIC1->Mode==vic1_mode_START) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			vcom1_skip_get(pVIC1->pVCom);
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			return;
		}
		pVIC1->Send=vic1_cmd_START;
		pVIC1->SendCnt=0;
	} else if(pVIC1->Mode==vic1_mode_SEND) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			vcom1_skip_get(pVIC1->pVCom);
			//ESC����U���M
			pVIC1->Mode=vic1_mode_RESEND;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ESC;
			return;
		}
		//ESC����U���M
		pVIC1->Mode=vic1_mode_RESEND;
		pVIC1->SendCnt=0;
		pVIC1->Send=vic1_cmd_ESC;
	} else if(pVIC1->Mode==vic1_mode_STOP) {
		if(pVIC1->TimeOutCnt>=vic1_MaxTimeOutNum) {
			pVIC1->TimeOutCnt=0;
			//�������Ȃ��Ƃ������Ă���̂ŁA�����Ă�����
			pVIC1->Mode=vic1_mode_IDLE;
			pVIC1->SendCnt=0;
			pVIC1->Send=vic1_cmd_ERR;
			return;
		}		pVIC1->Send=vic1_cmd_STOP;
		pVIC1->SendCnt=0;
	}
	++(pVIC1->TimeOutCnt);
}

//========== vic1 ��{�֐� ==========
//vic1������������ �����́A�����̃`�����l���ԍ�(0x00-0x0F)�A����̃`�����l���ԍ��A�^���鑗��M�p�Œ蒷�o�b�t�@�A�h���X
void vic1_initialize(vic1* pVIC1, vcom1* pVCom1, const unsigned char Ch, unsigned char* BufBegin, unsigned char* BufEnd, hmLib_vFp_v FpTimeoutRestart, hmLib_vFp_v FpTimeoutEnable, hmLib_vFp_v FpTimeoutDisable) {
	pVIC1->pVCom=pVCom1;

	_vic1_buf_initialize(pVIC1, BufBegin,BufEnd);

	pVIC1->Fp_timeout_disable=FpTimeoutDisable;
	pVIC1->Fp_timeout_enable=FpTimeoutEnable;
	pVIC1->Fp_timeout_restart=FpTimeoutRestart;

	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Ch=Ch;
	pVIC1->Err=vic1_error_NULL;
	pVIC1->Fail=0x00;
	pVIC1->TimeOutCnt=0;

	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->NextTergetCh=vic1_ch_NULL;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->Send=vic1_cmd_NULL;
	pVIC1->SendCnt=0;

	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCh=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
//	maintask_start(vic1_task, 0, 0, 0);
}
//vic1���I�[������
void vic1_finalize(vic1* pVIC1){
	if(pVIC1->Fp_timeout_disable)pVIC1->Fp_timeout_disable();

	pVIC1->pVCom=0;

	pVIC1->Ch=vic1_ch_NULL;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->NextTergetCh=vic1_ch_NULL;

	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->Err=0;

	_vic1_buf_finalize(pVIC1);

	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Send=vic1_cmd_NULL;
	pVIC1->SendCnt=0;
	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
	pVIC1->RecvCh=vic1_ch_NULL;

//	maintask_stop(vic1_task);
}


//========== gate interface =================
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian  vic1_can_putc(vic1* pVIC1){
	//��M�f�[�^�͏�Ɏ󂯓���\�i�ǂݎ̂Ă邩�ǂ����͕ʁj
	return hmLib_true;
}
//��M�f�[�^1byte�ڔ��ʊ֐�
void x_vic1_putc_1byte(vic1* pVIC1, unsigned char c) {
	//��������Ch���H
	if(pVIC1->Ch==(c&0x0F)) {
		pVIC1->RecvCh=(c>>4);
		++(pVIC1->RecvCnt);
	}//��L�ȊO�Ȃ�A���Ȃ��Ƃ������ɂƂ��ăR�}���h�ł͂Ȃ�
	else {
		//��M���Ȃ�f�[�^���������Ă���
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c);
		}
	}
}
//��M�f�[�^2byte�ڔ��ʊ֐�
void x_vic1_putc_2byte(vic1* pVIC1, unsigned char c1, unsigned char c2) {
	//���ʐM���̑���̏ꍇ
	if(pVIC1->RecvCh==pVIC1->TergetCh) {
		//�Ƃɂ����R�����g�͕����Ă���
		pVIC1->Recv=c2;
		++(pVIC1->RecvCnt);
	}//�m��Ȃ��l����Start�R�}���h�������ꍇ
	else if(c2==vic1_cmd_START) {
		//����������Ă���
		pVIC1->Recv=c2;
		++(pVIC1->RecvCnt);
	}//��L�ȊO�Ȃ�A���Ȃ��Ƃ������ɂƂ��ăR�}���h�ł͂Ȃ�
	else {
		//�J�E���^��߂�
		pVIC1->RecvCnt=0;

		//��M���Ȃ�f�[�^���������Ă���
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}
		x_vic1_putc_1byte(pVIC1, c2);
	}
}
//��M�f�[�^3byte�ڔ��ʊ֐�
void x_vic1_putc_3byte(vic1* pVIC1, unsigned char c1, unsigned char c2, unsigned char c3) {
	//Trmn1
	if(c3==0x0d) {
		++(pVIC1->RecvCnt);
	}//��L�ȊO�Ȃ�A���Ȃ��Ƃ�1byte�ڂ̕����͎����ɂƂ��ăR�}���h�ł͂Ȃ�
	else {
		//�J�E���^��߂�
		pVIC1->RecvCnt=0;

		//��M���Ȃ�f�[�^���������Ă���
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}
		x_vic1_putc_1byte(pVIC1, c2);

		if(pVIC1->RecvCnt==0) {
			x_vic1_putc_1byte(pVIC1, c3);
		} else if(pVIC1->RecvCnt==1) {
			x_vic1_putc_2byte(pVIC1, c2, c3);
		}
	}
}
//��M�f�[�^3byte�ڔ��ʊ֐�
void x_vic1_putc_4byte(vic1* pVIC1, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4) {
	//Trmn1
	if(c4==0x0a) {
		//�R�}���h��M�����A����
		_vic1_inform_recv(pVIC1);
	}//��L�ȊO�Ȃ�A���Ȃ��Ƃ�1byte�ڂ̕����͎����ɂƂ��ăR�}���h�ł͂Ȃ�
	else {
		//�J�E���^��߂�
		pVIC1->RecvCnt=0;

		//��M���Ȃ�f�[�^���������Ă���
		if(pVIC1->Mode==vic1_mode_RECV) {
			_vic1_buf_putc(pVIC1, c1);
		}

		x_vic1_putc_1byte(pVIC1, c2);

		if(pVIC1->RecvCnt==0) {
			x_vic1_putc_1byte(pVIC1, c3);

			if(pVIC1->RecvCnt==0) {
				x_vic1_putc_1byte(pVIC1, c4);
			} else if(pVIC1->RecvCnt==1) {
				x_vic1_putc_2byte(pVIC1, c3, c4);
			}

		} else if(pVIC1->RecvCnt==1) {
			x_vic1_putc_2byte(pVIC1, c2, c3);

			if(pVIC1->RecvCnt==0) {
				x_vic1_putc_1byte(pVIC1, c4);
			} else if(pVIC1->RecvCnt==1) {
				x_vic1_putc_2byte(pVIC1, c3,c4);
			} else if(pVIC1->RecvCnt==2) {
				x_vic1_putc_3byte(pVIC1, c2,c3,c4);
			}
		}

	}
}
//��M�f�[�^�𓊂������
void vic1_putc(vic1* pVIC1, unsigned char c){
	if(pVIC1->RecvCnt==0) {
		x_vic1_putc_1byte(pVIC1, c);
	} else if(pVIC1->RecvCnt==1) {
		x_vic1_putc_2byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), c);
	} else if(pVIC1->RecvCnt==2) {
		x_vic1_putc_3byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), pVIC1->Recv, c);
	} else {
		x_vic1_putc_4byte(pVIC1, ((pVIC1->RecvCh)<<4)|(pVIC1->Ch), pVIC1->Recv, 0x0d, c);
	}
}
//��M�f�[�^���t���b�V������igate�݊��p�AVIC���ł̏����͂Ȃ��j
void vic1_flush(vic1* pVIC1) {}
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian  vic1_can_getc(vic1* pVIC1){
	//���M���[�h�ɑJ�ڂł���Ƃ̒ʍ�������΁A�J��
	if(_vic1_checkData(pVIC1))return hmLib_true;

	//���M���[�h�ŁA���M�f�[�^���S�đ��M�������Ă��Ȃ��ꍇ
	if(pVIC1->Send==vic1_cmd_DATA && pVIC1->SendCnt==0xFF)return _vic1_buf_can_getc(pVIC1);

	//�R�}���h���M���̏ꍇ
	if(pVIC1->Send!=vic1_cmd_NULL)return hmLib_true;

	return hmLib_false;
}
//���M�f�[�^���Ăяo��
unsigned char vic1_getc(vic1* pVIC1) {
	unsigned char c=0x00;

	//���M�ł��Ȃ��ꍇ�̓G���[
	if(!vic1_can_getc(pVIC1)) {
		pVIC1->Err=vic1_error_INVALID_GETC;
		return 0xC0;
	}

	//�f�[�^���M���[�h�ŁA���f�[�^�𑗂�I���Ă��Ȃ��ꍇ
	if(pVIC1->Send==vic1_cmd_DATA && pVIC1->SendCnt==0xFF) {
		//�f�[�^���M���̏ꍇ�́A�f�[�^�𑗐M����
		c=_vic1_buf_getc(pVIC1);

		//���M���ׂ��f�[�^�𑗂�I�����ꍇ�́ASendCnt��߂�
		if(_vic1_buf_flowing(pVIC1)==0) {
			//SendCnt�𐳂����l�ɖ߂�
			pVIC1->SendCnt=0;
		}
	}else{
		//SendCnt�����Ă�����A�G���[����
		if(pVIC1->SendCnt>3) {
			pVIC1->Err=vic1_error_INCORRECT_SENDCNT;
			pVIC1->Send=vic1_cmd_ERR;
			pVIC1->SendCnt=0;
		}

		//�R�}���h���M���̏ꍇ�́A�R�}���h��ǂ�
		if(pVIC1->SendCnt==0) {
			c=((pVIC1->Ch<<4) | pVIC1->TergetCh);
			//���M�J�E���^��i�߂�
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==1) {
			switch(pVIC1->Send) {
			case vic1_cmd_ACK:
				c=vic1_cmd_ACK;
				break;
			case vic1_cmd_NACK:
				c=vic1_cmd_NACK;
				break;
			case vic1_cmd_START:
				c=vic1_cmd_START;
				break;
			case vic1_cmd_STOP:
				c=vic1_cmd_STOP;
				break;
			case vic1_cmd_DATA:
				c=vic1_cmd_DATA;
				break;
			default:
				pVIC1->Send=vic1_cmd_ERR;
				c=vic1_cmd_ERR;
				break;
			}
			//���M�J�E���^��i�߂�
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==2) {
			c=0x0d;
			//���M�J�E���^��i�߂�
			++(pVIC1->SendCnt);
		} else if(pVIC1->SendCnt==3) {
			_vic1_inform_send(pVIC1);
			c=0x0a;
		}

	}
	return c;
}
//���M�f�[�^�𗬂ꑱ���Ă��邩�m�F�iVIC�̏ꍇ�́A�R�}���h������0,����ȊO��1�j
hmLib_boolian vic1_flowing(vic1* pVIC1) { return pVIC1->Send!=vic1_cmd_NULL; }

//============ vic functions ================
//�ʐM�������I�ɏI��������
void vic1_force_end(vic1* pVIC1) {
	pVIC1->Mode=vic1_mode_IDLE;
	pVIC1->Send=vic1_cmd_ERR;
	pVIC1->SendCnt=0;
	pVIC1->Recv=vic1_cmd_NULL;
	pVIC1->RecvCnt=0;
	pVIC1->ActiveCmd=vic1_cmd_NULL;
	pVIC1->Fail=0;
	pVIC1->TergetCh=vic1_ch_NULL;
	pVIC1->NextTergetCh=vic1_ch_NULL;

	//�f�[�^��M���Ȃ��������Ƃɂ���
	vcom1_flush(pVIC1->pVCom);
	//�f�[�^���M���Ȃ��������Ƃɂ���
	vcom1_cancel_get(pVIC1->pVCom);
}
//�G���[���擾����
unsigned char vic1_error(vic1* pVIC1) { return pVIC1->Err; }
//�G���[���N���A����
void vic1_clear_error(vic1* pVIC1) { pVIC1->Err=vic1_error_NULL; }
#
#endif
