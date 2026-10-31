#ifndef CHATGPT_ASYNC_LOOKUP_H
#define CHATGPT_ASYNC_LOOKUP_H
#include <atomic>
/* Persistent single-host lookup storage: a late lwIP callback never points at
 * a worker's stack. A cancelled worker leaves at most one lookup pending; the
 * next worker can wait on it instead of allocating another callback context. */
template<class Address> class AsyncLookup {
 std::atomic<unsigned> state{0};Address address{};bool found=false;
 public:
 bool begin(){
  unsigned current=state.load(std::memory_order_acquire);
  while(current!=1){if(state.compare_exchange_weak(current,1,std::memory_order_acq_rel))return true;}
  return false;
 }
 void complete(const Address *result){
  found=result!=nullptr;if(result)address=*result;
  state.store(2,std::memory_order_release);
 }
 bool ready()const{return state.load(std::memory_order_acquire)==2;}
 bool result(Address &out)const{if(!ready()||!found)return false;out=address;return true;}
};
#endif
