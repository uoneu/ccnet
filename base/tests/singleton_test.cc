#include "../singleton.h"
#include "../current_thread.h"
#include "../noncopyable.h"
#include "../thread.h"

#include <stdio.h>
#include <string>
#include <iostream>

class Test : ccnet::noncopyable
{
 public:
  Test()
  {
    printf("tid=%d, constructing %p\n", ccnet::current_thread::tid(), this);
  }

  ~Test()
  {
    printf("tid=%d, destructing %p %s\n", ccnet::current_thread::tid(), this, name_.c_str());
  }

  const std::string& name() const { return name_; }
  void setName(const std::string& n) { name_ = n; }

 private:
  std::string name_;
};


class TestNoDestroy : ccnet::noncopyable
{
 public:
  // Tag member for Singleton<T>
  void no_destroy();

  TestNoDestroy()
  {
    printf("tid=%d, constructing TestNoDestroy %p\n", ccnet::current_thread::tid(), this);
  }

  ~TestNoDestroy()
  {
    printf("tid=%d, destructing TestNoDestroy %p\n", ccnet::current_thread::tid(), this);
  }
};



void threadFunc()
{
  printf("tid=%d, %p name=%s\n",
         ccnet::current_thread::tid(),
         &ccnet::Singleton<Test>::instance(),
         ccnet::Singleton<Test>::instance().name().c_str());
  ccnet::Singleton<Test>::instance().setName("only one, changed");
}

int main()
{

  ccnet::Singleton<Test>::instance().setName("only one");
  ccnet::Thread t1(threadFunc);
  t1.start();
  t1.join();
  ccnet::mysleep(2);
  printf("====\n");
  printf("tid=%d, %p name=%s\n",
         ccnet::current_thread::tid(),
         &ccnet::Singleton<Test>::instance(),
         ccnet::Singleton<Test>::instance().name().c_str());
  
  ccnet::Singleton<TestNoDestroy>::instance();
  printf("with valgrind, you should see %zd-byte memory leak.\n", sizeof(TestNoDestroy));


  return 0;
}

