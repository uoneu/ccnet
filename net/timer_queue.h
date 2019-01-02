
#ifndef CCNET_NET_TIMERQUEUE_H
#define CCNET_NET_TIMERQUEUE_H

#include <set>
#include <vector>



#include "../base/mutex.h"
#include "../base/timestamp.h"
#include "./callbacks.h"
#include "./channel.h"

namespace ccnet
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// 定时器队列（定时器的管理器）， 只有两个入口【添加、删除】
/// 基于set（红黑树），按时间戳排序，方便插入删除响应
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop); //  定时器队列构造函数， 所属事件循环
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.
  

  // 添加定时器， cb定时回调函数，闹钟，是否重复
  // 可以跨线程调用，线程安全的
  TimerId addTimer(TimerCallback cb, Timestamp when, double interval); 

  void cancel(TimerId timerId); // 删除定时器




 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer*> Entry; // 定时器入口，first是死亡事件，set按照这个时间排序
  typedef std::set<Entry> TimerList; // 定时器队列
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;

  // 以下成员函数只能在其所属的IO线程中调用，因而不必要加锁 (统一在loop线程中调用）
  // 服务器性能杀手之一是锁竞争，所以尽可能减少锁的使用
  void addTimerInLoop(Timer* timer);  // 把定时器添加到某个循环中
  void cancelInLoop(TimerId timerId); // 在loop中删除某定时器
  // called when timerfd alarms
  void handleRead(); // 处理定时器可读
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now); // 返回超时的定时器列表
  void reset(const std::vector<Entry>& expired, Timestamp now); // 对超时的定时器重置，可能有重复定时器
  		
  bool insert(Timer* timer); // 插入定时器

  EventLoop* loop_; // 所属EventLoop
  const int timerfd_; // 定时器文件描述符
  Channel timerfdChannel_; // 定时器通道
  // Timer list sorted by expiration
  TimerList timers_; // timers_ 是按到期时间排序

  // for cancel()
  // activeTimers_和timers_保存的相同的数据
  // activeTimers_按对象地址排序、timers_按到期时间排序
  ActiveTimerSet activeTimers_; 
  bool callingExpiredTimers_; /* atomic， 是否处于调用超时定时器当中 */ 
  ActiveTimerSet cancelingTimers_; // 保存被取消的定时器
};

}
}
#endif  // CCNET_NET_TIMERQUEUE_H
