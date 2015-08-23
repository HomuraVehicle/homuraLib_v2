#ifndef HMR_VMC1_INC
#define HMR_VMC1_INC 400
#
/*===hmrVMC1===
	�قނ�̑��c�҂�MainPic���Ȃ���ʐM�̒ʐM�t�H�[�}�b�g���K�肷��t�@�C��
	�o�b�t�@����̑����֐���p�ӂ��Ă��ƁA
		��M�f�[�^��VMC1�֐��Q�Ɏ󂯓n���ƁA�o�b�t�@�ւ̒ǉ��������s���Ă����
		���M�f�[�^��VMC1�֐��Q�ŗv������ƁA�o�b�t�@��ǂݏo���ēK���ȑ��M������Ԃ�

hmrVMC1_v4_00/130223 hmIto
	c�t�@�C������extern C���s�v�������̂��폜
	vmc1_create�̈���������VMCID���Avmc1_initialize�Ɉړ�
	boolian��hmLib_boolian�ɕύX
	hmrType��include���폜
	can_iniSendPac()/can_iniRecvPac()��ǉ�
		���ꂼ��A���M/��M���̃p�P�b�g���J����
hmrVMC1_v3_00/130211 hmIto
	can_existSendDat�֐���vmc_interface�ɒǉ�
		existSendDat�֐������s�\����₢���킹��֐�
	getSendErr/getRecvErr/clearSendErr/clearRecvErr�֐���ǉ�
		���ꂼ��C�Ō�ɔ�����������M���̃G���[���擾/�N���A����
		�G���[�������ł��C���ɓ���Ɏx��͂Ȃ��i�����I�Ƀp�P�b�g�����Ȃǂ��āC����M�͏�������Ă���j
hmrVMC1_v2_02/121128 hmIto
	hmCLib��hmLib�̓����ɔ����AhmLib_v3_04�ɑΉ�����悤�ύX
	hmrVMC1���g��C++�ɑΉ�
hmrVMC1_v2_01/121027 hmIto
	vmc1_send�ɂ����āC0byte�f�[�^���G���[�����ɂ��Ă����̂��C��
hmrVMC1_v2_00/121020 hmIto
	������VMC�̓����^�p�ɑΉ�
		3byte�̃f�o�C�X�ŗL��VMCID��V���Ɏw��
			���M��ID�Ƒ��M��ID��VMC�\���̂ɒǉ�
		��M���ɂ͎��g��VMCID�����̒ʐM�̂ݎ�M
			���g��VMCID��create�֐��Ŏw��
		���M���ɂ͑��M���VMCID���w�肷�邱�Ƃ��K�v
			�����VMCID��initialize�֐��Ŏw��
		�Ȍ�̋K�i�́A#{VMCID[3]}.....##crlf
			static��STRT,TRMN�͔p�~
	�֐�����force_ini_send/recv����force_end_send/recv�ɕύX
	finalize�ǉ�
	create���ł�initialize�������s��p�~
hmrVMC1_v1_04/121014 hmIto
	force_ini_send/recv�֐���ǉ�
		timeout�ȂǁAVMC1�̎�M�̔@���Ɋւ�炸�����ɃX�e�[�^�X������������̂Ɏg�p
hmrVMC1_v1_03/121013 hmIto
	���O���VMC1��vmc1�ɕύX
hmrVMC1_v1_02/121008 hmIto
	�eDat��ID�ɂ��Ă�VMC1�ŊǗ�����悤�ύX�@���M�E��M���̊֐��`�ȂǕύX
	�f�[�^�T�C�Y�̃o�b�t�@�T�C�Y��2byte�֊g��
	DatID[1],DatSize[2],Dat[DatSize]�Ƀf�[�^�̃t�H�[�}�b�g��ύX
	#PacID[3],Dat...,##crlf�Ƀp�P�b�g�̃t�H�[�}�b�g��ύX
	121009Test_hmrCom_hmrVMC1.c�ɂāA����m�F�ς�
hmrVMC1_v1_01/120922 hmIto
	�ЂƂ܂����삪���肵�Ă��邱�Ƃ��m�F�i120922Test_hmrCom_hmrVMC1.c�ɂē���m�F�ς݁j
hmrVMC1_v1_00/120921 hmIto
	�o�[�W�����Ǘ��J�n
*/
#ifndef HMLIB_TYPE_INC
#	include"hmLib_v3_05/bytebase_type.h"
#endif
#ifdef __cplusplus
extern "C"{
#endif
#define vmc1_RECVERR_fail_to_iniRecvDat 1
#define vmc1_RECVERR_fail_to_recv 2
#define vmc1_SENDERR_fail_to_existSendDat 1
#define vmc1_SENDERR_fail_to_iniSendDat 2
#define vmc1_SENDERR_fail_to_send 3

typedef hmLib_uint16 vmc1_dsize_t;
typedef hmLib_uint8 vmc1_did_t;
//typedef hmLib_boolian hmLib_boolian;
typedef void(*vmc1_vFp_v)(void);
typedef void(*vmc1_vFp_uc)(unsigned char);
typedef void(*vmc1_vFp_did_dsize)(vmc1_did_t,vmc1_dsize_t);
typedef unsigned char(*vmc1_ucFp_v)(void);
typedef hmLib_boolian(*vmc1_bFp_v)(void);
typedef void(*vmc1_vFp_pdid_pdsize)(vmc1_did_t*,vmc1_dsize_t*);
//VMC1�\���̂̐錾
typedef struct{
	//���M�厯��ID
	unsigned char VMCID[3];
	//���M�掯��ID
	unsigned char TVMCID[3];
	//��M���[�h
	unsigned char RecvMode;
	//��M���[�h�̃J�E���^
	unsigned char RecvModeCnt;
	//��M�f�[�^�̃J�E���^
	vmc1_dsize_t RecvDatCnt;
	//��M���[�h�̃G���[
	unsigned char RecvErr;
	//��M�f�[�^�̃T�C�Y
	vmc1_dsize_t RecvDatSize;
	//��M�f�[�^��id
	vmc1_did_t RecvDatID;
	//���M���[�h
	unsigned int SendMode;
	//���M���[�h�̃J�E���^
	vmc1_dsize_t SendCnt;
	//���M���[�h�̃G���[
	unsigned char SendErr;
	//���M�f�[�^�̃T�C�Y
	vmc1_dsize_t SendDatSize;
	//��M�f�[�^��id
	vmc1_did_t SendDatID;
/************************����֐�**************************/
	//iniRecvPac�����s���ėǂ����̊m�F�ɌĂ΂��
	vmc1_bFp_v fp_can_iniRecvPac;
	//PacStrt��M�������ɌĂ΂��
	vmc1_vFp_v fp_iniRecvPac;
	//PacTrmn��M�������ɌĂ΂��@�����̓G���[�̗L��
	vmc1_vFp_uc fp_finRecvPac;
	//iniRecvDat�����s���ėǂ����̊m�F�ɌĂ΂��
	vmc1_bFp_v fp_can_iniRecvDat;
	//Dat��M�J�n���ɌĂ΂�� �����͎�M����f�[�^�T�C�Y ���s�́C�K��can_iniRecvDat���m�F��
	vmc1_vFp_did_dsize fp_iniRecvDat;
	//Dat��M�I�����ɌĂ΂�� �����̓G���[�̗L��
	vmc1_vFp_uc fp_finRecvDat;
	//recv�����s���Ă悢���̊m�F�ɌĂ΂��
	vmc1_bFp_v fp_can_recv;
	//Dat�̒��g��M���ɌĂ΂��@��������M�����f�[�^ ���s�́C�K��fp_can_recv���m�F��
	vmc1_vFp_uc fp_recv;
	//iniSendPac�����s���ėǂ����̊m�F�ɌĂ΂��
	vmc1_bFp_v fp_can_iniSendPac;
	//RacStrt���M�������ɌĂ΂��
	vmc1_vFp_v fp_iniSendPac;
	//PacTrmn���M�������ɌĂ΂��@�����̓G���[�̗L��
	vmc1_vFp_uc fp_finSendPac;
	//existSendDat�֐��̎��s�̉ۂ��m�F
	vmc1_bFp_v fp_can_existSendDat;
	//���M���K�v�ȃf�[�^�̗L���̊m�F ���s�́C�K��can_existSendDat���m�F��
	vmc1_bFp_v fp_existSendDat;
	//iniSendDat�����s���ėǂ����̊m�F�ɌĂ΂�� ���s�́C�K��existSendDat���m�F��
	vmc1_bFp_v fp_can_iniSendDat;
	//Dat���M�m�莞�ɌĂ΂��@�T�C�Y��id��߂� ���s�́C�K��can_iniSendDat���m�F��
	vmc1_vFp_pdid_pdsize fp_iniSendDat;
	//Dat���M�I�����ɌĂ΂��@�����̓G���[�̗L��
	vmc1_vFp_uc fp_finSendDat;
	//send�����s���Ă悢���̊m�F�ɌĂ΂��
	vmc1_bFp_v fp_can_send;
	//Dat�̒��g���M���ɌĂ΂�� ���s�́C�K��fp_can_send���m�F��
	vmc1_ucFp_v fp_send;
}VMC1;
//VMC1���쐬���� VMCID�̎w�肪�K�v
VMC1* vmc1_create
	(vmc1_bFp_v fp_can_iniRecvPac_		//iniRecvPac�����s�\��
	,vmc1_vFp_v fp_iniRecvPac_			//PacStrt��M�������ɌĂ΂��
	,vmc1_vFp_uc fp_finRecvPac_			//PacTrmn��M�������ɌĂ΂��@�����̓G���[�̗L��
	,vmc1_bFp_v fp_can_iniRecvDat_		//iniRecvDat�����s���ėǂ����̊m�F�ɌĂ΂��
	,vmc1_vFp_did_dsize fp_iniRecvDat_	//Dat��M�m�莞�ɃT�C�Y��id��n�����
	,vmc1_vFp_uc fp_finRecvDat_			//Dat��M�I�����ɌĂ΂�� �����̓G���[�̗L��
	,vmc1_bFp_v fp_can_recv_			//recv�����s���Ă悢���̊m�F�ɌĂ΂��
	,vmc1_vFp_uc fp_recv_				//Dat�̒��g��M���ɌĂ΂��@��������M�����f�[�^
	,vmc1_bFp_v fp_can_iniSendPac		//iniSendPac�����s�\��
	,vmc1_vFp_v fp_iniSendPac_			//RacStrt���M�������ɌĂ΂��
	,vmc1_vFp_uc fp_finSendPac_			//PacTrmn���M�������ɌĂ΂��@�����̓G���[�̗L��
	,vmc1_bFp_v fp_can_existSendDat_	//existSendDat�֐������s�\����₢���킹��
	,vmc1_bFp_v fp_existSendDat_		//���M���K�v�ȃf�[�^�̗L���̊m�F
	,vmc1_bFp_v fp_can_iniSendDat_		//iniSendDat�����s���ėǂ����̊m�F�ɌĂ΂��
	,vmc1_vFp_pdid_pdsize fp_iniSendDat_//Dat���M�m�莞�ɌĂ΂��@�T�C�Y��߂�
	,vmc1_vFp_uc fp_finSendDat_			//Dat���M�I�����ɌĂ΂��@�����̓G���[�̗L��
	,vmc1_bFp_v fp_can_send_			//send�����s���Ă悢���̊m�F�ɌĂ΂��
	,vmc1_ucFp_v fp_send_);				//Dat�̒��g���M���ɌĂ΂��
//VMC1���I������
void vmc1_release(VMC1* pVMC1);
//VMC1������������ �����͑��M���VMCID
void vmc1_initialize(VMC1* pVMC1,const unsigned char VMCID_[3],const unsigned char TVMCID_[3]);
//VMC1���I�[������
void vmc1_finalize(VMC1* pVMC1);
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian  vmc1_can_recv(VMC1* pVMC1);
//��M�f�[�^�𓊂������
void vmc1_recv(VMC1* pVMC1,unsigned char c_);
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian  vmc1_can_send(VMC1* pVMC1);
//���M�f�[�^���Ăяo��
unsigned char vmc1_send(VMC1* pVMC1);
//��M�������I�ɏI��������
void vmc1_force_end_recv(VMC1* pVMC1,unsigned char Err);
//���M�������I�ɏI��������
void vmc1_force_end_send(VMC1* pVMC1,unsigned char Err);
//���M�G���[���擾����
unsigned char vmc1_getSendErr(VMC1* pVMC1);
//���M�G���[���N���A����
void vmc1_clearSendErr(VMC1* pVMC1);
//��M�G���[���擾����
unsigned char vmc1_getRecvErr(VMC1* pVMC1);
//��M�G���[���N���A����
void vmc1_clearRecvErr(VMC1* pVMC1);
#ifdef __cplusplus
}	//extern "C"{
#endif
#
#endif
