#include "./event_loop.h"

#include "../base/logging.h"
#include "../base/mutex.h"
#include "./channel.h"
#include "./poller.h"
#include "./sockets_ops.h"
#include "./timer_queue.h"

#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace ccnet;
using namespace ccnet::net;

namespace
{
__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;


// eventfd用于通知的文件描述符
// 创建一个eventfd的fd后，read:处理了事件、write:通知有事件发生。
// 只有8byte的缓冲字节
int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
   // LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}


EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    iteration_(0),
    threadId_(current_thread::tid()),
    poller_(Poller::newDefaultPoller(this)), // 默认适用epoll
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentActiveChannel_(NULL)
{
  //LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread)
  {
    //LOG_FATAL << "Another EventLoop " << t_loopInThisThread
          //    << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(
      std::bind(&EventLoop::handleRead, this)); // 设置wakeupChannel_通道的回调函数，纳入poller中管理
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
 // LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
      //      << " destructs in thread " << current_thread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}


// 事件循环，该函数不能跨线程调用
// 只能在创建该对象的线程中使用
void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread(); //断言在当前线程中
  looping_ = true;
  quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
  //LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_)
  {
    activeChannels_.clear(); // 清空活跃的通道
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_); // 返回活跃的通道
    ++iteration_;
    /*
    if (Logger::logLevel() <= Logger::TRACE)
    {
      printActiveChannels();
    }*/
    // TODO sort channel by priority
    eventHandling_ = true;
    for (Channel* channel : activeChannels_) // 遍历所有活跃通道
    {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);// 处理通道
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;
    doPendingFunctors(); // 处理附加回调函数, timer in
  }

  //LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}


// 该函数可以跨线程调用
void EventLoop::quit()
{
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread())
  {
    wakeup(); //如果跨线程调用，一定要唤醒，使得loop中的poller可以返回
    // poll状态可能阻塞， 如果不唤醒， loop可能一直阻塞
  }
}


//在IO函数中执行某个回调函数，该函数可以跨线程调用
void EventLoop::runInLoop(Functor cb)
{
  if (isInLoopThread())
  { // 如果是当前IO线程调用runInLoop, 则同步调用cb
    cb();
  }
  else
  { // 如果其他线程调用，异步的将cb添加到队列
    queueInLoop(std::move(cb));
  }
}


//将任务添加到队列
void EventLoop::queueInLoop(Functor cb)
{
  {
  MutexLockGuard lock(mutex_);
  pendingFunctors_.push_back(std::move(cb));
  }
  
  // 调用线程不是当前IO线程需要唤醒，让poller返回
  // 或者是当前IO线程，且正在调用pendingFunctors_
  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup(); //// 唤醒owner线程，往wakefd_写数据
  }
}

size_t EventLoop::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return pendingFunctors_.size();
}



////////////////////////////////////////////////////////////////////////////////////////

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId  timerId)
{
  return timerQueue_->cancel(timerId);
}

///////////////////////////////////////////////////////////////////////////////////////



void EventLoop::updateChannel(Channel* channel)
{ // channel的updateChannel调用，然后其再调用poller
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

// 从poller列表中移除Channel
void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_)
  {
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
  /*
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  current_thread::tid();*/
}

// 唤醒事件
void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);  // 通知有事件发生
  if (n != sizeof one)
  {
    //LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

// wakeupFd_ 事件处理
void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    //LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}


// 处理队列中的事件
void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
  MutexLockGuard lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i)
  {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
  for (const Channel* channel : activeChannels_)
  {
   // LOG_TRACE << "{" << channel->reventsToString() << "} ";
  }
}

