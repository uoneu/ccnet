#include "../threadlocal.h"
#include "../current_thread.h"
#include "../thread.h"

#include <stdio.h>

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

  const ccnet::string& name() const { return name_; }
  void setName(const ccnet::string& n) { name_ = n; }

 private:
  ccnet::string name_;
}; // class test


ccnet::ThreadLocal<Test> testObj1; // 
ccnet::ThreadLocal<Test> testObj2;



void print()
{
  printf("tid=%d, obj1 %p name=%s\n",
         ccnet::current_thread::tid(),
         &testObj1.value(),
         testObj1.value().name().c_str());
  printf("tid=%d, obj2 %p name=%s\n",
         ccnet::current_thread::tid(),
         &testObj2.value(),
         testObj2.value().name().c_str());
}

void threadFunc()
{
  print();
  testObj1.value().setName("changed 1");
  testObj2.value().setName("changed 42");
  print();
}



int main()
{
  testObj1.value().setName("main one");
  print();
  
  printf("-----------\n\n");
  ccnet::Thread t1(threadFunc);
  t1.start();
  t1.join();

  printf("-----------\n\n");
  testObj2.value().setName("main two");
  print();

  pthread_exit(0);
}
