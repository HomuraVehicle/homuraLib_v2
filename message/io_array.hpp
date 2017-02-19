#ifndef HMR_MESSAGE_IOARRAY_INC
#define HMR_MESSAGE_IOARRAY_INC 100
#
/*
io_interfaceの配列を持ち、IDによってアクセス可能なmapチックな配列

=== message::io_reference_array ===
v1_00/140412 hmIto
	微修正を終了　とりあえず、完成
*/
#include<vector>
#include<memory>
#include<algorithm>
#include<homuraLib_config/allocator.hpp>
#include"io_reference.hpp"
namespace hmr {
	namespace message {
		class io_array {
			typedef std::pair<data::id_type, io_interface*> io_element;
			typedef std::vector<io_element, allocator> containor;
			struct compare {
				bool operator()(const io_element& val1_, const io_element& val2_) const {
					if(val2_.first==data_id::null)return false;
					else if(val1_.first==data_id::null)return true;
					else return val1_.first<val2_.first;
				}
			};
		public:
			typedef io_element pair;
			struct iterator {
			public:
				typedef containor::iterator containor_iterator;
				typedef io_interface* pointer;
				typedef io_interface& reference;
				typedef const io_interface* const_pointer;
				typedef const io_interface& const_reference;
			private:
				containor_iterator Itr;
			public:
				iterator() {}
				iterator(containor_iterator CItr_) :Itr(CItr_) {}
			public:
				iterator& operator++() {
					++Itr;
					return *this;
				}
				iterator& operator--() {
					--Itr;
					return *this;
				}
				iterator operator++(int) {
					iterator ans=*this;
					++(*this);
					return ans;
				}
				iterator operator--(int) {
					iterator ans=*this;
					--(*this);
					return ans;
				}
				friend bool operator==(const iterator& itr1_, const iterator& itr2_){
					return itr1_.Itr==itr2_.Itr;
				}
				friend bool operator!=(const iterator& itr1_, const iterator& itr2_){
					return itr1_.Itr!=itr2_.Itr;
				}
				reference operator*() {
					return *(Itr->second);
				}
				pointer operator->() {
					return Itr->second;
				}
				const_reference operator*()const {
					return *(Itr->second);
				}
				const_pointer operator->()const {
					return Itr->second;
				}
				containor_iterator raw_iterator()const { return Itr; }
			};
			struct const_iterator {
			public:
				typedef containor::const_iterator containor_iterator;
				typedef const io_interface* const_pointer;
				typedef const io_interface& const_reference;
			private:
				containor_iterator Itr;
			public:
				const_iterator() {}
				const_iterator(containor_iterator CItr_) :Itr(CItr_) {}
				const_iterator(iterator Itr_) :Itr(Itr_.raw_iterator()) {}
			public:
				const_iterator& operator++() {
					++Itr;
					return *this;
				}
				const_iterator& operator--() {
					--Itr;
					return *this;
				}
				const_iterator operator++(int) {
					const_iterator ans=*this;
					++(*this);
					return ans;
				}
				const_iterator operator--(int) {
					const_iterator ans=*this;
					--(*this);
					return ans;
				}
				friend bool operator==(const const_iterator& itr1_, const const_iterator& itr2_) {
					return itr1_.Itr==itr2_.Itr;
				}
				friend bool operator!=(const const_iterator& itr1_, const const_iterator& itr2_) {
					return itr1_.Itr!=itr2_.Itr;
				}
				const_reference operator*()const {
					return *(Itr->second);
				}
				const_pointer operator->()const {
					return Itr->second;
				}
				containor_iterator raw_iterator()const { return Itr; }
			};
			struct precursor {
				friend class io_array;
			private:
				containor& rVec;
			public:
				precursor(containor& rVec_) :rVec(rVec_) {}
			};
			struct builder {
			private:
				containor Vec;
			public:
				void regist(io_interface& Ref_, data::id_type ID_) {
					Vec.push_back(io_element(ID_, &Ref_));
				}
				void reserve(unsigned int Size_) {Vec.reserve(Size_);}
				precursor operator()(void) {
					return precursor(Vec);
				}
			};
		private:
			containor Vec;
		public:
			io_array() {}
			io_array(precursor Precursor_){
				Vec.swap(Precursor_.rVec);
			}
			const io_array& operator=(precursor Precursor_) {
				Vec.swap(Precursor_.rVec);
				return *this;
			}
		private:
			io_array(const io_array&);
			const io_array& operator=(const io_array&);
		public:
			iterator begin() { 
				return iterator(Vec.begin());
			}
			iterator end() {
				return iterator(Vec.end());
			}
			const_iterator begin()const{
				return const_iterator(Vec.begin());
			}
			const_iterator end()const{
				return const_iterator(Vec.end());
			}
			io_interface& operator[](unsigned int Pos_){
				return *(Vec.at(Pos_).second);
			}
			const io_interface& operator[](unsigned int Pos_)const {
				return *(Vec.at(Pos_).second);
			}
			unsigned int size() { return Vec.size(); }
			iterator find(data::id_type ID_) {
				iterator ans=std::lower_bound(Vec.begin(), Vec.end(), io_element(ID_,0), compare());
				if(ans.raw_iterator()->first==ID_)return ans;
				else return end();
			}
			const_iterator find(data::id_type ID_)const{
				const_iterator ans=std::lower_bound(Vec.begin(), Vec.end(), io_element(ID_,0), compare());
				if(ans.raw_iterator()->first==ID_)return ans;
				else return end();
			}
		};
	}
}
#
#endif
