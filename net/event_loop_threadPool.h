// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef CCNET_NET_EVENTLOOPTHREADPOOL_H
#define CCNET_NET_EVENTLOOPTHREADPOOL_H

#include "../base/noncopyable.h"
#include "../base/types.h"

#include <functional>
#include <memory>
#include <vector>

namespace ccnet
{
/// 
/// EventLoop线程池
/// 轮询的方式使用其中的线程
///

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop* getNextLoop(); // 取下一个Loop线程，采用轮询的方式使用pool中的线程

  /// with the same hash code, it will always return the same EventLoop
  EventLoop* getLoopForHash(size_t hashCode);

  std::vector<EventLoop*> getAllLoops(); // 得到所有的EventLoop

  bool started() const // EventLoop线程池是否启动
  { return started_; }

  const string& name() const // EventLoop_pool的名称
  { return name_; }

 private:

  EventLoop* baseLoop_; // 线程池所属的loop
  string name_; // 池名称
  bool started_; // 是否启动
  int numThreads_; // 线程的数量
  int next_; // 新连接到来，所选择的EventLoop对象下标
  std::vector<std::unique_ptr<EventLoopThread>> threads_; // 智能指针会自动释放对象
  std::vector<EventLoop*> loops_;
};

}
}

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
