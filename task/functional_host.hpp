#ifndef HMR_TASK_FUNCTIONALHOST_INC
#define HMR_TASK_FUNCTIONALHOST_INC 100
#
#include<algorithm>
#include<vector>
#include<hmLib_v3_06/type.h>
#include<homuraLib_v2/allocator.hpp>
#include"client_interface.hpp"
#include"host_interface.hpp"
namespace hmr{
	namespace task {
		template<typename Alloc=default_allocator>
		class functional_host:public host_interface {
		private:
			struct client_holder {
			public:
				client_interface* pClient;
				duration Interval;
				duration Count;
			public:
				client_holder(client_interface& Client_, duration Interval_=0, duration Count_=0) 
					: pClient(&Client_)
					, Interval(Interval_)
					, Count(Count_) {
				}
				client_holder(const client_holder& my_)
					: pClient(my_.pClient)
					, Interval(my_.Interval)
					, Count(my_.Count) {
				}
				const client_holder& operator=(const client_holder& my_) {
					if(this!=&my_) {
						pClient=my_.pClient;
						Interval=my_.Interval;
						Count=my_.Count;
					}
					return *this;
				}
				friend bool operator==(const client_holder& my1_, const client_holder& my2_) {
					return my1_.pClient==my2_.pClient;
				}
			};
			struct is_ref_stop {
				bool operator()(client_holder& Client_) {return Client_.Interval<0;}
			};
		public:
			typedef std::vector<client_holder, Alloc> container;
			typedef typename container::iterator iterator;
			typedef typename container::const_iterator const_iterator;
		private:
			container Tasks;
		public://host_interface
			bool start(client_interface& Client_, duration Interval_, duration Count_) {
				if(is_start(Client_))return true;
				Tasks.push_back(client_holder(Client_,Interval_,Count_));
				return false;
			}
			void quick_start(client_interface& Client_, duration Interval_, duration Count_) {
				Tasks.push_back(client_holder(Client_,Interval_,Count_));
			}
			bool is_start(client_interface& Client_)const {
				const_iterator itr=std::find(Tasks.begin(),Tasks.end(),client_holder(Client_));
				return (itr!=Tasks.end());
			}
			bool restart(client_interface& Client_, duration Interval_, duration Count_=0) {
				iterator itr=std::find(Tasks.begin(),Tasks.end(),client_holder(Client_));
				if(itr==Tasks.end())return true;

				itr->Interval=Interval_;
				itr->Count=Count_;
				return false;
			}
			bool stop(client_interface& Client_) {
				iterator itr=std::find(Tasks.begin(),Tasks.end(),client_holder(Client_));
				if(itr==Tasks.end())return true;

				Tasks.erase(itr);
				return false;
			}
		public:
			void reserve(unsigned int ReservedSize_){ Tasks.reserve(ReservedSize_); }
			unsigned int size()const { return Tasks.size(); }
			bool empty()const { return Tasks.empty(); }
			void operator()(duration Interval_) {
				//タスクバッファを順に確認する
				for(iterator itr=Tasks.begin(); itr != Tasks.end(); ++itr) {
					//前回のカウンターと今回のカウンターの間にIntervalを跨いでいれば、実行
					if(itr->Interval > 0){
						//カウンター加算
						itr->Count += Interval_;

						//カウンターがオーバーフローした場合
						if(itr->Count >= itr->Interval){
							//カウンタから除算
							itr->Count %= itr->Interval;

							//タスクを実行
							itr->Interval = (*(itr->pClient))(itr->Interval);
						}
					}
				}

				//終了要求があったタスクをはじく
				Tasks.erase(std::remove_if(Tasks.begin(), Tasks.end(), is_ref_stop()) , Tasks.end());
			}
		};
	}
}
#
#endif
