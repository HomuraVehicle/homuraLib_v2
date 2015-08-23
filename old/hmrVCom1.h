#ifndef HMR_VCOM1_INC
#define HMR_VCOM1_INC 104
#
#include"hmrVComData.h"
/*---vcom1---
vcom_data��vcom1�`���̃f�[�^������Ƃ�ϊ�����

=== vcom1 ===
v1_04/140302 hmIto
	vcom1_skip_get�֐���ǉ�
v1_03/140118 hmIto
	0byte�f�[�^�𑗐M����ۂɁA�f�[�^�̔j�������Ă��Ȃ��������߁A���x�������f�[�^�𑗂낤�Ƃ��Ă��܂��Ă��������C��
v1_02/140110 hmIto
	vcom_data��id�ɂ������K��ύX�ɑΉ�
v1_01/131224 hmIto
	�f�o�b�O�f�o�b�O�f�o�b�O
	�قڃe�X�g���������͂�
v1_00/131220 hmIto
	VMC���番�����č쐬
*/
#ifdef __cplusplus
extern "C"{
#endif

	typedef hmLib_bFp_v vcom1_bFp_v;
	typedef void(*vcom1_vFp_pdata)(vcom_data*);
	typedef hmLib_boolian(*vcom1_bFp_pstr_strsize)(hmLib_cstring *, hmLib_cstring_size_t);
	typedef struct {
		//============== recv ===============
		unsigned char RecvMode;
		vcom_data RecvData;
		vcom_ch_t RecvCh;
		vcom_size_t RecvCnt;
		//============== send ===============
		unsigned char SendMode;
		vcom_data SendData;
		vcom_size_t SendCnt;
		hmLib_boolian SendEof;
		//============== functions ===============
		vcom1_bFp_v _fp_can_move_push;					//��M�f�[�^���Ԃ�����ł�����
		vcom1_vFp_pdata _fp_move_push;					//��M�f�[�^���Ԃ�����
		vcom1_bFp_v _fp_can_move_pop;					//���M�f�[�^�𔲂��o���Ă�����
		vcom1_vFp_pdata _fp_move_pop;					//���M�f�[�^�𔲂��o��
		vcom1_bFp_pstr_strsize _fp_cstring_construct;	//cstring�m�ۈ˗��֐�	
	}vcom1;
	//vcom1������
	void vcom1_initialize(vcom1* pVCom1,
		vcom1_bFp_v Fp_can_move_push,
		vcom1_vFp_pdata Fp_move_push,
		vcom1_bFp_v Fp_can_move_pop,
		vcom1_vFp_pdata Fp_move_pop,
		vcom1_bFp_pstr_strsize Fp_cstring_construct);
	//vcom1�I�[��
	void vcom1_finalize(vcom1* pVCom1);
	//======== gate interface =======
	//send�ł��邩�H
	hmLib_boolian vcom1_can_getc(vcom1* pVCom);
	//���M�������1byte�擾����
	unsigned char vcom1_getc(vcom1* pVCom);
	//���M������eof�ʒu=Pac�I�[���ǂ��������m����
	hmLib_boolian vcom1_flowing(vcom1* pVCom);
	//recv�ł��邩�H(�P��return 1)
	hmLib_boolian vcom1_can_putc(vcom1* pVCom);
	//��M�������1byte�^����
	void vcom1_putc(vcom1* pVCom, unsigned char c);
	//flush����(eof������=Pac�����)
	void vcom1_flush(vcom1* pVCom);

	//======= vcom functions ========
	//���M��ch���擾����
	unsigned char vcom1_get_ch(vcom1* pVCom);
	//���M���L�����Z������i���񑗐M�́A���̃f�[�^�j
	void vcom1_cancel_get(vcom1* pVCom);
	//���M���X�L�b�v����i���񑗐M�́A���̃f�[�^�j
	void vcom1_skip_get(vcom1* pVCom);
	//��M����ch���擾����
	void vcom1_put_ch(vcom1* pVCom, unsigned char Ch);
	//��M���L�����Z������
	void vcom1_cancel_put(vcom1* pVCom);

#ifdef __cplusplus
}
#endif
#
#endif
