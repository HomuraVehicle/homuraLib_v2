#ifndef HMR_GATEADAPTOR_INC
#define HMR_GATEADAPTOR_INC 100
#
#include<hmLib/gate.hpp>
#include<homuraLib_v2/gate.hpp>
namespace hmr{
	struct homuragate_adaptor :public hmLib::gate {
	private:
		hmr::gate& rGate;
	public:
		homuragate_adaptor(hmr::gate& rGate_) :rGate(rGate_) {}
		//delete copy 
		homuragate_adaptor(const homuragate_adaptor&)=delete;
		const homuragate_adaptor& operator=(const homuragate_adaptor&)=delete;
		//move OK
		homuragate_adaptor(homuragate_adaptor&& My_):rGate(My_.rGate) {}
		const homuragate_adaptor& operator=(homuragate_adaptor&& My_) {
			if(this!=&My_) {
				rGate=My_.rGate;
			}
			return *this;
		}
	public:
		//gate���J���Ă��邩�ǂ����̊m�F
		virtual bool is_open() { return rGate.is_open(); }
		//��M�\��Ԃ��̊m�F
		virtual bool can_getc() {return rGate.can_getc();}
		//�Pbyte��M
		virtual char getc() { return static_cast<char>(rGate.getc()); }
		//��M���p�����Ă��邩���m�F����
		virtual bool flowing() {return rGate.flowing();}
		//���M�\��Ԃ��̊m�F
		virtual bool can_putc() { return rGate.can_putc(); }
		//�Pbyte���M
		virtual void putc(char c) { rGate.putc(static_cast<unsigned char>(c));}
		//���M��ł��؂�
		virtual void flush() {rGate.flush();}
	};
	struct hmLibgate_adaptor :public hmr::gate{
	private:
		hmLib::gate& rGate;
	public:
		hmLibgate_adaptor(hmLib::gate& rGate_) :rGate(rGate_) {}
	public:
		//gate���J���Ă��邩�ǂ����̊m�F
		virtual bool is_open() { return rGate.is_open(); }
		//��M�\��Ԃ��̊m�F
		virtual bool can_getc() { return rGate.can_getc(); }
		//�Pbyte��M
		virtual unsigned char getc() { return static_cast<unsigned char>(rGate.getc()); }
		//��M���p�����Ă��邩���m�F����
		virtual bool flowing() { return rGate.flowing(); }
		//���M�\��Ԃ��̊m�F
		virtual bool can_putc() { return rGate.can_putc(); }
		//�Pbyte���M
		virtual void putc(unsigned char c) { rGate.putc(static_cast<char>(c)); }
		//���M��ł��؂�
		virtual void flush() { rGate.flush(); }
	};
}
#
#endif
