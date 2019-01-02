// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef CCNET_NET_ACCEPTOR_H
#define CCNET_NET_ACCEPTOR_H

#include <functional>

#include "./channel.h"
#include "./socket.h"

namespace ccnet
{
namespace net
{

class EventLoop;
class InetAddress;

/// 接收器，用于服务器端接受连接
/// Acceptor of incoming TCP connections.
///
class Acceptor : noncopyable
{
 public:
  typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  EventLoop* loop_;
  Socket acceptSocket_; // listening socket(即server socket， 专门用于接收TCP连接）
  Channel acceptChannel_; // 用于观察socket的可读事件
  NewConnectionCallback newConnectionCallback_; // 设置连接回调函数
  bool listenning_; // 是否处于监听状态
  int idleFd_; // 空闲描述符， 用于处理EMFILE
};

}
}

#endif  // CCNET_NET_ACCEPTOR_H
