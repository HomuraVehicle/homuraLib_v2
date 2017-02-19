#ifndef HMR_MESSAGE_IOREFERENCE_INC
#define HMR_MESSAGE_IOREFERENCE_INC 100
#
/*=== message::io_reference ===
input_interface, output_interface, io_interfaceあるいはinput_interface+output_interfaceを、
あたかも同一にio_interfaceとして扱えるようになる、ラッパークラス

=== io_reference ===
v1_00/140412 hmIto
	リリース
*/
#include"io_interface.hpp"
namespace hmr {
	namespace message {
		class io_reference :public io_interface{
			input_interface* pInput;
			output_interface* pOutput;
		public:
			io_reference() :pInput(0), pOutput(0) {}
			io_reference(input_interface& rInput_) :pInput(&rInput_), pOutput(0) {}
			io_reference(output_interface& rOutput_) :pInput(0), pOutput(&rOutput_) {}
			io_reference(input_interface& rInput_, output_interface& rOutput_) :pInput(&rInput_), pOutput(&rOutput_) {}
			io_reference(io_interface& rIO_) :pInput(&rIO_), pOutput(&rIO_) {}
		protected:
			virtual bool io_can_put() {
				if(!pInput)return true;
				return pInput->can_put();
			}
			virtual bool io_put(const data& rData_) {
				if(!pInput)return false;
				return pInput->put(rData_);
			}
			virtual void io_flush() {
				if(!pInput)return;
				pInput->flush();
			}
		protected:
			virtual bool io_can_get() {
				if(!pOutput)return false;
				return pOutput->can_get();
			}
			virtual bool io_get(data& rData_) {
				if(!pOutput)return false;
				return pOutput->get(rData_);
			}
			virtual bool io_flowing() {
				if(!pOutput)return false;
				return pOutput->flowing();
			}
		public:
			bool isInput()const { return pInput!=0; }
			bool isOutput()const { return pOutput!=0; }
		};
	}
}
#
#endif
