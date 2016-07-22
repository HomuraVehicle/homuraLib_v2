#ifndef HMR_STATICBUFFERALLOCATOR_INC
#define HMR_STATICBUFFERALLOCATOR_INC 100
#
#include<XC32/interrupt.hpp>
#include<bitset>
#include"type.hpp"
namespace hmr{
	template<typename T,unsigned int Size,unsigned int BufNum, typename Identifier>
	struct static_buffer_allocator{
	private:
		typedef static_buffer_allocator<void, Size, BufNum, Identifier> void_type;
	public:
		// 型定義
		typedef unsigned int size_type;
		typedef signed int difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef static_buffer_allocator<T,Size,BufNum,Identifier> my_type;
		// アロケータをU型にバインドする
		template <class U>
		struct rebind {
			typedef static_buffer_allocator<U, Size, BufNum, Identifier> other;
		};

		// コンストラクタ
		static_buffer_allocator() throw() {}
		static_buffer_allocator(const my_type&) throw() {}
		template <class U> static_buffer_allocator(const static_buffer_allocator<U, Size, BufNum, Identifier>&) throw() {}
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
	public:
		static size_type allocated_num(){ return void_type::allocated_num(); }
		static bool allocated_full(){ return void_type::allocated_full(); }
	};
	template<unsigned int Size, unsigned int BufNum, typename Identifier>
	struct static_buffer_allocator<void, Size, BufNum, Identifier>{
	private:
		//アライメント問題回避用構造体
		//	二次元配列だと、N番目の配列の先頭が構造体確保不能なアドレスとなる可能性がある
		//	構造体にしてしまえば、先頭は必ず構造体を置けるメモリアドレスとなるはず
		struct buffer_holder{
			unsigned char Data[Size];
		};
	private:
		static buffer_holder Buffer[BufNum];
		static std::bitset<BufNum> IsUse;
	public:
		// 型定義
		typedef unsigned int size_type;
		typedef signed int difference_type;
		typedef void* pointer;
		typedef const void* const_pointer;
		typedef static_buffer_allocator<void, Size, BufNum, Identifier> my_type;
		// アロケータをU型にバインドする
		template <class U>
		struct rebind {
			typedef static_buffer_allocator<U, Size, BufNum, Identifier> other;
		};

		// コンストラクタ
		static_buffer_allocator() throw() {}
		static_buffer_allocator(const my_type&) throw() {}
		template <class U> static_buffer_allocator(const static_buffer_allocator<U, Size, BufNum, Identifier>&) throw() {}
		// デストラクタ
		~static_buffer_allocator() throw() {}

		// メモリを割り当てる
		pointer allocate(size_type num, const_pointer hint = 0) {
			unsigned int BufPos = 0;				

			//allocateが競合するとまずいので、interrupt_lock
			xc32::interrupt::lock_guard Lock(xc32::interrupt::Mutex);

			for(; BufPos < BufNum; ++BufPos){
				if(!IsUse.test(BufPos))break;
			}
			if(BufPos == BufNum)return 0;
			IsUse.set(BufPos);
			
			return Buffer[BufPos].Data;
		}
		// メモリを解放する
		void deallocate(pointer p, size_type num) {
			//deallocateは競合しえないので、lock不要
			unsigned int BufPos = 0;
			for(; BufPos<BufNum; ++BufPos){
				if(Buffer[BufPos].Data == p)break;
			}
			if(BufPos == BufNum)return;
			IsUse.reset(BufPos);
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
		static void* buffer(unsigned int BufPos_){return Buffer[BufPos_].Data;}
		static unsigned int find_buffer_pos(const void* Ptr){
			for(unsigned int BufPos = 0; BufPos<BufNum; ++BufPos){
				const unsigned char* BufAddress = static_cast<const unsigned char*>(buffer(BufPos));
				if(BufAddress <= static_cast<const unsigned char*>(Ptr)
					&& static_cast<const unsigned char*>(Ptr) < BufAddress + Size){
					return BufPos;
				}
			}
			return Size;
		}
		static void deallocate_buffer(void* Ptr){
			my_type().deallocate(buffer(find_buffer_pos(Ptr)), 1);
		}
	private:
		template<unsigned int BufPos>
		struct delete_function{
			static void exe_delete(){
				my_type().deallocate(my_type::buffer(BufPos), 1);
			}
		};
		struct delete_fp_array_holder{
			static vFp_v delete_fp_array[BufNum];
			delete_fp_array_holder(){
				array_initializer<BufNum - 1>::ini();
			}
			template<unsigned int BufPos>
			struct array_initializer{
				static void ini(){
					delete_fp_array[BufPos] = delete_function<BufPos>::exe_delete;
					if(BufPos>0)array_initializer<BufPos - 1>::ini();
				}
			};
		};
		static delete_fp_array_holder DeleteFpArrayHolder;
	public:
		static vFp_v get_delete_fp(const void* Ptr){
			unsigned int BufPos = find_buffer_pos(Ptr);
			if(BufPos >= BufNum) return 0;
			else return	DeleteFpArrayHolder[BufPos];
		}
	public:
		static size_type allocated_num()throw(){ return IsUse.count(); }
		static bool allocated_full()throw(){ return IsUse.all(); }
	};
	template<unsigned int Size,unsigned int BufNum,typename Identifier>
	typename static_buffer_allocator<void, Size, BufNum, Identifier>::buffer_holder static_buffer_allocator<void, Size, BufNum, Identifier>::Buffer[BufNum];
	template<unsigned int Size,unsigned int BufNum,typename Identifier>
	std::bitset<BufNum> static_buffer_allocator<void, Size, BufNum, Identifier>::IsUse;
}
#
#endif
