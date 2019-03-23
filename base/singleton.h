#ifndef CCNET_BASE_SINGLETON_H
#define CCNET_BASE_SINGLETON_H

#include "./noncopyable.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h> // atexit

namespace ccnet
{
/// 线程安全的Singleton
///
/// 在多线程环境中，有些事仅需要执行一次。通常当初始化应用程序时，可以比较容易地将其放在main函数中。
/// 但当你写一个库时，就不能在main里面初始化了，你可以用静态初始化，但使用一次初始化（pthread_once）会比较容易些。


namespace detail
{
////不能侦测继承的成员函数  
// This doesn't detect inherited member functions!
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template<typename T>
struct has_no_destroy
{
  // 萃取的又一个新技术，SFINAE技术，即匹配失败不是错误
  template<typename C> static char test(decltype(&C::no_destroy)); 
  template<typename C> static int32_t test(...);
  const static bool value = sizeof(test<T>(0)) == 1; // sizeof 可求函数返回类型大小
};
}



template<typename T>
class Singleton : noncopyable
{
 public:
  static T& instance()
  { 
   // 第一次调用会在init函数内部创建，pthread_once保证该函数只被调用一次！！！！
   // 并且pthread_once()能保证线程安全，效率高于mutex  
    pthread_once(&ponce_, &Singleton::init);  
    assert(value_ != NULL);
    return *value_;
  }

 private:
  Singleton();
  ~Singleton();

  static void init()
  {
    value_ = new T();
    if (!detail::has_no_destroy<T>::value)  //保证支持销毁方法才会注册atexit  
    {
      ::atexit(destroy); // 注册终止函数
    }
  }

  static void destroy() //程序结束后自动调用该函数销毁  
  {
    // 用typedef定义了一个数组类型，数组的大小不能为-1，利用这个方法，如果是不完全类型，编译阶段就会发现错误
    // 要销毁这个类型，这个类型必须是完全类型
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy; (void) dummy;

    delete value_;
    value_ = NULL;
  }

 private:
  static pthread_once_t ponce_;
  static T*             value_;
};


// 静态成员初始化
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = NULL;

}




#endif //CCNET_BASE_SINGLETON_H

