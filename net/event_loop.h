#ifndef CCNET_NET_EVENTLOOP_H
#define CCNET_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "../base/mutex.h"
#include "../base/current_thread.h"
#include "../base/timestamp.h"
#include "./callbacks.h"
#include "./timer_id.h"


namespace ccnet
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

/// 事件分发器,负责IO和定时事件的分派
/// 事件循环(反应堆Reactor)
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop : noncopyable
{
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop(); // 类似于统一事件源, 所有事件处理都在这里进行，包括定时器

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor cb); // 添加任务到IO线程，异步调用。如果在其他线程中添加，则添加到队列，在loop中统一处理
  // 保证可以跨线程调用
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor cb); // 添加任务到队列

  size_t queueSize() const;




  //////////////////////////////////////////////////////////////////////////////////////////
  // timers 有关定时器的几个接口
  TimerId runAt(Timestamp time, TimerCallback cb); // 在某个时刻调用 TimerCallback
  TimerId runAfter(double delay, TimerCallback cb); // 过一段时间调用 TimerCallback
  TimerId runEvery(double interval, TimerCallback cb); // 每隔一段时间调用 TimerCallback， 间隔型的定时器
  void cancel(TimerId timerId); // cancel 取消 timer
  ////////////////////////////////////////////////////////////////////////////////////////

  // internal usage
  void wakeup();
  void updateChannel(Channel* channel); // 在poller中添加或者更新通道,
  void removeChannel(Channel* channel); // 从poller中移除通道
  bool hasChannel(Channel* channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }
  bool isInLoopThread() const { return threadId_ == current_thread::tid(); }  // 遵循 one loop per thread
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  static EventLoop* getEventLoopOfCurrentThread();





 private:
  void abortNotInLoopThread();
  void handleRead();  // waked up
  void doPendingFunctors();

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel*> ChannelList;

  bool looping_; /* atomic 是否处于循环状态*/
  std::atomic<bool> quit_; // 是否退出loop
  bool eventHandling_; /* atomic 是否处于事件处理状态*/
  bool callingPendingFunctors_; /* atomic */
  int64_t iteration_;
  const pid_t threadId_;  // 当前对象所属线程id
  Timestamp pollReturnTime_; // 调用poll返回的时间 
  std::unique_ptr<Poller> poller_; // 决定调用poll/epoll，多态
  std::unique_ptr<TimerQueue> timerQueue_;  // 定时器队列
  int wakeupFd_;   // 用于eventfd
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeupChannel_; // 用于wakeupFd_的通道
  boost::any context_;

  // scratch variables
  ChannelList activeChannels_; // poller返回的活动通道列表
  Channel* currentActiveChannel_; // 当前正在处理的活动通道

  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_; // 异步线程添加的事件，类似于统一事件源，这里用于eventfd，不用socketpair
};

}
}
#endif  // CCNET_NET_EVENTLOOP_H
