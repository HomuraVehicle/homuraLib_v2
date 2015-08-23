#ifndef HMR_VCOM_VCOM_INC
#define HMR_VCOM_VCOM_INC 100
#
/*---vcom---
gate���ǂ��̕����񑗎�M�p�t�H�[�}�b�g
*/
namespace hmr {
	class gate {
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
	};
}
#
#endif
