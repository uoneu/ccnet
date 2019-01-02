#ifndef CCNET_BASE_THREADLOCAL_H
#define CCNET_BASE_THREADLOCAL_H

#include "./mutex.h"  // MCHECK
#include "./noncopyable.h"

#include <pthread.h>

namespace ccnet
{

///
/// 线程私有数据
/// ThreadLocal<int> p
/// ThreadLocal<class A> pa;
template<typename T>
class ThreadLocal : noncopyable
{
 public:
  ThreadLocal()
  {
    MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor)); // 线程退出时，调用析构函数
  }

  ~ThreadLocal()
  {
    MCHECK(pthread_key_delete(pkey_));
  }

  T& value()
  {
    T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
    if (!perThreadValue)
    {
      T* newObj = new T();
      MCHECK(pthread_setspecific(pkey_, newObj));
      perThreadValue = newObj;
    }
    return *perThreadValue;
  }

 private:

  static void destructor(void *x)
  {
    T* obj = static_cast<T*>(x);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy; (void) dummy;
    delete obj;
  }

 private:
  pthread_key_t pkey_;
};

}
#endif // CCNET_BASE_THREADLOCAL_H


