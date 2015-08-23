#ifndef HMR_IO_IODRIVERINTERFACE_INC
#define HMR_IO_IODRIVERINTERFACE_INC 100

/*--- homuraLib_v2::io::iodriver_interface ---
=== homuraLib_v2::io::iodriver_interface ===
v1_00/140707 hmIto
	�N���A��~����݂̂ɐ������āA�쐬
===IODriver::ItfIODriver===
v1_00/140126 hmIto
cExclusiveWorkIODriver��ǉ�
��X���b�h�֐���IO��r���I�ɋ쓮����^�C�v��IODriver
�r���I�Ȃ̂ŁA����IO�̓����쓮�͂ł��Ȃ�
void operator()(void)�֐������I�Ɏ��s���Ă��K�v������
v1_00/140118 amby
�쐬�J�n

�݌v
�@	�ʐM���J�n�����Task�̃I�y���[�^�����I�ɌĂ�
	�@
	 */

#include <memory>
#include <functional>
namespace hmr {
	namespace io{
		//IODriver�̊��N���X
		class iodriver_interface {
		public:
			typedef unsigned int ch_t;
		public://--- iodriver_interface ---
			// start�֐� : Ch �Ŏw�肳�ꂽ���̂̒ʐM���J�n����
			virtual bool start(ch_t Ch_)=0;
			// Ch�w��stop�֐� : Ch �Ŏw�肳�ꂽ���̂̒ʐM���I������
			virtual void stop(ch_t Ch_)=0;
			// stop�֐� : �����Ă�����̂����ׂĎ~�߂Ă��܂�
			virtual void stop()=0;
			// Ch�w��is_start�֐� : Ch �Ŏw�肳�ꂽ���̂��ʐM������Ԃ�
			virtual bool is_start(ch_t Ch_)const=0;
			// is_start�֐� : �����ꂩ���ʐM������Ԃ�
			virtual bool is_start()const=0;
		};
	}
}
#
#endif
