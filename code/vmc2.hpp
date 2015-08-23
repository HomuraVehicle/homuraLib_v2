#ifndef HMR_CODE_VMC2_INC
#define HMR_CODE_VMC2_INC 200
#
/*=== vcom::vmc2===
�قނ�̑��c�҂�MainPic���Ȃ���ʐM�̒ʐM�t�H�[�}�b�g���K�肷��t�@�C��
�o�b�t�@����̑����֐���p�ӂ��Ă��ƁA
��M�f�[�^��VMC1�֐��Q�Ɏ󂯓n���ƁA�o�b�t�@�ւ̒ǉ��������s���Ă����
���M�f�[�^��VMC1�֐��Q�ŗv������ƁA�o�b�t�@��ǂݏo���ēK���ȑ��M������Ԃ�


=== hmrVMC2 ===
v2_00/140413 hmIto
	C++��
v1_02/131224 hmIto
	vmc2 functions�̃G���[�ɂ������֐�����ύX
v1_01/131220 hmIto
	code�����̐V�K�ʐM�t�H�[�}�b�g�Avmc2���ЂƂ܂�����
v1_00/131217 hmIto
	VCom���ʃC���^�[�t�F�[�X�ɑΉ���ƊJ�n
=== hmrVMC1 ===
v4_00/130223 hmIto
	c�t�@�C������extern C���s�v�������̂��폜
	vmc1_create�̈���������VMCID���Avmc1_initialize�Ɉړ�
	boolian��bool�ɕύX
	hmrType��include���폜
	can_iniSendPac()/can_iniRecvPac()��ǉ�
		���ꂼ��A���M/��M���̃p�P�b�g���J����
v3_00/130211 hmIto
	can_existSendDat�֐���vmc_interface�ɒǉ�
		existSendDat�֐������s�\����₢���킹��֐�
	getSendErr/getRecvErr/clearSendErr/clearRecvErr�֐���ǉ�
		���ꂼ��C�Ō�ɔ�����������M���̃G���[���擾/�N���A����
		�G���[�������ł��C���ɓ���Ɏx��͂Ȃ��i�����I�Ƀp�P�b�g�����Ȃǂ��āC����M�͏�������Ă���j
v2_02/121128 hmIto
	hmCLib��hmLib�̓����ɔ����AhmLib_v3_03�ɑΉ�����悤�ύX
	hmrVMC1���g��C++�ɑΉ�
v2_01/121027 hmIto
	vmc1_get�ɂ����āC0byte�f�[�^���G���[�����ɂ��Ă����̂��C��
v2_00/121020 hmIto
	������VMC�̓����^�p�ɑΉ�
		3byte�̃f�o�C�X�ŗL��VMCID��V���Ɏw��
		���M��ID�Ƒ��M��ID��VMC�\���̂ɒǉ�
		��M���ɂ͎��g��VMCID�����̒ʐM�̂ݎ�M
		���g��VMCID��create�֐��Ŏw��
		���M���ɂ͑��M���VMCID���w�肷�邱�Ƃ��K�v
		�����VMCID��initialize�֐��Ŏw��
		�Ȍ�̋K�i�́A#{VMCID[3]}.....##crlf
		static��STRT,TRMN�͔p�~
	�֐�����force_ini_get/put����force_end_get/put�ɕύX
	finalize�ǉ�
	create���ł�initialize�������s��p�~
v1_04/121014 hmIto
	force_ini_get/put�֐���ǉ�
	timeout�ȂǁAVMC1�̎�M�̔@���Ɋւ�炸�����ɃX�e�[�^�X������������̂Ɏg�p
v1_03/121013 hmIto
	���O���VMC1��vmc1�ɕύX
v1_02/121008 hmIto
	�eDat��ID�ɂ��Ă�VMC1�ŊǗ�����悤�ύX�@���M�E��M���̊֐��`�ȂǕύX
	�f�[�^�T�C�Y�̃o�b�t�@�T�C�Y��2byte�֊g��
		DatID[1],DatSize[2],Dat[DatSize]�Ƀf�[�^�̃t�H�[�}�b�g��ύX
		#PacID[3],Dat...,##crlf�Ƀp�P�b�g�̃t�H�[�}�b�g��ύX
		121009Test_hmrCom_hmrVMC1.c�ɂāA����m�F�ς�
v1_01/120922 hmIto
	�ЂƂ܂����삪���肵�Ă��邱�Ƃ��m�F�i120922Test_hmrCom_hmrVMC1.c�ɂē���m�F�ς݁j
v1_00/120921 hmIto
	�o�[�W�����Ǘ��J�n
*/
#include<hmLib_v3_06/type.h>
#include<homuraLib_v2/gate.hpp>
#include"client.hpp"
namespace hmr {
	namespace code {
		class vmc2 :public gate{
		private:
			static const unsigned char PacStrt[4];
			static const unsigned char PacStrtSize;
			static const unsigned char PacTrmn[4];
			static const unsigned char PacTrmnSize;
		private:
			enum putmode {
				putmode_IDLE=0x00, putmode_DATA=0x01
			};
			enum getmode {
				getmode_STRT=0x00, getmode_DATA=0x01, getmode_TRMN=0x02
			};
		public:
			enum puterr {
				puterr_NULL=0x00, puterr_RECVREJECTED=0x10, puterr_INVALID_PUTC=0x20
			};
			enum geterr {
				geterr_NULL=0x00, geterr_SENDREJECTED=0x10, geterr_INVALID_GETC=0x20
			};
		private:
			//���g��Ch
			unsigned char Ch;
			//��M���[�h
			unsigned char RecvMode;
			//��M���[�h�̃J�E���^
			unsigned char RecvCnt;
			//��M�f�[�^�̑��MCh
			unsigned char RecvCh;
			//��M���[�h�̃G���[
			unsigned char RecvErr;
			//���M���[�h
			unsigned char SendMode;
			//���M���[�h�̃J�E���^
			unsigned char SendCnt;
			//���M���[�h�̃G���[
			unsigned char SendErr;
			//VCom
			client* pVCom1;
		public:
			//vmc2������������
			vmc2(unsigned char Ch);
			//vmc2������������iVCom�w��Łj
			vmc2(unsigned char Ch, client& rVCom1);
			//vmc2���I�[������
			~vmc2();
		public:
			bool open(client& rVCom1);
			bool close();
		public:
			//���łɗ��p�\�ɂȂ��Ă��邩
			virtual bool is_open();
			//�t���b�V������i�����������Ȃ��j
			virtual void flush();
			//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
			virtual bool can_putc();
			//��M�f�[�^�𓊂������
			virtual void putc(unsigned char c);
			//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
			virtual bool can_getc();
			//���M�f�[�^���Ăяo��
			virtual unsigned char getc();
			//���M�f�[�^��eof�ʒu�łȂ������m�F����
			virtual bool flowing();
		public:
			//��M�������I�ɏI��������
			void force_end_get();
			//���M�������I�ɏI��������
			void force_end_put();
			//���M�G���[���擾����
			unsigned char error_of_get() { return SendErr; }
			//���M�G���[���N���A����
			void clear_error_of_get() { SendErr=0; }
			//��M�G���[���擾����
			unsigned char error_of_put() { return RecvErr; }
			//��M�G���[���N���A����
			void clear_error_of_put() { RecvErr=0; }
		};
	}
}
#
#endif
