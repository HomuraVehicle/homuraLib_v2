#ifndef HMR_VCOMDATA_INC
#define HMR_VCOMDATA_INC 101
#
#include<hmLib_v3_06/type.h>
#include<hmLib_v3_06/cstring.h>
/*---vcom---
Volcanic Communication Data Interface.
�E�قނ�p�̃f�[�^����M�p���ʃv���g�R��
	�e���W���[���́ACH0-7�̌ŗL�`�����l��������
	�e����M�f�[�^�́A���M��CH[1], ����ID[1], DataSize[2],Data[DataSize]�������Ă���
	����ID��0x00-0xEF�̒l���Ƃ�B0xF0-0xFF��vcom����ɗ\�񂳂�Ă���A�Ǝ��g�p�͕s�B
	DataSize�́A0�`4096byte�͈͓̔�
�Evcom�́A���ʃv���g�R�������̒ʐM�K�i����A�f�[�^�̑���M���s���B
�E���̋��ʃv���g�R���ɏ����Ă���ʐM�K�i�́Avcom���g���ĒʐM���s��
	vcom���ێ�����֐��|�C���^��p���āA�ʐM���s����B

=== vcom ===
v1_01/140110 hmIto
	ID�ɂ������K�������
v1_00/131220 hmIto
	VMC1�����ƂɁA��ʂ�̋@�\������
*/
#ifdef __cplusplus
extern "C"{
#endif
	typedef hmLib_uint8 vcom_ch_t;
	typedef hmLib_uint8 vcom_id_t;
	typedef hmLib_uint8 vcom_err_t;
	typedef hmLib_cstring_size_t vcom_size_t;

	typedef struct {
		vcom_ch_t Ch;				//��M���̑��M��Ch/���M���̑��M��Ch
		vcom_id_t ID;				//Data����ID
		vcom_err_t Err;			//DataErr�R�[�h
		hmLib_u16Fp_v Accessible;	//Data�A�N�Z�X�\�̈�
		hmLib_cstring Data;			//Data���� Size���܂܂�Ă���
	}vcom_data;

//vcom_data�^�����e����ő�f�[�^�T�C�Y���`
#define vcom_data_MAXSIZE			4096

//vcom_data�\��ID�ꗗ
#define vcom_data_id_is_valid(id)	((unsigned char)(id)<0xF0)
#define vcom_data_id_NULL	0xFF	//Null ID
#define vcom_data_id_EOF	0xFE	//End of File���ʗpID

//vcom_data�G���[�R�[�h�ꗗ�i�d���j
#define vcom_data_error_NULL		0x00		//Data�ɃG���[���Ȃ�
#define vcom_data_error_STRANGER	0x01		//Data�̑��M��CH����������
#define vcom_data_error_UNKNOWN		0x02		//Data�̎���ID����������
#define vcom_data_error_FAILNEW		0x04		//Data�̊m�ۂɎ��s����
#define vcom_data_error_SIZEOVER	0x08		//Data��Size���傫������
#define vcom_data_error_OVERFLOW	0x10		//Data��Size�ɔ�ׂđ�������
#define vcom_data_error_UNDERFLOW	0x20		//Data��Size�ɔ�ׂď��Ȃ�����
#define vcom_data_error_BROKEN		0x40		//Data�����Ă���

	//�t�H�[�}�b�g
	void vcom_data_format(vcom_data* Data_);
	//�R���X�g���N�g�ς݂��ǂ���(�ŏ���format����Ă��Ȃ��ꍇ�́A����s��)
	hmLib_boolian vcom_data_is_construct(vcom_data* Data);
	//�R���X�g���N�g
	void vcom_data_construct(vcom_data* Data, hmLib_cstring* mStr, vcom_ch_t Ch, vcom_id_t ID, vcom_err_t Err, hmLib_u16Fp_v Accessible);
	//�f�B�X�g���N�g(�ŏ���format��construct������Ă��Ȃ��ꍇ�́A����s��)
	void vcom_data_destruct(vcom_data* Data);
	//eof�f�[�^�ɂ���
	void vcom_data_set_eof(vcom_data* Data_);
	//eof�f�[�^���ǂ����𔻕ʂ���
	hmLib_boolian vcom_data_eof(vcom_data* Data_);
	//move
	void vcom_data_move(vcom_data* From, vcom_data* To);
	//swap
	void vcom_data_swap(vcom_data* Data1, vcom_data* Data2);

#ifdef __cplusplus
}
#endif
#
#endif
