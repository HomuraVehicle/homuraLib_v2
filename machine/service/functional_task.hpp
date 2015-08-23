#ifndef HMR_MACHINE_SERVICE_FUNCTIONALTASK_INC
#define HMR_MACHINE_SERVICE_FUNCTIONALTASK_INC 200
#
/*===hmrTask===
�^�X�N�Ǘ��p�֐��Z�b�g
�E�^�X�N�����ԂƂƂ��ɓo�^����ƁA�w�莞�ԊԊu�Ń^�X�N���쓮����B
�E�^�X�N�V�X�e�����̂́A�^�C�}�[����task_interrupt_function�����I�ɌĂяo���Ď��s����B
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

=== hmr::machine::service::functional_task ===
v2_00/141201 hmIto
	cpp��
===hmrTask===
v1_08/141122 hmIto
	cpp���痘�p�ł���悤�ɕύX
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
#include<homuraLib_v2/type.hpp>
namespace hmr {
	namespace machine {
		namespace service {
			namespace functask{
				//*******************************//
				typedef sint16(*s16Fp_s16)(sint16);
				typedef void(*vFp_v)(void);
				typedef s16Fp_s16 function;
				typedef vFp_v inform;
				//�^�X�N�N���X
				typedef struct {
					//�^�X�N�����Ŏ��s�������֐�
					function Fp;
					//�^�X�N��������~���ꂽ�ꍇ�̒ʒm�֐�
					inform InformStop;
					//�^�X�N�̎��s�Ԋu
					uint16 Interval;
				}task;
				typedef struct {
					//���ԗݐϗp�ϐ�
					uint16 Counter;
					//�^�X�N�V�X�e���̓o�^���[���łȂ��Ȃ����ꍇ�ɒʒm
					vFp_v Fp_informStart;
					//�^�X�N�V�X�e�����炷�ׂĂ̓o�^���������Əꍇ�ɒʒm
					vFp_v Fp_informStop;
					//�^�X�N�p�֐��z��
					task* Buf;
					//�ő�̃^�X�N�̐�
					uint8 MaxSize;
					//���݂̃^�X�N�̐�
					uint8 Size;
					//dynamic�Ɋm�ۂ������ǂ����̃t���O
					bool DynamicFlag;
				}taskmaster;

				//===�^�X�N�o�b�t�@�p�֐�===
				//�^�X�N�V�X�e���t�H�[�}�b�g
				void format(taskmaster* TaskMaster);
				//�^�X�N�V�X�e��������������Ă��邩
				bool is_constructed(taskmaster* TaskMaster);
				//�^�X�N�V�X�e���������֐�
				void construct_dynamic(taskmaster* TaskMaster, uint8 MaxSize);
				//�^�X�N�V�X�e���������֐�
				void construct_static(taskmaster* TaskMaster, task* Begin, task* End);
				//�^�X�N�V�X�e���I�[���֐�	
				void destruct(taskmaster* TaskMaster);
				//�^�X�N��V�K�o�^�ł���m�F����
				bool can_start(taskmaster* TaskMaster);
				//�^�X�N��o�^�i��d�o�^�͉���j
				bool start(taskmaster* TaskMaster, sint16 Interval, function TaskFp, inform TaskInformStopFp);
				//�^�X�N��o�^�i��d�o�^�ł��Ă��܂��̂ŁA�댯�j
				bool quick_start(taskmaster* TaskMaster, sint16 Interval, function TaskFp, inform TaskInformStopFp);
				//�^�X�N��ݒ�ύX
				bool restart(taskmaster* TaskMaster, function TaskFp, sint16 Interval);
				//�^�X�N���폜
				bool stop(taskmaster* TaskMaster, function TaskFp);
				//�^�X�N�̎��s
				void interrupt_function(taskmaster* TaskMaster, sint16 Interval);
				//�^�X�N�V�X�e���̓o�^���[���łȂ��Ȃ����ꍇ�ɒʒm
				void resgist_informStart(taskmaster* TaskMaster, vFp_v Fp);
				//�^�X�N�V�X�e�����炷�ׂĂ̓o�^���������Əꍇ�ɒʒm
				void resgist_informStop(taskmaster* TaskMaster, vFp_v Fp);
			}
		}
	}
}
#
#endif
