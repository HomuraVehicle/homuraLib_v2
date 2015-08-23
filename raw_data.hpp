#ifndef HMR_RAWDATA_INC
#define HMR_RAWDATA_INC 100
#
/*
=== ���� ===
�{���́A�l������K�v�̂���G���f�B�A���̈Ⴂ�𖳎����Ă���B
�ΏۂƂ��Ă���f�o�C�X�i�R���p�C���j�ԂŁA�G���f�B�A���������ł��邱�Ƃ��m�F���Ďg�����ƁB
�܂��AByteNum��sizeof�Ŏ擾����Ɗ����ƂɈႤ�ꍇ������i�A���C�����g�A�p�f�B���O���Ō����j�̂ŁAsizeof���g���Ă͂Ȃ�Ȃ��B
*/
namespace hmr {
	template<typename output_iterator,typename T>
	output_iterator write_rawdata(output_iterator itr, const T& Val, unsigned int ByteNum) {
		const unsigned char* Ptr=static_cast<const unsigned char*>(static_cast<const void*>(&Val));
		for(unsigned int Cnt=0; Cnt<ByteNum; ++Cnt) {
			(*itr++)=(*Ptr++);
		}
		return itr;
	}
	template<typename input_iterator,typename T>
	input_iterator read_rawdata(input_iterator itr, T& Val, unsigned int ByteNum) {
		unsigned char* Ptr=static_cast<unsigned char*>(static_cast<void*>(&Val));
		for(unsigned int Cnt=0; Cnt<ByteNum; ++Cnt) {
			(*Ptr++)=(*itr++);
		}
		return itr;
	}
}
#
#endif
