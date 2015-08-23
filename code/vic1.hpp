#ifndef HMR_CODE_VIC1_INC
#define HMR_CODE_VIC1_INC 200
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
v2_00/140413 hmIto
	C++��
v1_02/140302 hmIto
	�^�C���A�E�g�񐔂ɏ����݂��A�I�[�o�[�����ꍇ�͑����̑[�u���Ƃ�悤�ɏC��
	�Ⴆ�΁A���݂��Ȃ����舶��Start�𖳌��ɌJ��Ԃ��Ă��܂���������
v1_01/140115 hmIto
	�`�F�b�N�T����crc8���̗p
v1_00/131224 hmIto
	�����[�X
*/

#define MaxTimeOutNum 5
//ch
#define ch_NULL		0xFF

#include<memory>
#include<hmLib_v3_06/type.h>
#include<homuraLib_v2/gate.hpp>
#include"client.hpp"
namespace hmr {
	namespace code {
		//vic1�\���̂̐錾
		class vic1 :public gate{
		public:
			enum error {
				error_NULL=0x00,
				error_SAVE_REJECTED=0x10,
				error_INCORRECT_SENDCNT=0x20,
				error_OVERFLOW=0x30,
				error_OVERREAD=0x40,
				error_INVALID_GETC=0x50
			};
		private:
			enum mode {
				mode_IDLE=0x00,
				mode_START=0x10,
				mode_SEND=0x20,
				mode_RESEND=0x21,
				mode_STOP=0x30,
				mode_RECV=0x40
			};
			enum cmd {
				cmd_ACK=0x00,
				cmd_NACK=0x10,
				cmd_START=0x20,
				cmd_STOP=0x30,
				cmd_DATA=0x40,
				cmd_ESC=0x50,
				cmd_WHAT=0x60,
				cmd_ERR=0xF0,
				cmd_NULL=0xFF
			};
		private:
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

		public:
			class timeout_functions {
			public:
				virtual void restart()=0;
				virtual void enable()=0;
				virtual void disable()=0;
			};
		private:
			timeout_functions* pTimeout;
		private:
			class buffer :public gate{
			private:
				vic1* VIC1;
				//����M�p�C���^�[�t�F�[�X
				client* pClient;
				//����M���L�o�b�t�@
				unsigned char* Begin;
				unsigned char* End;
				//����M���L�o�b�t�@�̃C�e���[�^
				unsigned char* GateItr;
				//VCom���̃C�e���[�^
				unsigned char* VComItr;
			public:
				//�o�b�t�@������
				buffer(vic1* VIC1_, unsigned char* BufBegin, unsigned char* BufEnd);
				//�o�b�t�@������
				buffer(vic1* VIC1_,client* pClient_, unsigned char* BufBegin, unsigned char* BufEnd);
				//�o�b�t�@�I�[��
				~buffer();
			public:
				bool open(client* pClient_);
				bool close();
			private:
				//��[�ł�������[
				void load();
			public:
				//========== vic1::buf ===========
				//���łɗ��p�\�ɂȂ��Ă��邩
				virtual bool is_open();
				//�o�b�t�@�ɏ������݉\��
				virtual bool can_putc();
				//�o�b�t�@�֏�������
				virtual void putc(unsigned char c);
				//�o�b�t�@�֏������񂾃f�[�^������
				virtual void flush();
				//�o�b�t�@����̓ǂݏo���\��
				virtual bool can_getc();
				//�o�b�t�@����̓ǂݏo��
				virtual unsigned char getc();
				//�o�b�t�@�ւ���̓ǂݏo������؂�ʒu�ɂ��邩
				virtual bool flowing();
			public:
				//========== 
				//�o�b�t�@�ւ̓ǂݍ��݂��Ȃ��������Ƃɂ���
				void cancel_get();
				//�o�b�t�@�̓����f�[�^�����ׂĔj�����Avcom�ɂ�Packet�I����ʒm
				void errorflush();
				//�o�b�t�@�ւ̏�������/�ǂݍ��݂��I�����A�o�b�t�@���N���A����
				void clear();
				//vcom���̂��f�[�^�������Ă��邩
				bool vcom_can_getc();
				//vcom���̂�flowing�͂ǂ��Ȃ��Ă��邩
				bool vcom_flowing();
				//vcom���̂Ƀf�[�^�擾���X�L�b�v������
				void vcom_skip_get();
				//vcom���̂�flowing�͂ǂ��Ȃ��Ă��邩
				unsigned char vcom_get_ch();
			};
			buffer Buffer;
		public:
			//vic1������������
			//	BufEnd - BufBegin - 1 ���ő�f�[�^�T�C�Y���ƂȂ�
			//	�ő�f�[�^�T�C�Y���͒ʐM���邷�ׂẴf�o�C�X�Ԃœ���ł���K�v������
			//	timeout�֐��̌Ăяo���^�C�~���O�ʍ��p�֐��Q�������n��
			vic1(client& rClient_
				, const unsigned char Ch
				, unsigned char* BufBegin
				, unsigned char* BufEnd
				, timeout_functions& timeout_);
			//vic1������������
			//	BufEnd - BufBegin - 1 ���ő�f�[�^�T�C�Y���ƂȂ�
			//	�ő�f�[�^�T�C�Y���͒ʐM���邷�ׂẴf�o�C�X�Ԃœ���ł���K�v������
			//	timeout�֐��̌Ăяo���^�C�~���O�ʍ��p�֐��Q�������n��
			vic1(const unsigned char Ch
				, unsigned char* BufBegin
				, unsigned char* BufEnd
				, timeout_functions& timeout_);			//vic1���I�[������
			~vic1();
		public:
			bool open(client& rClient_);
			bool close();
		private:
			//========== vic1::gate ===========
			//-------- Mode�J��4�֐� ----------------
			//�R�}���h���M������
			void inform_send();
			//�������̃R�}���h��M����
			void inform_recv();
			//�f�[�^�̗L�����m�F���āAIDLE����START�֑J�ڂ���֐�
			bool checkData();
		public:
			//timeout�����[�h���ꕔ�ς��Ă���ł�����
			//	vic1��timeout�R�}���h�đ��֐�
			void timeout();
		public:
			//========== gate interface =================
			//���łɗ��p�\�ɂȂ��Ă��邩
			virtual bool is_open();
			//��M�f�[�^�𓊂�����\���m�F 0:�s��,1:��
			virtual bool can_putc();
			//��M�f�[�^�𓊂������
			virtual void putc(unsigned char c_);
			//��M�f�[�^���t���b�V������igate�݊��p�AVIC���ł̏����͂Ȃ��j
			virtual void flush();
			//���M�f�[�^���Ăяo���\���m�F 0:�s��,1:��
			virtual bool can_getc();
			//���M�f�[�^���Ăяo��
			virtual unsigned char getc();
			//���M�f�[�^�𗬂ꑱ���Ă��邩�m�F�iVIC�̏ꍇ�́A�f�[�^�V�[�P���X������0�A����ȊO��1�j
			virtual bool flowing();
		public:
			//============ vic functions ================
			//�ʐM�������I�ɏI��������
			void force_end();
			//�G���[���擾����
			unsigned char error() { return Err; }
			//�G���[���N���A����
			void clear_error() { Err=error_NULL; }
		private:
			//��M�f�[�^1byte�ڔ��ʊ֐�
			void putc_1byte(unsigned char c);
			//��M�f�[�^2byte�ڔ��ʊ֐�
			void putc_2byte(unsigned char c1, unsigned char c2);
			//��M�f�[�^3byte�ڔ��ʊ֐�
			void putc_3byte(unsigned char c1, unsigned char c2, unsigned char c3);
			//��M�f�[�^3byte�ڔ��ʊ֐�
			void putc_4byte(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4);
		};
	}
}
#
#endif
