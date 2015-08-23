#ifndef HMR_IODRIVER_EXCULUSIVEIODRIVER_INC
#define HMR_IODRIVER_EXCULUSIVEIODRIVER_INC 100

/*--- homura IO Driver ---
=== homuraLib_v2::io::exclusive_iodriver ===
v1_00/140707 hmIto
	Pic������g����悤�ɋ@�\����
===IODriver::ExclusiveFunctionalIODriver===
v1_00/140126 hmIto
cExclusiveFunctionalIODriver��ǉ�
	��X���b�h�֐���IO��r���I�ɋ쓮����^�C�v��IODriver
	�r���I�Ȃ̂ŁA����IO�̓����쓮�͂ł��Ȃ�
	void operator()(void)�֐������I�Ɏ��s���Ă��K�v������
 */

#include <map>
#include<homuraLib_config/allocator.hpp>
#include "io_interface.hpp"
#include "iodriver_interface.hpp"

namespace hmr {
	namespace io {
		//IO��r���I�ɋ쓮����^�C�v��IODriver
		class exclusive_iodriver:public iodriver_interface{
		private:
			typedef std::map<ch_t, io_interface&,std::less<ch_t>,allocator> io_map;
			typedef std::pair<ch_t, io_interface&> io_pair;
		private:
			io_map TaskMap;
			ch_t TaskCh;
		public:
			exclusive_iodriver() :TaskCh(0) {}
		public://--- iodriver_interface ---
			// start�֐� : Ch �Ŏw�肳�ꂽ���̂̒ʐM���J�n����
			virtual bool start(ch_t Ch_) {
				auto itr=TaskMap.find(Ch_);

				// ����Ch�����݂��Ȃ���΁A�G���[��Ԃ�
				if(itr == std::end(TaskMap)) return true;
				itr->second.start();

				//Ch�o�^
				TaskCh=Ch_;

				return false;
			}
			// Ch�w��stop�֐� : Ch �Ŏw�肳�ꂽ���̂̒ʐM���I������
			virtual void stop(ch_t Ch_) {
				if(TaskCh!=Ch_)return;
				stop();
			}
			// stop�֐� : �����Ă�����̂����ׂĎ~�߂Ă��܂�
			virtual void stop() {
				//���݃^�X�N���쓮���Ă��Ȃ���΁A�G���[��Ԃ�
				if(TaskCh==0)return;

				//�^�X�N���N���A
				auto itr=TaskMap.find(TaskCh);

				// ����Ch�����݂��Ȃ���΁A�G���[��Ԃ�
				if(itr == std::end(TaskMap)) return;

				//Task���~�߂�
				itr->second.stop();
				TaskCh=0;
			}
			// Ch�w��is_start�֐� : Ch �Ŏw�肳�ꂽ���̂��ʐM������Ԃ�
			virtual bool is_start(ch_t Ch_)const {
				return TaskCh==Ch_;
			}
			// is_start�֐� : �����ꂩ���ʐM������Ԃ�
			virtual bool is_start()const {
				return TaskCh>0;
			}
		public:
			// iotask�����W�X�g���Ă���
			bool regist(ch_t Ch_, io_interface& rIO_) {
				//0Ch�͐�L
				if(Ch_==0)return true;

				//�o�^�ς݂�Ch�Ȃ�A�G���[��Ԃ�
				if(TaskMap.find(Ch_) != std::end(TaskMap))return true;

				//TaskMap�ɓo�^
				TaskMap.insert(io_pair(Ch_, rIO_));

				return false;
			}
			//���݂̃`�����l�����擾
			ch_t getCh()const { return TaskCh; }
		};
	}
}
#
#endif
