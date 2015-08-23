#ifndef HMR_RAWDATA_INC
#define HMR_RAWDATA_INC 100
#
/*
=== 注意 ===
本来は、考慮する必要のあるエンディアンの違いを無視している。
対象としているデバイス（コンパイラ）間で、エンディアンが同じであることを確認して使うこと。
また、ByteNumもsizeofで取得すると環境ごとに違う場合がある（アライメント、パディング等で検索）ので、sizeofを使ってはならない。
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
