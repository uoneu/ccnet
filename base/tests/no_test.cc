#include <iostream>

#include "../copyable.h"
#include "../noncopyable.h"
#include "../types.h"
#include "../bounded_blocking_queue.h"
#include "../threadlocal.h"
#include "../threadlocal_singleton.h"
#include "../mutex.h"

using namespace ccnet;
using namespace std;

template<typename T>
class Sgl {
public:
  static T* getInstance(){
    if (value_ == nullptr) {
      mutex_.lock();
      if (value_ == nullptr)
        value_ = new T();
      mutex_.unlock();
    }

    return value_;
  }

 private:
  Sgl()=default;
  ~Sgl(); 

  static T *value_;
  static MutexLock mutex_;
  
};

template<typename T>
T* Sgl<T>::value_ = nullptr;

template<typename T>
MutexLock Sgl<T>::mutex_;




int main(int argc, char *argv[]) {

    std::cout << "=======\n";
    BoundedBlockingQueue<int> bq(10);
    bq.put(8);
    cout << bq.take();

    int *p = Sgl<int>::getInstance();
    *p = 99;
    cout << *p;


    return 0;
}
