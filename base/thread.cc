#include "./thread.h"
#include "./current_thread.h"
#include "./exception.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace ccnet
{
namespace detail
{


void afterFork()
{
  ccnet::current_thread::t_cachedTid = 0;
  ccnet::current_thread::t_threadName = "main";
  current_thread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}


// 可用于设置主线程的id
class ThreadNameInitializer
{
 public:
  ThreadNameInitializer()
  {
    ccnet::current_thread::t_threadName = "main";
    current_thread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;
// 全部变量类，这个对象构造先于main函数，当我们的程序引入这个库时，
// 这个全局函数直接构造，我们程序的main()函数还没有执行。





// 线程数据
struct ThreadData
{
  typedef ccnet::Thread::ThreadFunc ThreadFunc;
  ThreadFunc func_;
  string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(ThreadFunc func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }

  void runInThread()
  {
    *tid_ = ccnet::current_thread::tid(); // 设置线程id
    tid_ = NULL;
    latch_->countDown(); //计时结束，线程准备好， 通知Thread::start()函数可以运行
    latch_ = NULL;

    ccnet::current_thread::t_threadName = name_.empty() ? "ccnetThread" : name_.c_str();
    ::prctl(PR_SET_NAME, ccnet::current_thread::t_threadName);
    try
    {
      func_(); // 回调函数， 线程运行函数
      ccnet::current_thread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      ccnet::current_thread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      ccnet::current_thread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      ccnet::current_thread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
}; // ThreadData



void* startThread(void* obj) // 被pthread_create调用
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread(); 
  delete data;
  return NULL;
}


} // detail
} // ccnet



using namespace ccnet;


AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc &func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n),
    latch_(1)
{
  setDefaultName();
  printf("===\n");
}


Thread::Thread(ThreadFunc &&func, const string &n) 
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(func),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_); // 如果没有join， 设置线程为分离状态，运行结束自动释放资源
  }
}

void Thread::setDefaultName() // 设置线程默认名称（当前线程的数量）
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data)) // 创建线程
  {
    started_ = false;
    delete data; // or no delete?
    //LOG_SYSFATAL << "Failed in pthread_create";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL); // 成功返回0，否则返回错误编号
}

