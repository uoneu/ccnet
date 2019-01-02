#ifndef CCNET_NET_TIMER_H
#define CCNET_NET_TIMER_H

#include "../base/atomic.h"
#include "../base/timestamp.h"
#include "./callbacks.h"

namespace ccnet
{
namespace net
{
/// 定时器类
/// Timer封装了定时器的一些参数，例如超时回调函数、超时时间、定时器是否重复、重复间隔时间、定时器的序列号。
/// Internal class for timer event.
///
class Timer : noncopyable
{
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(s_numCreated_.incrementAndGet())
  { }

  void run() const // 运行定时器的回调回调函数
  {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; } // 定时器编号

  void restart(Timestamp now);

  static int64_t numCreated() { return s_numCreated_.get(); }

 private:
  const TimerCallback callback_; // 回调函数
  Timestamp expiration_; // 超时时间戳
  const double interval_; // 间隔多久重新闹铃
  const bool repeat_; // 是否重复
  const int64_t sequence_; // 定时器序号

  static AtomicInt64 s_numCreated_; // 定时器计数，原子的、static，可保证唯一性
};
}
}
#endif  // CCNET_NET_TIMER_H
