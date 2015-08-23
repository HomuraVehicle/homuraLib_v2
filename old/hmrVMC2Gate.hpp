#ifndef HMR_VMC2GATE_INC
#define HMR_VMC2GATE_INC 101
#
/*---hmrVMC2Gate---
vmc2��gate�C���^�[�t�F�[�X�ł̃��b�s���O

===hmrVMC2Gate===
v1_01/140126 hmIto
	hmrVMC2�̃C���N���[�h�p�X��ύX
v1_00/131230 hmIto
	���������̂ōč쐬

*/
#include<memory>
#include<hmLib_v3_06/gate.hpp>
#include"hmrVMC2.h"
namespace hmr {
	class vmc2_gate :public hmLib::gate {
		vmc2 VMC2;
		std::unique_ptr<vcom1> pVCom1;
	public:
		vmc2_gate()=default;
		template<typename vcom1_creater>
		vmc2_gate(unsigned char Ch, vcom1_creater VCom1Creater) :pVCom1(VCom1Creater()) {
			vmc2_initialize(&VMC2, pVCom1.get(), Ch);
		}
		//vmc2_gate cannot copy
		vmc2_gate(const vmc2_gate& my_)=delete;
		const vmc2_gate& operator=(const vmc2_gate& my_)=delete;
		//vmc2_gate can move
		vmc2_gate(vmc2_gate&& my_) :VMC2(my_.VMC2) {
			std::swap(pVCom1, my_.pVCom1);
		}
		const vmc2_gate& operator=(vmc2_gate&& my_) {
			if(this!=&my_) {
				VMC2=my_.VMC2;
				std::swap(pVCom1, my_.pVCom1);
			}
			return *this;
		}
		~vmc2_gate() { if(is_open())close(); }
	public:
		template<typename vcom1_creater>
		void open(vcom1_creater VCom1Creater, unsigned char Ch) {
			hmLib_assert(!is_open(), hmLib::gate_opened_exception, "This gate have already been opened.");

			pVCom1.reset(VCom1Creater());
			vmc2_initialize(&VMC2, pVCom1.get(), Ch);
		}
		void close(void) {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");

			vmc2_finalize(&VMC2);
			pVCom1.reset(nullptr);
		}
		//gate���J���Ă��邩�ǂ����̊m�F
		bool is_open() override { return static_cast<bool>(pVCom1); }
	public://vmc2 functions
		//���M�������I�ɏI��������
		void force_end_get() {
			return vmc2_force_end_get(&VMC2);
		}
		//��M�������I�ɏI��������
		void force_end_put() {
			return vmc2_force_end_put(&VMC2);
		}
		//���M�G���[���擾����
		unsigned char error_of_get() {
			return vmc2_error_of_get(&VMC2);
		}
		//���M�G���[���N���A����
		void clear_error_of_get() {
			return vmc2_clear_error_of_get(&VMC2);
		}
		//��M�G���[���擾����
		unsigned char error_of_put() {
			return vmc2_error_of_put(&VMC2);
		}
		//��M�G���[���N���A����
		void clear_error_of_put(){
			return vmc2_clear_error_of_get(&VMC2);
		}
	public://gate interface
		//��M�\��Ԃ��̊m�F
		bool can_getc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vmc2_can_getc(&VMC2)!=0;
		}
		//�Pbyte��M
		char getc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return static_cast<char>(vmc2_getc(&VMC2));
		}
		//��M���p�����Ă��邩���m�F����
		bool flowing(){
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vmc2_flowing(&VMC2)!=0;
		}
		//���M�\��Ԃ��̊m�F
		bool can_putc() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vmc2_can_putc(&VMC2)!=0;
		}
		//�Pbyte���M
		void putc(char c) {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vmc2_putc(&VMC2,c);
		}
		//���M��ł��؂�
		void flush() {
			hmLib_assert(is_open(), hmLib::gate_not_opened_exception, "This gate have not been opened yet.");
			return vmc2_flush(&VMC2);
		}
	};
}
#
#endif
