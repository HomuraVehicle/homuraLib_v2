#ifndef HMR_VMC1_C_INC
#define HMR_VMC1_C_INC 400
#
#define HMLIB_NOLIB
/*
vmc1_recv�̎d�l�ɗv����
	Trmn�u���v����M�����ꍇ�ɂ́A�o�b�t�@�ɂ͕񍐂���Trmn�J�E���^�[����
	�Ō�܂�Trmn����M�����ꍇ�A���̂܂܏I��=>Trmn�̓o�b�t�@�ɂ͍Ō�܂ŕ񍐂���Ȃ�
	����A�r����Trmn�ł͂Ȃ��������������ꍇ�ɂ́A�{���񍐂��ׂ�����������=Trmn[0�`Trmn�J�E���^�[]���o�b�t�@�ɕ񍐂���
*/
#ifndef HMR_VMC1_INC
#	include"hmrVMC1.h"
#endif

#include<stdlib.h>

#define vmc1_RECVMODE_WAIT	0	//PacketStrt�҂�
#define vmc1_RECVMODE_DTID	1	//DatID����M��
#define vmc1_RECVMODE_SIZE	2	//DatSize����M��
#define vmc1_RECVMODE_DATA	3	//Dat����M��
#define vmc1_SENDMODE_STRT	1
#define vmc1_SENDMODE_CHEK	2
#define vmc1_SENDMODE_DTID	3
#define vmc1_SENDMODE_SIZE	4
#define vmc1_SENDMODE_DATA	5
#define vmc1_SENDMODE_TRMN	6

//multi-thread�Ŏg���Ȃ����߁A�p�~
//�Ȍ�AVMCID�ƒ��ł��őΉ�
//**************** config ***********************//
//static const unsigned char vmc1_RecvPacStrt[4]={'#','h','m','r'};
//static const unsigned char vmc1_RecvPacStrtSize=4;
//static const unsigned char vmc1_RecvPacTrmn[4]={'#','#',0x0d,0x0a};
//static const unsigned char vmc1_RecvPacTrmnSize=4;
//static const unsigned char vmc1_SendPacStrt[4]={'#','c','t','r'};
//static const unsigned char vmc1_SendPacStrtSize=4;
//static const unsigned char vmc1_SendPacTrmn[4]={'#','#',0x0d,0x0a};
//static const unsigned char vmc1_SendPacTrmnSize=4;
//***********************************************//
//vmc1PacStrt={'#',VMCID[3]}
static const unsigned char vmc1_PacTrmn[4]={'#','#',0x0d,0x0a};
static const unsigned char vmc1_PacTrmnSize=4;

//VMC1������������
VMC1* vmc1_create
	(vmc1_bFp_v fp_can_iniRecvPac_
	,vmc1_vFp_v fp_iniRecvPac_
	,vmc1_vFp_uc fp_finRecvPac_
	,vmc1_bFp_v fp_can_iniRecvDat_
	,vmc1_vFp_did_dsize fp_iniRecvDat_
	,vmc1_vFp_uc fp_finRecvDat_
	,vmc1_bFp_v fp_can_recv_
	,vmc1_vFp_uc fp_recv_
	,vmc1_bFp_v fp_can_iniSendPac_
	,vmc1_vFp_v fp_iniSendPac_
	,vmc1_vFp_uc fp_finSendPac_
	,vmc1_bFp_v fp_can_existSendDat_
	,vmc1_bFp_v fp_existSendDat_
	,vmc1_bFp_v fp_can_iniSendDat_
	,vmc1_vFp_pdid_pdsize fp_iniSendDat_
	,vmc1_vFp_uc fp_finSendDat_
	,vmc1_bFp_v fp_can_send_
	,vmc1_ucFp_v fp_send_){
	VMC1* p;
	p=(VMC1*)malloc(sizeof(VMC1));
	p->fp_can_iniRecvPac=fp_can_iniRecvPac_;
	p->fp_iniRecvPac=fp_iniRecvPac_;
	p->fp_finRecvPac=fp_finRecvPac_;
	p->fp_can_iniRecvDat=fp_can_iniRecvDat_;
	p->fp_iniRecvDat=fp_iniRecvDat_;
	p->fp_finRecvDat=fp_finRecvDat_;
	p->fp_can_recv=fp_can_recv_;
	p->fp_recv=fp_recv_;
	p->fp_can_iniSendPac=fp_can_iniSendPac_;
	p->fp_iniSendPac=fp_iniSendPac_;
	p->fp_finSendPac=fp_finSendPac_;
	p->fp_can_existSendDat=fp_can_existSendDat_;
	p->fp_existSendDat=fp_existSendDat_;
	p->fp_can_iniSendDat=fp_can_iniSendDat_;
	p->fp_iniSendDat=fp_iniSendDat_;
	p->fp_finSendDat=fp_finSendDat_;
	p->fp_can_send=fp_can_send_;
	p->fp_send=fp_send_;
	return p;
}
//VMC1���I������
void vmc1_release(VMC1* pVMC1){
	free(pVMC1);
}
//VMC1������������
void vmc1_initialize(VMC1* pVMC1,const unsigned char VMCID_[3],const unsigned char TVMCID_[3]){
	pVMC1->VMCID[0]=VMCID_[0];
	pVMC1->VMCID[1]=VMCID_[1];
	pVMC1->VMCID[2]=VMCID_[2];
	pVMC1->TVMCID[0]=TVMCID_[0];
	pVMC1->TVMCID[1]=TVMCID_[1];
	pVMC1->TVMCID[2]=TVMCID_[2];
	pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
	pVMC1->RecvModeCnt=0;
	pVMC1->RecvDatCnt=0;
	pVMC1->RecvErr=0;
	pVMC1->RecvDatSize=0;
	pVMC1->SendMode=vmc1_SENDMODE_STRT;
	pVMC1->SendCnt=0;
	pVMC1->SendErr=0;
	pVMC1->SendDatSize=0;
}
//VMC1���I�[������
void vmc1_finalize(VMC1* pVMC1){return;}
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian vmc1_can_recv(VMC1* pVMC1){
	switch(pVMC1->RecvMode){
	case vmc1_RECVMODE_WAIT:
		return pVMC1->fp_can_iniRecvPac();
	case vmc1_RECVMODE_DTID:
		return 1;
	case vmc1_RECVMODE_SIZE:
		if(pVMC1->RecvDatCnt==1)return pVMC1->fp_can_iniRecvDat();
		else return 1;
	case vmc1_RECVMODE_DATA:
		return pVMC1->fp_can_recv();
	default:
		return 0;
	}
}
//��M�f�[�^�𓊂������i��M���[�h�̏ꍇ�̎��ۂ̃f�[�^�̎�M����
void _vmc1_recv_data(VMC1* pVMC1,unsigned char c){
	//�f�[�^id��M��
	if(pVMC1->RecvMode==vmc1_RECVMODE_DTID){
		//�f�[�^ID���擾
		pVMC1->RecvDatID=c;
		++(pVMC1->RecvDatCnt);

		//�f�[�^id��M������
		if(pVMC1->RecvDatCnt>=1){
			//���[�h���T�C�Y��M���[�h(SIZE)�ֈڍs
			pVMC1->RecvMode=vmc1_RECVMODE_SIZE;
			pVMC1->RecvDatCnt=0;
			pVMC1->RecvDatSize=0;
		}
	}//�f�[�^�T�C�Y��M��
	else if(pVMC1->RecvMode==vmc1_RECVMODE_SIZE){
		//�f�[�^�T�C�Y���擾
		pVMC1->RecvDatSize|=((vmc1_dsize_t)(c)<<(8*pVMC1->RecvDatCnt));
		++(pVMC1->RecvDatCnt);

		//�f�[�^�T�C�Y��M������
		if(pVMC1->RecvDatCnt>=2){
			//�f�[�^���o�b�t�@����M�ł��Ȃ���
			if(!pVMC1->fp_can_iniRecvDat()){
				//PAC�������I�ɕ���
				pVMC1->fp_finRecvPac(1);
				//���[�h��Pac�ҋ@���[�h�ֈڍs
				pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
				pVMC1->RecvModeCnt=0;
				pVMC1->RecvErr=vmc1_RECVERR_fail_to_iniRecvDat;
			}//��M���ׂ��f�[�^�����݂���ꍇ
			else{
				//���[�h��Pac��M���[�h�ֈڍs
				pVMC1->RecvMode=vmc1_RECVMODE_DATA;
				pVMC1->RecvDatCnt=0;
				//�f�[�^�����݂��Ȃ���(0byte�f�[�^)
				if(pVMC1->RecvDatSize==0){
					//���[�h��ID��M���[�h�ֈڍs
					pVMC1->RecvMode=vmc1_RECVMODE_DTID;
					pVMC1->RecvDatID=0;
					pVMC1->RecvDatSize=0;
					pVMC1->RecvDatCnt=0;
				}else{
					pVMC1->fp_iniRecvDat(pVMC1->RecvDatID,pVMC1->RecvDatSize);
					pVMC1->RecvDatCnt=0;
				}
			}
		}
	}//�f�[�^��M��
	else if(pVMC1->RecvMode==vmc1_RECVMODE_DATA){
		//�f�[�^���o�b�t�@����M�ł��Ȃ���
		if(pVMC1->fp_can_recv()==0){
			//DAT,PAC�������I�ɕ���
			pVMC1->fp_finRecvDat(1);
			pVMC1->fp_finRecvPac(1);
			//���[�h��Pac�ҋ@���[�h�ֈڍs
			pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
			pVMC1->RecvModeCnt=0;
			pVMC1->RecvErr=vmc1_RECVERR_fail_to_recv;
		}else{
			//�o�b�t�@�֑}��
			pVMC1->fp_recv(c);

			//�J�E���^�[���Z�b�g
			++(pVMC1->RecvDatCnt);

			//�SDat��M����(id���܂܂��̂ŁA���ł�1�������Z���Ă���)
			if(pVMC1->RecvDatCnt >= pVMC1->RecvDatSize){
				pVMC1->fp_finRecvDat(0);
				//���[�h��Dat��M���[�h�ֈڍs
				pVMC1->RecvMode=vmc1_RECVMODE_DTID;
				pVMC1->RecvDatCnt=0;
				pVMC1->RecvDatID=0;
				pVMC1->RecvDatSize=0;
			}
		}
	}
}
//��M�f�[�^�𓊂������
void vmc1_recv(VMC1* pVMC1,unsigned char c){
	vmc1_dsize_t cnt;
	//���M���[�h(vmc1_RecvPacStrt�҂�)
	if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT){
		//Pac�����������J���Ȃ��ꍇ�́A�ǂݎ̂�
		if(!pVMC1->fp_can_iniRecvPac())return;
		//�擪���ʋL���̎�M��
		if(pVMC1->RecvModeCnt==0){
			if(c=='#'){
				++(pVMC1->RecvModeCnt);
			}
		}//�擪���ʋL���̎�M�� ID�m�F��
		else{
			//ID����v
			if(c==pVMC1->VMCID[pVMC1->RecvModeCnt-1]){
				++(pVMC1->RecvModeCnt);
				if(pVMC1->RecvModeCnt==4){
					//���[�h��Pac��M���[�h�ֈڍs
					pVMC1->RecvMode=vmc1_RECVMODE_DTID;
					pVMC1->RecvModeCnt=0;
					pVMC1->RecvDatID=0;
					pVMC1->RecvDatSize=0;
					pVMC1->RecvDatCnt=0;
					pVMC1->fp_iniRecvPac();
				}
			}//ID�͕s��v�����擪���ʋL������M
			else if(c=='#'){
				pVMC1->RecvModeCnt=1;
			}//����ȊO
			else{
				pVMC1->RecvModeCnt=0;
			}
		}
	}//��M���[�h(PacTrmn�҂�)
	else{
		//��M������RecvModeCnt�Ԗڂ�PacTrmn�����ƈ�v
		if(vmc1_PacTrmn[pVMC1->RecvModeCnt]==c){
			//�J�E���^�[���Z
			++(pVMC1->RecvModeCnt);
			//�Svmc1_RecvPacTrmn��M
			if(pVMC1->RecvModeCnt==vmc1_PacTrmnSize){
				//���[�h�C��
				if(pVMC1->RecvMode!=vmc1_RECVMODE_DTID && pVMC1->RecvMode!=vmc1_RECVMODE_WAIT){
					pVMC1->fp_finRecvDat(1);
					pVMC1->fp_finRecvPac(1);
				}else{
					pVMC1->fp_finRecvPac(0);
				}

				//���[�h��Pac�ҋ@���[�h�ֈڍs
				pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
				pVMC1->RecvModeCnt=0;
			}
		}//��M������0�Ԗڂ�PacTrmn�����ƈ�v
		else if(vmc1_PacTrmn[0]==c){
			//Trmn�Ɉ�v��������܂ł̕�����Trmn�łȂ��������ƂɂȂ�̂ŁA���̕������߂čĎ�M
			for(cnt=0; cnt< pVMC1->RecvModeCnt;++cnt){
				_vmc1_recv_data(pVMC1,vmc1_PacTrmn[cnt]);
				if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)break;
			}
			//�J�E���^�[��1��(1�����͈�v��������)
			pVMC1->RecvModeCnt=1;
		}//��M������PacTrmn�����ƕs��v���������A����܂ł�Trmn�Ǝv���ăp�X��������������ꍇ
		else if(pVMC1->RecvModeCnt>0){
			//����܂ł̕�����Trmn�łȂ��������ƂɂȂ�̂ŁA���̕������߂čĎ�M
			for(cnt=0; cnt< pVMC1->RecvModeCnt;++cnt){
				_vmc1_recv_data(pVMC1,vmc1_PacTrmn[cnt]);
				if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)break;
			}
			if(pVMC1->RecvMode!=vmc1_RECVMODE_WAIT)_vmc1_recv_data(pVMC1,c);
			//�J�E���^�[���Z�b�g
			pVMC1->RecvModeCnt=0;
		}//Trmn�J�E���^�[�ɂ͈����������Ă��Ȃ�
		else{
			//�ʏ�̎�M����
			_vmc1_recv_data(pVMC1,c);
		}
	}
}
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian vmc1_can_send(VMC1* pVMC1){
	switch(pVMC1->SendMode){
	case vmc1_SENDMODE_STRT:
		return pVMC1->fp_can_iniSendPac();
	case vmc1_SENDMODE_CHEK:
		if(!pVMC1->fp_can_existSendDat())return 0;	//�m�F�ł��Ȃ��Ƃ��͖����Ȃ̂�0
		if(!pVMC1->fp_existSendDat())return 1;		//���M���ׂ�Dat�����݂��Ȃ�=TRMN�����Ȃ̂�1
		return pVMC1->fp_can_iniSendDat();
	case vmc1_SENDMODE_DTID:
		return 1;
	case vmc1_SENDMODE_SIZE:
		return 1;
	case vmc1_SENDMODE_DATA:
		return pVMC1->fp_can_send();
	case vmc1_SENDMODE_TRMN:
		return 1;
	default:
		return 0;
	}
}
//���M�f�[�^���Ăяo��
unsigned char vmc1_send(VMC1* pVMC1){
	unsigned char c;
	//�񑗐M���[�h(vmc1_SendPacStrt�҂�)
	if(pVMC1->SendMode==vmc1_SENDMODE_STRT){
		//���������p�P�b�g���J���Ȃ��Ƃ��́A0��Ԃ��B
		if(!pVMC1->fp_can_iniSendPac())return 0;
		//�ŏ��̕����̏ꍇ
		if(pVMC1->SendCnt==0){
			c='#';
			++(pVMC1->SendCnt);
		//ID���t�t�F�[�Y
		}else{
			c=pVMC1->TVMCID[pVMC1->SendCnt-1];
			++(pVMC1->SendCnt);
			//PacStrt�𑗂�I�������A���M�p�ɏ�����
			if(pVMC1->SendCnt>=4){
				pVMC1->fp_iniSendPac();
				pVMC1->SendMode=vmc1_SENDMODE_CHEK;
				pVMC1->SendCnt=0;
			}
		}
	}
	else{
		//�m�F���[�h�ɂȂ��Ă���ꍇ
		if(pVMC1->SendMode==vmc1_SENDMODE_CHEK){
			//���̃f�[�^���݂��m�F�ł��Ȃ��ꍇ�́C�G���[�I��
			if(!pVMC1->fp_can_existSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_existSendDat;
			}//���f�[�^�����݂��Ȃ��ꍇ�́C���ʂ�TRMN�ɐݒ�
			else if(!pVMC1->fp_existSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
			}//���f�[�^���擾�ł��Ȃ��ꍇ�́C�G���[�I��
			else if(!pVMC1->fp_can_iniSendDat()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_iniSendDat;
			}//����ȊO�̏ꍇ�́C�f�[�^ID���M���[�h�ֈڍs
			else{
				pVMC1->fp_iniSendDat(&(pVMC1->SendDatID),&(pVMC1->SendDatSize));
				pVMC1->SendMode=vmc1_SENDMODE_DTID;
				pVMC1->SendCnt=0;
			}
		}//DATA���[�h�̏ꍇ���ēx�m�F
		else if(pVMC1->SendMode==vmc1_SENDMODE_DATA){
			//send�Ɏ��s����ꍇ�͋����I��
			if(!pVMC1->fp_can_send()){
				pVMC1->SendMode=vmc1_SENDMODE_TRMN;
				pVMC1->SendCnt=0;
				pVMC1->SendErr=vmc1_SENDERR_fail_to_send;
			}
		}

		//ID�𑗂�
		if(pVMC1->SendMode==vmc1_SENDMODE_DTID){
			//ID���܂߂����𑗐M����̂ŁASize+1
			c=pVMC1->SendDatID;
			++(pVMC1->SendCnt);
			//���M������
			if(pVMC1->SendCnt==1){
				pVMC1->SendMode=vmc1_SENDMODE_SIZE;
				pVMC1->SendCnt=0;
			}
		}//�T�C�Y�𑗂�
		else if(pVMC1->SendMode==vmc1_SENDMODE_SIZE){
			c=(unsigned char)((pVMC1->SendDatSize>>(pVMC1->SendCnt*8))&0xFF);
			++(pVMC1->SendCnt);
			//���M������
			if(pVMC1->SendCnt==2){
				if(pVMC1->SendDatSize>0){
					pVMC1->SendMode=vmc1_SENDMODE_DATA;
					pVMC1->SendCnt=0;
				}else{
					//���M�̐���I����ʒm
					pVMC1->fp_finSendDat(0);
					pVMC1->SendMode=vmc1_SENDMODE_CHEK;
					pVMC1->SendCnt=0;
				}
			}
		}//�f�[�^�𑗂�
		else if(pVMC1->SendMode==vmc1_SENDMODE_DATA){	
			c=pVMC1->fp_send();
			++(pVMC1->SendCnt);
			//�S�f�[�^���M������
			if(pVMC1->SendCnt==pVMC1->SendDatSize){
				//���M�̐���I����ʒm
				pVMC1->fp_finSendDat(0);
				//���f�[�^�̗L���͍��m�F�����A�m�F���[�h�ֈڍs
				pVMC1->SendMode=vmc1_SENDMODE_CHEK;
				pVMC1->SendCnt=0;
			}
		}//TRMN�𑗂�
		else if(pVMC1->SendMode==vmc1_SENDMODE_TRMN){
			//vmc1_SendPacStrt1�������M
			c=vmc1_PacTrmn[pVMC1->SendCnt];
			++(pVMC1->SendCnt);

			//PacStrt�𑗂�I�������A���M�p�ɏ�����
			if(pVMC1->SendCnt>=vmc1_PacTrmnSize){
				pVMC1->SendMode=vmc1_SENDMODE_STRT;
				pVMC1->SendCnt=0;
				pVMC1->fp_finSendPac(pVMC1->SendErr);
				pVMC1->SendErr=0;
			}
		}
	}
	return c;
}
//��M�������I�ɏI��������
void vmc1_force_end_recv(VMC1* pVMC1,unsigned char Err){
	if(pVMC1->RecvMode==vmc1_RECVMODE_WAIT)return;
	if(pVMC1->RecvMode==vmc1_RECVMODE_DATA){
		pVMC1->fp_finRecvDat(Err);
	}
	pVMC1->fp_finRecvPac(Err);
	
	pVMC1->RecvDatCnt=0;
	pVMC1->RecvDatID=0;
	pVMC1->RecvDatSize=0;
	pVMC1->RecvMode=vmc1_RECVMODE_WAIT;
	pVMC1->RecvModeCnt=0;
}
//���M�������I�ɏI��������
void vmc1_force_end_send(VMC1* pVMC1,unsigned char Err){
	if(pVMC1->SendMode==vmc1_SENDMODE_STRT)return;
	if(pVMC1->SendMode==vmc1_SENDMODE_DATA){
		pVMC1->fp_finSendDat(Err);
	}
	pVMC1->fp_finSendPac(Err);

	pVMC1->SendCnt=0;
	pVMC1->SendDatID=0;
	pVMC1->SendDatSize=0;
	pVMC1->SendErr=0;
	pVMC1->SendMode=vmc1_SENDMODE_STRT;
}
//���M�G���[���擾����
unsigned char vmc1_getSendErr(VMC1* pVMC1){return pVMC1->SendErr;}
//���M�G���[���N���A����
void vmc1_clearSendErr(VMC1* pVMC1){pVMC1->SendErr=0;}
//��M�G���[���擾����
unsigned char vmc1_getRecvErr(VMC1* pVMC1){return pVMC1->RecvErr;}
//��M�G���[���N���A����
void vmc1_clearRecvErr(VMC1* pVMC1){pVMC1->RecvErr=0;}
#
#endif
