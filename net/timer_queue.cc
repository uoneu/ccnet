#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "./timer_queue.h"

#include "./event_loop.h"
#include "./timer.h"
#include "./timer_id.h"

#include <sys/timerfd.h>
#include <unistd.h>

namespace ccnet
{
namespace net
{
namespace detail
{

// 用于创建一个定时器文件，超时时可读
int createTimerfd()
{
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0)
  {
    //LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timerfd;
}

// 计算超时时刻与当前时间的时间差
struct timespec howMuchTimeFromNow(Timestamp when)
{
  int64_t microseconds = when.microSecondsSinceEpoch()
                         - Timestamp::now().microSecondsSinceEpoch();
  if (microseconds < 100)
  {
    microseconds = 100;
  }
  struct timespec ts;
  ts.tv_sec = static_cast<time_t>(
      microseconds / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(
      (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
  return ts;
}



// 清除定时器，避免一直触发（使用的LT模式，没有使用ET模式）
void readTimerfd(int timerfd, Timestamp now)
{
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  //LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if (n != sizeof howmany)
  {
    //LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}


//  重置定时器文件描述符
void resetTimerfd(int timerfd, Timestamp expiration)
{
  // wake up loop by timerfd_settime()
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret)
  {
    //LOG_SYSERR << "timerfd_settime()";
  }
}

}
}
}

using namespace ccnet;
using namespace ccnet::net;
using namespace ccnet::net::detail;

TimerQueue::TimerQueue(EventLoop* loop)
  : loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop, timerfd_),
    timers_(),
    callingExpiredTimers_(false)
{
  timerfdChannel_.setReadCallback(std::bind(&TimerQueue::handleRead, this)); // 设置可读的回调函数，一旦有定时器超时，即变的可读
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading(); // 加入到poller中关注
}

TimerQueue::~TimerQueue()
{
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_); // 关闭定时器文件描述符
  // do not remove channel, since we're in EventLoop::dtor();
  for (TimerList::iterator it = timers_.begin();
      it != timers_.end(); ++it)
  {
    delete it->second; // 释放所有定时器
  }
}

// 添加定时器
TimerId TimerQueue::addTimer(TimerCallback cb, Timestamp when, double interval)
{
  Timer* timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(
      std::bind(&TimerQueue::addTimerInLoop, this, timer)); // 保证可以跨线程
  return TimerId(timer, timer->sequence());
}

//取消定时器
void TimerQueue::cancel(TimerId timerId)
{
  loop_->runInLoop(
      std::bind(&TimerQueue::cancelInLoop, this, timerId));
}


// 在loop中添加定时器
void TimerQueue::addTimerInLoop(Timer* timer)
{
  loop_->assertInLoopThread();
  // 插入一个定时器，有可能使得最早到期的定时器发生改变, 比如插入的定时器超时时间比现有定时器超时时间还早
  bool earliestChanged = insert(timer); 

  if (earliestChanged)
  { // 重置定时器的超时时刻（timerfd_settime)
    resetTimerfd(timerfd_, timer->expiration());
  }
}


// 在loop中取消一个定时器
void TimerQueue::cancelInLoop(TimerId timerId)
{
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  ActiveTimerSet::iterator it = activeTimers_.find(timer); // 查找该定时器
  if (it != activeTimers_.end())
  { // 在timers_和activeTimers_中删除
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1); (void)n;
    delete it->first; // FIXME: no delete please
    activeTimers_.erase(it);
  }
  else if (callingExpiredTimers_)
  { // 已经到期，并且正在调用回调函数的定时器
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}


// 处理所有超时定时器
void TimerQueue::handleRead()
{
  loop_->assertInLoopThread();
  Timestamp now(Timestamp::now());
  readTimerfd(timerfd_, now); // 清楚定时器文件描述上的时间，避免一直触发

  std::vector<Entry> expired = getExpired(now); // 获取该时刻之前的所有的定时器列表（超时定时器列表）

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();
  // safe to callback outside critical section
  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    it->second->run(); // 这里回调定时器的
  }
  callingExpiredTimers_ = false;
  // 若果不是一次性定时器，则需要重启
  reset(expired, now);
}


// 得到所有超时的定时器
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
  TimerList::iterator end = timers_.lower_bound(sentry); // >=now，返回第一个未超时的定时器
  assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired)); // 将超时的定时器插入到expired中
  timers_.erase(timers_.begin(), end); //timers_中移除到期的定时器

  // 从activeTimers_中移除到期的定时器
  for (std::vector<Entry>::iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    ActiveTimer timer(it->second, it->second->sequence());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1); (void)n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

// 重启定时器，因为有某些定时器是重复的
void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
  Timestamp nextExpire;

  for (std::vector<Entry>::const_iterator it = expired.begin();
      it != expired.end(); ++it)
  {
    ActiveTimer timer(it->second, it->second->sequence());
    if (it->second->repeat() // 如果是重复的定时器或者未取消定时器
        && cancelingTimers_.find(timer) == cancelingTimers_.end())
    { 
      it->second->restart(now);
      insert(it->second);
    }
    else
    { // 一次性定时器或已被取消的定时器是不能重复的，因此从队列中删除该定时器
      // FIXME move to a free list
      delete it->second; // FIXME: no delete please
    }
  }

  if (!timers_.empty())
  { // 重启定时器之后，最早的超时时间可能改变
    nextExpire = timers_.begin()->second->expiration();
  }

  if (nextExpire.valid())
  {
    resetTimerfd(timerfd_, nextExpire);
  }
}



// 插入定时器
bool TimerQueue::insert(Timer* timer)
{
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  // 最早到期时间是否改变
  bool earliestChanged = false; 
  Timestamp when = timer->expiration(); //timer的超时时间戳 
  TimerList::iterator it = timers_.begin(); // 定时器队列中最早超时时间戳
  if (it == timers_.end() || when < it->first)
  {// 如果队里空 或 timer超时早于timers_中的最早到期时间
    earliestChanged = true;
  }
  { // 插入到timers_中
    std::pair<TimerList::iterator, bool> result
      = timers_.insert(Entry(when, timer)); 
    assert(result.second); (void)result;
  }
  { // 插入到activeTimers_中
    std::pair<ActiveTimerSet::iterator, bool> result
      = activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second); (void)result;
  }

  assert(timers_.size() == activeTimers_.size());
  return earliestChanged;
}

