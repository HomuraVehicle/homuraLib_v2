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
		//gateが開いているかどうかの確認
		virtual bool is_open() { return rGate.is_open(); }
		//受信可能状態かの確認
		virtual bool can_getc() {return rGate.can_getc();}
		//単byte受信
		virtual char getc() { return static_cast<char>(rGate.getc()); }
		//受信が継続しているかを確認する
		virtual bool flowing() {return rGate.flowing();}
		//送信可能状態かの確認
		virtual bool can_putc() { return rGate.can_putc(); }
		//単byte送信
		virtual void putc(char c) { rGate.putc(static_cast<unsigned char>(c));}
		//送信を打ち切る
		virtual void flush() {rGate.flush();}
	};
	struct hmLibgate_adaptor :public hmr::gate{
	private:
		hmLib::gate& rGate;
	public:
		hmLibgate_adaptor(hmLib::gate& rGate_) :rGate(rGate_) {}
	public:
		//gateが開いているかどうかの確認
		virtual bool is_open() { return rGate.is_open(); }
		//受信可能状態かの確認
		virtual bool can_getc() { return rGate.can_getc(); }
		//単byte受信
		virtual unsigned char getc() { return static_cast<unsigned char>(rGate.getc()); }
		//受信が継続しているかを確認する
		virtual bool flowing() { return rGate.flowing(); }
		//送信可能状態かの確認
		virtual bool can_putc() { return rGate.can_putc(); }
		//単byte送信
		virtual void putc(unsigned char c) { rGate.putc(static_cast<char>(c)); }
		//送信を打ち切る
		virtual void flush() { rGate.flush(); }
	};
}
#
#endif
