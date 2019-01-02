#ifndef CCNET_BASE_MUTEX_H
#define CCNET_BASE_MUTEX_H


#include <assert.h>
#include <pthread.h>

#include "./current_thread.h"
#include "./noncopyable.h"


#define MCHECK(ret) ({ decltype(ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

namespace ccnet
{
// 互斥锁 的封装
// 进行了两个类的封装，在实际的使用中更常使用MutexLockGuard类，
//     因为该类可以在析构函数中自动解锁，避免了某些情况忘记解锁
// MutexLockGuard使用RAII, 构造时申请资源，析构时释放资源



// Use as data member of a class, eg.
//
// class Foo
// {
//  public:
//   int size() const;
//
//  private:
//   mutable MutexLock mutex_;
//   std::vector<int> data_; // GUARDED BY mutex_
// };
class MutexLock : noncopyable
{
 public:
  MutexLock()
    : holder_(0)
  {
    MCHECK(pthread_mutex_init(&mutex_, NULL));
  }

  ~MutexLock()
  {
    assert(holder_ == 0); // 断言该锁没有被任何线程占用，才可以销毁
    MCHECK(pthread_mutex_destroy(&mutex_));
  }

 
  bool isLockedByThisThread() const  // 是否当前线程拥有该锁
  {
    return holder_ == current_thread::tid();
  }


  void assertLocked() const // 断言当前线程拥有该锁
  {
    assert(isLockedByThisThread());
  }

  // internal usage

  void lock()  //加锁
  {
    //assert(holder_ == 0);  // 断言时空锁
    MCHECK(pthread_mutex_lock(&mutex_));
    assignHolder();
  }

  void unlock() //解锁
  {
    unassignHolder();
    MCHECK(pthread_mutex_unlock(&mutex_));
  }

  pthread_mutex_t* getPthreadMutex() // 获取threadMutex对象
  {
    return &mutex_;
  }

 private:
  friend class Condition;

  class UnassignGuard : noncopyable
  {
   public:
    UnassignGuard(MutexLock& owner)
      : owner_(owner)
    {
      owner_.unassignHolder();
    }

    ~UnassignGuard()
    {
      owner_.assignHolder();
    }

   private:
    MutexLock& owner_;
  };

  void unassignHolder()
  {
    holder_ = 0;
  }

  void assignHolder()
  {
    holder_ = current_thread::tid();
  }

  pthread_mutex_t mutex_;
  pid_t holder_;  //占用锁的线程id

};






// Use as a stack variable, eg.
// int Foo::size() const
// {
//   MutexLockGuard lock(mutex_);
//   return data_.size();
// }
class MutexLockGuard : noncopyable
{
 public:
  explicit MutexLockGuard(MutexLock& mutex)
    : mutex_(mutex)
  {
    mutex_.lock();
  }

  ~MutexLockGuard()
  {
    mutex_.unlock();
  }

 private:

  MutexLock& mutex_;
};

}

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

#endif  // CCNET_BASE_MUTEX_H
