#ifndef HMR_TASK_INC
#define HMR_TASK_INC 100
#
/*=== task ===
�^�X�N�Ǘ��p�֐��Z�b�g
�E�^�X�N�����ԂƂƂ��ɓo�^����ƁA�w�莞�ԊԊu�Ń^�X�N���쓮����B
�E�^�X�N�z�X�g���̂́A�^�C�}�[����operator()�����I�ɌĂяo���Ď��s����B
	�E�o�^���ꂽ�^�X�N�́A�^�X�N�V�X�e���̌Ăяo���^�C�~���O�Ŏ��s�̗L���𔻒肳���B
�E�e�^�X�N�́A���s�A�ҋ@�A��~��3�̏�Ԃɕ��ނ����B
	�E���s�F�^�X�N�o�b�t�@�ɓo�^����A�w�莞�Ԃ��ƂɎ��s���Ă�����
	�E�ҋ@�F�^�X�N�o�b�t�@�ɓo�^����Ă��邪�A���s����Ȃ����
	�E��~�G�^�X�N�o�b�t�@�ɓo�^����Ă��Ȃ����
	�E���s/�ҋ@<=>��~�̑J�ڂ́A�o�b�t�@�����𔺂����ߒᑬ�@���΂炭���p���Ȃ��ꍇ�ɗ��p
	�E���s<=>�ҋ@�̑J�ڂ́A���Ԏw��ύX�݂̂̂��ߍ����@�ꎞ�I�ɒ�~���������ɗ��p
�E�^�X�N�̎��s�̗L�����̐���́A�^�X�N��������A�^�X�N�O������̓�ʂ�̕��@�Ő��䂷��B
	�E�^�X�N�O������̏���
		�Estart/quick_start/restart�Ń^�X�N���J�n/�ҋ@���w��
		�Estart�Ń^�X�N���J�n����B���łɓo�^�ς݂̃^�X�N�ɑ΂��Ď��s�����ꍇ�A���ԊԊu��ύX����B
			�E�w�莞�ԊԊu��0�̏ꍇ�́A�ҋ@��ԂɂȂ�B
			�E�w�莞�ԊԊu�����̏ꍇ�́A���̃^�X�N���~����B
		�Equick_start��start��菈���������B�������A���łɂ��̃^�X�N���o�^�ς݂łȂ��ꍇ�����g���Ȃ��B
			�E�o�^�ς݃^�X�N�ɑ΂��Ďg�p�����ꍇ�́A����`�̓���ƂȂ�B�֎~�B
		�Erestart�Ń^�X�N�̎��ԊԊu��ύX����B���̃^�X�N���o�^�ς݂łȂ��ꍇ�̓G���[��Ԃ��B
			�Estart�Ƃ̈Ⴂ�́A�V�K�o�^���ł��Ȃ����Ƃ����B
		�Estop�Ń^�X�N���~����B���̃^�X�N���o�^�ς݂łȂ��ꍇ�̓G���[��Ԃ��B
			�Erestart�Ŏw�莞�ԊԊu��-1�ɂ����ꍇ�Ɠ��������s���B
	�E�^�X�N��������̏���
		�E�o�^�֐��߂�l�����̒l�Ȃ�A��������s���
		�E�o�^�֐��߂�l��0�Ȃ�A�ҋ@��ԂɑJ��
		�E�o�^�֐��߂�l�����̒l�Ȃ�A��~

=== homuraLib_v2::task ===
v1_00/140712 hmIto
	cpp��
	task���O��Ԃ�V���ɐݒu���A�����̒�`�t�@�C���𕪗�
===hmrTask===
v2_00/131207 hmIto
	�j��I�ύX
	�X�̃^�X�N��counter���Ǘ�����悤�ɕύX
v1_06/130921 hmIto
	task_informStart/Stop�̓o�^�p�֐���ǉ�
v1_05/130905 hmIto
	�^�X�N���o�b�t�@����������Ɉꎞ��~�ɂ���u�ҋ@�v��ǉ�
	��ԑJ�ڂ̎�i���A�^�X�N�����ƃ^�X�N�O���Ő���
		�^�X�N��������́A�^�X�N�̖߂�l�Ń^�X�N�̎��s�Ԋu/�ҋ@/��~�����s�\
		�^�X�N�O������́Astart/quick_start/restart/stop�֐��Ő���
v1_04/130720 hmIto
	timer5�Ɉˑ����Ȃ��`�ɕύX
		task_interrupt_function�����荞�݊֐����Ŏ��s���Ă��炤
v1_03/121006 hmIto
	�^�X�N�֐�����task_stop�֐��̎g�p���֎~
	�^�X�N�֐��̖߂�l���^�̂Ƃ��ɁATask���~����@�\��ǉ�
	Task��task�ɕύX
v1_02/121005 hmIto
	�^�X�N�̊��荞�ݏ����֌W�̊֐��Q���Atimer5_XXX�n�֐��łƂ肠��������
		timer5_XXX�n�֐��Q�́AhmrDevice�����hmrDeviceConfig�Ɉˑ�
v1_01/120922 hmIto
	�C���^�[�t�F�C�X������������
		//�^�X�N�������֐�
		void task_initialize(void);
		//�^�X�N�I�[���֐�
		void task_finalize(void);
		//�^�X�N�ɓo�^�ł���m�F����
		task_bool_t task_can_start(void);
		//�^�X�N��o�^
		task_bool_t task_start(task_vFp_v TaskFp);
		//�^�X�N��o�^(��d�o�^�͉��)
		task_bool_t task_check_start(task_vFp_v TaskFp);
		//�^�X�N���J��
		task_bool_t task_stop(task_vFp_v TaskFp);
	120922Test_hmrTask.h/c�œ���m�F�ς�
	�^�X�N�̊��荞�݊֌W�������ȉ��̊֐���������
		//�^�X�N�Ŏg���Ă��銄�荞�݃^�C�}�[��L���ɂ���
		void _task_enable_timer(void);
		//�^�X�N�Ŏg���Ă��銄�荞�݃^�C�}�[�𖳌��ɂ���
		void _task_disable_timer(void);
		//�^�X�N�N���e�B�J���������ɁAtask_start,task_check_start,task_stop�����s����\�������銄�荞�݂����b�N
		void _task_lock_interrupt(void);
		//_task_lock_interrupt�Ń��b�N�����荞�݂�����
		void _task_unlock_interrupt(void);
		//timer5���荞�݊֐�(timer4�ƘA�����Ă���炵��
		void __attribute__((interrupt, no_auto_psv)) _T5Interrupt(void);
	timer4/timer5�̐ݒ���@�Ȃǂ��v�`�F�b�N
v1_00/120921 hmIto
	�^�X�N�̊�{�\�������݌v
*/
#include"task/client_interface.hpp"
#include"task/host_interface.hpp"
#include"task/functional_host.hpp"
#include"task/utility.hpp"
#include"task/watchdog.hpp"
#
#endif
