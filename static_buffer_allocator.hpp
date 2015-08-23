#ifndef HMR_STATICBUFFERALLOCATOR_INC
#define HMR_STATICBUFFERALLOCATOR_INC 100
#
#include<XC32Lib/interrupt.hpp>
#include<bitset>
#include"type.hpp"
namespace hmr{
	template<typename T,unsigned int Size,unsigned int Num, typename Identifier>
	struct static_buffer_allocator{
	private:
		typedef static_buffer_allocator<void, Size, Num, Identifier> void_type;
	public:
		// 型定義
		typedef unsigned int size_type;
		typedef signed int difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef static_buffer_allocator<T,Size,Num,Identifier> my_type;
		// アロケータをU型にバインドする
		template <class U>
		struct rebind {
			typedef static_buffer_allocator<U, Size, Num, Identifier> other;
		};

		// コンストラクタ
		static_buffer_allocator() throw() {}
		static_buffer_allocator(const my_type&) throw() {}
		template <class U> static_buffer_allocator(const static_buffer_allocator<U, Size, Num, Identifier>&) throw() {}
		// デストラクタ
		~static_buffer_allocator() throw() {}

		// メモリを割り当てる
		pointer allocate(size_type num, const_pointer hint = 0) {
			return static_cast<pointer>(void_type().allocate(num, hint));
		}
		// メモリを解放する
		void deallocate(pointer p, size_type num) {
			void_type().deallocate(p,num);
		}

		// 割当て済みの領域を初期化する
		void construct(pointer p, const T& value) { new((void*)p) T(value); }
		// 初期化済みの領域を削除する
		void destroy(pointer p) { p->~T(); }

		// アドレスを返す
		pointer address(reference value) const { return &value; }
		const_pointer address(const_reference value) const { return &value; }

		// 割当てることができる最大の要素数を返す
		size_type max_size() const throw() {
			return Size / sizeof(T);
		}
		friend bool operator==(const my_type& My1_, const my_type& My2_) {
			return true;
		}
		friend bool operator!=(const my_type& My1_, const my_type& My2_) {
			return false;
		}
		
	public:
		static void* buffer(unsigned int Pos_){return void_type::buffer(Pos_);}
		static unsigned int find_buffer_pos(void* Ptr_){ return void_type::find_buffer_pos(Ptr_); }
		static void deallocate_buffer(void* Ptr){ void_type::deallocate_buffer(Ptr); }
		static vFp_v get_delete_fp(const void* Ptr){return void_type::get_delete_fp();}
	};
	template<unsigned int Size, unsigned int Num, typename Identifier>
	struct static_buffer_allocator<void, Size, Num, Identifier>{
	private:
		//アライメント問題回避用構造体
		//	二次元配列だと、N番目の配列の先頭が構造体確保不能なアドレスとなる可能性がある
		//	構造体にしてしまえば、先頭は必ず構造体を置けるメモリアドレスとなるはず
		struct buffer_holder{
			unsigned char Data[Size];
		};
	private:
		static buffer_holder Buffer[Num];
		static std::bitset<Num> IsUse;
	public:
		// 型定義
		typedef unsigned int size_type;
		typedef signed int difference_type;
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef static_buffer_allocator<void, Size, Num, Identifier> my_type;
		// アロケータをU型にバインドする
		template <class U>
		struct rebind {
			typedef static_buffer_allocator<U, Size, Num, Identifier> other;
		};

		// コンストラクタ
		static_buffer_allocator() throw() {}
		static_buffer_allocator(const my_type&) throw() {}
		template <class U> static_buffer_allocator(const static_buffer_allocator<U, Size, Num, Identifier>&) throw() {}
		// デストラクタ
		~static_buffer_allocator() throw() {}

		// メモリを割り当てる
		pointer allocate(size_type num, const_pointer hint = 0) {
			unsigned int Pos = 0;				

			//allocateが競合するとまずいので、interrupt_lock
			xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);

			for(; Pos < Size; ++Pos){
				if(!IsUse.test(Pos))break;
			}
			if(Pos == Size)return 0;
			IsUse.set(Pos);
			
			return Buffer[Pos].Data;
		}
		// メモリを解放する
		void deallocate(pointer p, size_type num) {
			//deallocateは競合しえないので、lock不要
			unsigned int Pos = 0;
			for(; Pos<Size; ++Pos){
				if(Buffer[Pos].Data == p)break;
			}
			if(Pos == Size)return;
			IsUse.reset(Pos);
		}

		// 割当てることができる最大の要素数を返す
		size_type max_size() const throw() {
			return Size;
		}
		friend bool operator==(const my_type& My1_, const my_type& My2_) {
			return true;
		}
		friend bool operator!=(const my_type& My1_, const my_type& My2_) {
			return false;
		}
		static void* buffer(unsigned int Pos_){return Buffer[Pos_].Data;}
		static unsigned int find_buffer_pos(const void* Ptr){
			for(unsigned int Pos = 0; Pos<Size; ++Pos){
				if(static_cast<const unsigned char*>(Buffer[Pos].Data) <= static_cast<const unsigned char*>(Ptr)
					&& static_cast<const unsigned char*>(Ptr) < static_cast<const unsigned char*>(Buffer[Pos].Data) + Size){
					return Pos;
				}
			}
			return Size;
		}
		static void deallocate_buffer(void* Ptr){
			my_type().deallocate(buffer(find_buffer_pos(Ptr)), 1);
		}
	private:
		template<unsigned int Pos>
		struct delete_function{
			static void exe_delete(){
				my_type().deallocate(my_type::buffer(Pos), 1);
			}
		};
		struct delete_fp_array_holder{
			static vFp_v delete_fp_array[Num];
			delete_fp_array_holder(){
				array_initializer<Num - 1>::ini();
			}
			template<unsigned int Pos>
			struct array_initializer{
				static void ini(){
					delete_fp_array[Pos] = delete_function<Pos>::exe_delete;
					if(Pos>0)array_initializer<Pos - 1>::ini();
				}
			};
		};
		static delete_fp_array_holder DeleteFpArrayHolder;
	public:
		static vFp_v get_delete_fp(const void* Ptr){
			unsigned int Pos = find_buffer_pos(Ptr);
			if(Pos >= Size) return 0;
			else return	DeleteFpArrayHolder[Pos];
		}
	};
	template<unsigned int Size,unsigned int Num,typename Identifier>
	typename static_buffer_allocator<void, Size, Num, Identifier>::buffer_holder static_buffer_allocator<void, Size, Num, Identifier>::Buffer[Num];
	template<unsigned int Size,unsigned int Num,typename Identifier>
	std::bitset<Num> static_buffer_allocator<void, Size, Num, Identifier>::IsUse;
}
#
#endif
