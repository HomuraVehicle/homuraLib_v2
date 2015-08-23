#ifndef CRC8_INC
#define CRC8_INC 100
#
/*
---CRC8---
����璷�������ACRC8�̃o�C�g�����@�\��񋟂��郉�C�u����

crc8_bitshift_putc/puts
	�r�b�g�V�t�g�^crc8���Z�֐�
	���x�͒x�����A������������Ȃ�
crc8_table_putc/puts
	�e�[�u���^crc8���Z�֐�
	���������A�e�[�u���p������256byte�������
	initialize/finalize���K�v�ƂȂ�
	is_valid�֐��Ńe�[�u�����L�������m�F�ł���
crc8_putc/puts
	�e�[�u�����g�p�\�Ȃ�e�[�u���^�A�����łȂ���΃r�b�g�V�t�g�^��p����crc8���Z���s��

===crc8===
v1_00/140115 hmIto
	�쐬
*/
#ifdef __cplusplus
extern "C"{
#endif
	//�r�b�g�V�t�g�ɂ��CRC8�̒P�o�C�g�v�Z
	unsigned char crc8_bitshift_putc(unsigned char crc8, unsigned char data);
	//�r�b�g�V�t�g�ɂ��CRC8�̕����o�C�g�v�Z
	unsigned char crc8_bitshift_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8�e�[�u���ɂ��CRC8�̒P�o�C�g�v�Z
	unsigned char crc8_table_putc(unsigned char crc8, unsigned char data);
	//CRC8�e�[�u���ɂ��CRC8�̕����o�C�g�v�Z
	unsigned char crc8_table_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8�̒P�o�C�g�v�Z
	unsigned char crc8_putc(unsigned char crc8, unsigned char data);
	//CRC8�̕����o�C�g�v�Z
	unsigned char crc8_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8�e�[�u���L�����m�F
	unsigned char crc8_table_is_valid();
	//CRC8�e�[�u���̏�����
	void crc8_table_initialize();
	//CRC8�e�[�u���̃��������蓖�ď�����
	void crc8_table_placement_initialize(unsigned char Table[256]);
	//CRC8�e�[�u���̏I�[��
	void crc8_table_finalize();
#ifdef __cplusplus
}
#endif
#
#endif
