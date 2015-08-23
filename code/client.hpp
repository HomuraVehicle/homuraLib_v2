#ifndef HMR_CODE_CLIENT_INC
#define HMR_CODE_CLIENT_INC 200
#
#include<homuraLib_v2/gate.hpp>
/*--- code::client ---
code�`���̒ʐM���˗����邽�߂�client�N���X���`���Ă���

v1_00/140413 hmIto
	cpp��
=== code ===
v1_04/140302 hmIto
	skip_get�֐���ǉ�
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
namespace hmr {
	namespace code {
		class client :public gate {
		public:
			//======== gate interface =======
			//���łɗ��p�\�ɂȂ��Ă��邩
			virtual bool is_open()=0;
			//send�ł��邩�H
			virtual bool can_getc()=0;
			//���M�������1byte�擾����
			virtual unsigned char getc()=0;
			//���M������eof�ʒu=Pac�I�[���ǂ��������m����
			virtual bool flowing()=0;
			//recv�ł��邩�H(�P��return 1)
			virtual bool can_putc()=0;
			//��M�������1byte�^����
			virtual void putc(unsigned char c)=0;
			//flush����(eof������=Pac�����)
			virtual void flush()=0;
		public:
			//======= vcom functions ========
			//���M��ch���擾����
			virtual unsigned char get_ch()=0;
			//���M���L�����Z������
			virtual void cancel_get()=0;
			//���M���X�L�b�v����
			virtual void skip_get()=0;
			//��M����ch���擾����
			virtual void put_ch(unsigned char Ch)=0;
			//��M���L�����Z������
			virtual void cancel_put()=0;
		};
	}
}
#
#endif
