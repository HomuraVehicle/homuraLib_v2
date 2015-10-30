#ifndef HMR_TASK_C_INC
#define HMR_TASK_C_INC 200
#
#include<cstdlib>
#include"functional_task.hpp"
namespace hmr {
	namespace machine {
		namespace service {
			namespace functask {
				namespace {
					//===�^�X�N�o�b�t�@�p�֐�===
					void buf_initialize(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@�̃T�C�Y���擾����
//					uint8 buf_size(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@���󂩊m�F����
//					bool buf_empty(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@����t���m�F����
					bool buf_full(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@�̐擪�A�h���X���擾����
					task* buf_begin(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@�̖����A�h���X���擾����
					task* buf_end(taskmaster* TaskMaster);
					//�^�X�N�o�b�t�@����A�^�X�N��T���Ă���
					task* buf_find(taskmaster* TaskMaster, function Fp);
					//�^�X�N�o�b�t�@�ɗv�f��ǉ�����
					bool buf_insert(taskmaster* TaskMaster, function Fp, sint16 Interval, inform TaskInformStopFp);
					//�^�X�N�o�b�t�@�̗v�f���폜����
					bool buf_erase(taskmaster* TaskMaster, task* itr);


					//===�^�X�N�o�b�t�@�p�֐�===
					void buf_initialize(taskmaster* TaskMaster) {
						if(TaskMaster==0)return;
						task* itr=buf_begin(TaskMaster);
						while(itr!=buf_end(TaskMaster)) {
							itr->Fp=0;
							itr->InformStop=0;
							itr->Interval=0;
							++itr;
						}
					}
					//�^�X�N�o�b�t�@�̃T�C�Y���擾����
//					uint8 buf_size(taskmaster* TaskMaster) {
//						if(TaskMaster==0)return 0;
//						return TaskMaster->Size;
//					}
					//�^�X�N�o�b�t�@���󂩊m�F����
//					bool buf_empty(taskmaster* TaskMaster) {
//						if(TaskMaster==0)return 0;
//						return TaskMaster->Size==0;
//					}
					//�^�X�N�o�b�t�@����t���m�F����
					bool buf_full(taskmaster* TaskMaster) {
						if(TaskMaster==0)return 1;
						return TaskMaster->Size==TaskMaster->MaxSize;
					}
					//�^�X�N�o�b�t�@�̐擪�A�h���X���擾����
					task* buf_begin(taskmaster* TaskMaster) {
						if(TaskMaster==0)return 0;
						return TaskMaster->Buf;
					}
					//�^�X�N�o�b�t�@�̖����A�h���X���擾����
					task* buf_end(taskmaster* TaskMaster) {
						if(TaskMaster==0)return 0;
						return TaskMaster->Buf+TaskMaster->Size;
					}
					//�^�X�N�o�b�t�@����A�^�X�N��T���Ă���
					task* buf_find(taskmaster* TaskMaster, function Fp) {
						if(TaskMaster==0)return 0;
						if(Fp==0)return 0;
						task* itr;

						//�I�[�܂Ŋm�F���āA����^�X�N�̗L�����m�F����
						itr=buf_begin(TaskMaster);
						while(itr!=buf_end(TaskMaster)) {
							if(itr->Fp==Fp)break;
							++itr;
						}

						if(itr==buf_end(TaskMaster))return 0;
						return itr;
					}
					//�^�X�N�o�b�t�@�ɗv�f��ǉ�����
					bool buf_insert(taskmaster* TaskMaster, function Fp, sint16 Interval, inform TaskInformStopFp) {
						if(TaskMaster==0)return 1;
						if(Fp==0)return 1;
						//task�o�b�t�@�������ς��Ȃ�G���[��Ԃ�
						if(buf_full(TaskMaster))return 1;

						//�o�^���e�𖖔��ɏ������݁A�T�C�Y�𑝂₷
						TaskMaster->Buf[TaskMaster->Size].Fp=Fp;
						TaskMaster->Buf[TaskMaster->Size].Interval=Interval;
						TaskMaster->Buf[TaskMaster->Size].InformStop=TaskInformStopFp;
						++TaskMaster->Size;

						//task�����߂ēo�^���ꂽ�ꍇ�́Ainform�֐����Ăяo��
						if(TaskMaster->Size==1 && TaskMaster->Fp_informStart)TaskMaster->Fp_informStart();

						return 0;
					}
					//�^�X�N�o�b�t�@�̗v�f���폜����
					bool buf_erase(taskmaster* TaskMaster, task* itr) {
						task* pre=0;
						if(TaskMaster==0 || itr==0)return 1;
						if(itr<buf_begin(TaskMaster) || buf_end(TaskMaster)<=itr)return 1;

						//�w��itr�Ȍ��
						while(1) {
							pre=itr++;
							if(itr==buf_end(TaskMaster))break;
							pre->Fp=itr->Fp;
							pre->Interval=itr->Interval;
							pre->InformStop=itr->InformStop;
							itr->Fp=0;
							itr->InformStop=0;
							itr->Interval=0;
						}

						--TaskMaster->Size;
						if(TaskMaster->Size==0 && TaskMaster->Fp_informStop)TaskMaster->Fp_informStop();

						return 0;
					}
				}
				//===�^�X�N�p���J�֐�===
				//�^�X�N�V�X�e���t�H�[�}�b�g
				void format(taskmaster* TaskMaster) {
					if(TaskMaster==0)return;
					TaskMaster->Buf=0;
					TaskMaster->Counter=0;
					TaskMaster->DynamicFlag=false;
					TaskMaster->Fp_informStart=0;
					TaskMaster->Fp_informStop=0;
					TaskMaster->MaxSize=0;
					TaskMaster->Size=0;
				}
				//�^�X�N�V�X�e��������������Ă��邩
				bool is_constructed(taskmaster* TaskMaster) {
					if(TaskMaster==0)return 0;
					return TaskMaster->Buf!=0;
				}
				//�^�X�N�V�X�e���������֐�
				void construct_dynamic(taskmaster* TaskMaster, uint8 MaxSize) {
					if(TaskMaster==0)return;
					TaskMaster->Buf=(task*)std::malloc(sizeof(task)*MaxSize);
					TaskMaster->Counter=0;
					TaskMaster->DynamicFlag=true;
					TaskMaster->Fp_informStart=0;
					TaskMaster->Fp_informStop=0;
					TaskMaster->MaxSize=MaxSize;
					TaskMaster->Size=0;
					buf_initialize(TaskMaster);
				}
				//�^�X�N�V�X�e���������֐�
				void construct_static(taskmaster* TaskMaster, task* Begin, task* End) {
					if(TaskMaster==0 || Begin==0 || End==0)return;
					TaskMaster->Buf=Begin;
					TaskMaster->Counter=0;
					TaskMaster->DynamicFlag=false;
					TaskMaster->Fp_informStart=0;
					TaskMaster->Fp_informStop=0;
					TaskMaster->MaxSize=(uint8)(End-Begin);
					TaskMaster->Size=0;
					buf_initialize(TaskMaster);
				}
				//�^�X�N�V�X�e���I�[���֐�	
				void destruct(taskmaster* TaskMaster) {
					task* itr=0;
					if(TaskMaster==0)return;
					itr=buf_begin(TaskMaster);
					while(itr!=buf_end(TaskMaster)) {
						if(itr->InformStop)itr->InformStop();
						++itr;
					}
					if(TaskMaster->Fp_informStop)TaskMaster->Fp_informStop();
					if(TaskMaster->DynamicFlag)std::free(TaskMaster->Buf);
					TaskMaster->Buf=0;
					TaskMaster->Counter=0;
					TaskMaster->DynamicFlag=false;
					TaskMaster->Fp_informStart=0;
					TaskMaster->Fp_informStop=0;
					TaskMaster->MaxSize=0;;
					TaskMaster->Size=0;
				}
				//�^�X�N�ɓo�^�ł���m�F����
				bool can_start(taskmaster* TaskMaster) {
					if(TaskMaster==0)return 0;
					return !buf_full(TaskMaster);
				}
				//�^�X�N��o�^(��d�o�^�͉��)
				bool start(taskmaster* TaskMaster, sint16 Interval, function TaskFp, inform TaskInformStopFp) {
					task* itr=0;
					if(TaskMaster==0 || TaskFp==0)return 1;

					//�^�X�N��T���ɍs��
					itr=buf_find(TaskMaster, TaskFp);

					//�V�K�̏ꍇ
					if(itr==buf_end(TaskMaster)) {
						//�w���Ԃ����̒l�Ȃ�A�G���[��߂�
						if(Interval<0)return 1;

						//�o�^�ł��Ȃ��ꍇ�́A�G���[��߂�
						if(buf_full(TaskMaster))return 1;

						//�^�X�N��o�^
						return buf_insert(TaskMaster, TaskFp, Interval, TaskInformStopFp);
					}//�����̏ꍇ
					else {
						//��~�v���Ȃ�A�폜����
						if(Interval<0)return buf_erase(TaskMaster, itr);

						//�o�^�C���Ȃ�A�w�莞�Ԃ�������
						itr->Interval=(uint16)(Interval);
						itr->Fp=TaskFp;
						itr->InformStop=TaskInformStopFp;
						return 0;
					}
				}
				//�^�X�N��o�^�i��d�o�^�ł��Ă��܂��̂ŁA�댯�j
				bool quick_start(taskmaster* TaskMaster, sint16 Interval, function TaskFp, inform TaskInformStopFp) {
					if(TaskMaster==0 || TaskFp==0)return 1;
					//�^�X�N�̎w�莞�Ԃ����������ꍇ�̓G���[��߂�
					if(Interval<=0)return 1;

					//�o�^�ł��Ȃ��ꍇ�̓G���[��߂�
					if(!can_start(TaskMaster))return 1;

					//�^�X�N��ǉ�
					buf_insert(TaskMaster, TaskFp, (uint16)Interval, TaskInformStopFp);

					return 0;
				}
				//�^�X�N��ݒ�ύX/�폜
				bool restart(taskmaster* TaskMaster, function TaskFp, sint16 Interval) {
					task* itr=0;
					if(TaskMaster==0 || TaskFp==0)return 1;

					//�^�X�N��T���ɍs��
					itr=buf_find(TaskMaster, TaskFp);

					//�V�K�̏ꍇ�A�G���[��Ԃ�
					if(itr==buf_end(TaskMaster))return 1;

					//��~�v���Ȃ�A�폜����
					if(Interval<0) {
						buf_erase(TaskMaster, itr);
					}

					//�o�^�C���Ȃ�A�w�莞�Ԃ�������
					itr->Interval=(uint16)(Interval);
					return 0;
				}
				//�^�X�N���J��
				bool stop(taskmaster* TaskMaster, function TaskFp) {
					if(TaskMaster==0 || TaskFp==0)return 1;
					return restart(TaskMaster, TaskFp, -1);
				}
				//�^�X�N�̎��s
				void interrupt_function(taskmaster* TaskMaster, sint16 Interval) {
					task* itr=0;
					uint16 NewCounter;
					if(TaskMaster==0)return;

					//�V���ȃJ�E���^�[���Ԃ��v�Z����
					NewCounter=((uint16)(TaskMaster->Counter)+(uint16)(Interval))%0x8000;

					//�^�X�N�o�b�t�@�����Ɋm�F����
					itr=buf_begin(TaskMaster);
					while(itr<buf_end(TaskMaster)) {
						//�O��̃J�E���^�[�ƍ���̃J�E���^�[�̊Ԃ�Interval���ׂ��ł���΁A���s
						if(itr->Interval>0 && TaskMaster->Counter/itr->Interval != NewCounter/itr->Interval) {
							//�^�X�N�����s
							Interval=itr->Fp(itr->Interval);
							//�I���v�����������ꍇ
							if(Interval<0) {
								buf_erase(TaskMaster, itr);
								continue;
							}
							//�v���ʂ�̃C���^�[�o���ɃZ�b�g���Ȃ���
							itr->Interval=Interval;
						}
						++itr;
					}

					//�J�E���^�[��V�������Ă���
					TaskMaster->Counter=NewCounter;
				}
				//�^�X�N�V�X�e���̓o�^���[���łȂ��Ȃ����ꍇ�ɒʒm
				void resgist_informStart(taskmaster* TaskMaster, vFp_v Fp_) {
					if(TaskMaster==0 || Fp_==0)return;
					TaskMaster->Fp_informStart=Fp_;
				}
				//�^�X�N�V�X�e�����炷�ׂĂ̓o�^���������Əꍇ�ɒʒm
				void resgist_informStop(taskmaster* TaskMaster, vFp_v Fp_) {
					if(TaskMaster==0 || Fp_==0)return;
					TaskMaster->Fp_informStop=Fp_;
				}
			}
		}
	}
}
#
#endif
