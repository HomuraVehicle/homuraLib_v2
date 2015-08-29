#ifndef HMR_VIC1_INC
#define HMR_VIC1_INC 102
#
/*
===hmrVIC1===
Homura Interactive Communication
	���o�C�g�����ƂɃ`�F�b�N�T���ɂ��f�[�^�������؁��đ����s���ʐM�`��
	�`�F�b�N�T���̊m�F�̂��߂ɂ��������x���s���`���̂��߁A����M�̐؂�ւ����x���ꍇ�͒ʐM���x���Ȃ�B

--- command ---
�E���M��
Start	���M���Ƃ��Ă���邱�Ƃ�ʒm
Stop	���M�����I���邱�Ƃ�ʒm
Data	�f�[�^�𑗐M
Esc		�����܂ł̑��M���e����x�L�����Z�����邱�Ƃ�ʒm
What	��M���ƂȂ�邱�Ƃ�ʒm
Err		�G���[�����������ߑ���M�֌W���I�����邱�Ƃ�ʒm
�E��M��
Ack
Nack

--- protocol ---
���M�t�H�[�}�b�g

�E�f�[�^���M��
Data[1-Max], CheckSum, CH/TCH, CMD=Data, _cr, _lf

�E���̑��̃R�}���h���M��
CH/TCH,CMD,_cr,_lf
CH/TCH:CH=4bit+TCH4bit


=== hmrVIC1 ===
v1_02/140302 hmIto
	�^�C���A�E�g�񐔂ɏ����݂��A�I�[�o�[�����ꍇ�͑����̑[�u���Ƃ�悤�ɏC��
		�Ⴆ�΁A���݂��Ȃ����舶��Start�𖳌��ɌJ��Ԃ��Ă��܂���������
v1_01/140115 hmIto
	�`�F�b�N�T����crc8���̗p
v1_00/131224 hmIto
	�����[�X
*/

#include<hmLib/type.h>
#include"hmrVCom1.h"
#ifdef __cplusplus
extern "C"{
#endif

//---------------- error ----------------
#define vic1_error_NULL					0x00
#define vic1_error_SAVE_REJECTED		0x10
#define vic1_error_INCORRECT_SENDCNT	0x20
#define vic1_error_OVERFLOW				0x30
#define vic1_error_OVERREAD				0x40
#define vic1_error_INVALID_GETC			0x50

//vic1�\���̂̐錾
typedef struct{
	//���M�厯��CH (0x00-0x08)
	unsigned char Ch;
	//���M�掯��CH (0x00-0x08)
	unsigned char TergetCh;
	//�\��ςݑ��M�掯��CH (0x00-0x08)
	unsigned char NextTergetCh;
	//���[�h
	unsigned char Mode;
	//���ݕԐM�҂����̑��M�R�}���h
	unsigned char ActiveCmd;
	//���M���R�}���h
	unsigned char Send;
	//���M�J�E���^
	unsigned char SendCnt;
	//��M���R�}���h
	unsigned char Recv;
	//��M�J�E���^
	unsigned char RecvCnt;
	//��M����Ch
	unsigned char RecvCh;

	//����M�������G���[���O
	unsigned char Err;
	//�f�[�^����M�����s���Ă��Ȃ���
	unsigned char Fail;
	//�^�C���A�E�g�񐔃J�E���^�[
	unsigned char TimeOutCnt;

	//����M���L�o�b�t�@
	unsigned char* _buf_Begin;
	unsigned char* _buf_End;
	//����M���L�o�b�t�@�̃C�e���[�^
	unsigned char* _buf_GateItr;
	//VCom���̃C�e���[�^
	unsigned char* _buf_VComItr;

	//timeout����p�֐��Q
	hmLib_vFp_v Fp_timeout_restart;
	hmLib_vFp_v Fp_timeout_enable;
	hmLib_vFp_v Fp_timeout_disable;

	//����M�p�C���^�[�t�F�[�X
	vcom1* pVCom;
}vic1;
//vic1������������
//	BufEnd - BufBegin - 1 ���ő�f�[�^�T�C�Y���ƂȂ�
//	�ő�f�[�^�T�C�Y���͒ʐM���邷�ׂẴf�o�C�X�Ԃœ���ł���K�v������
//	timeout�֐��̌Ăяo���^�C�~���O�ʍ��p�֐��Q�������n��
void vic1_initialize(vic1* pVIC1, vcom1* pVCom1, const unsigned char Ch, unsigned char* BufBegin, unsigned char* BufEnd
	,hmLib_vFp_v FpTimeoutRestart, hmLib_vFp_v FpTimeoutEnable, hmLib_vFp_v FpTimeoutDisable);
//vic1���I�[������
void vic1_finalize(vic1* pVIC1);
//vic1��timeout�����p���b�^�X�N
void vic1_timeout(vic1* pVIC1);
//========== gate interface =================
//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
hmLib_boolian  vic1_can_putc(vic1* pVIC1);
//��M�f�[�^�𓊂������
void vic1_putc(vic1* pVIC1, unsigned char c_);
//��M�f�[�^���t���b�V������igate�݊��p�AVIC���ł̏����͂Ȃ��j
void vic1_flush(vic1* pVIC1);
//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
hmLib_boolian  vic1_can_getc(vic1* pVIC1);
//���M�f�[�^���Ăяo��
unsigned char vic1_getc(vic1* pVIC1);
//���M�f�[�^�𗬂ꑱ���Ă��邩�m�F�iVIC�̏ꍇ�́A�f�[�^�V�[�P���X������0�A����ȊO��1�j
hmLib_boolian vic1_flowing(vic1* pVIC1);
//============ vic functions ================
//�ʐM�������I�ɏI��������
void vic1_force_end(vic1* pVIC1);
//�G���[���擾����
unsigned char vic1_error(vic1* pVIC1);
//�G���[���N���A����
void vic1_clear_error(vic1* pVIC1);
#ifdef __cplusplus
}	//extern "C"{
#endif
#
#endif
