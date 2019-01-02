// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "./tcp_server.h"


#include "./acceptor.h"
#include "./event_loop.h"
#include "./event_loop_threadPool.h"
#include "./sockets_ops.h"
#include "../base/logging.h"

#include <stdio.h>  // snprintf
#include <iostream>

using namespace ccnet;
using namespace ccnet::net;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(loop),
    ipPort_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
  assert(loop != nullptr);
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, this, _1, _2));  // 设置新连接到来时，新的回调函数
}

TcpServer::~TcpServer()
{
  loop_->assertInLoopThread();
 // LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& item : connections_)
  {
    TcpConnectionPtr conn(item.second);
    item.second.reset(); // 释放当前所控制的对象，引用计数减1
    conn->getLoop()->runInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

// 该函数多次调用是无害的
// 可跨线程调用
void TcpServer::start()
{
  if (started_.getAndSet(1) == 0) // 如果没有启动
  {
    threadPool_->start(threadInitCallback_);

    assert(!acceptor_->listenning());
    loop_->runInLoop(
        std::bind(&Acceptor::listen, get_pointer(acceptor_))); // 启动监听，观察是否有新的TCP连接
  }
  //printf("%s TcpServer::start()\n", Timestamp::now().toFormattedString().c_str());
  LOG_INFO << "TcpServer::start()";
}


// 一旦有新的连接请求，将调用该函数
// 该函数内新建一个TcpConnection，设置好相应的回调函数，一旦channel有事件发生，则调用对应的函数
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
  
  loop_->assertInLoopThread();
  EventLoop* ioLoop = threadPool_->getNextLoop(); // 轮转从loop线程池中取一个loop
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;
  //printf("%s TcpServer::newConnection: %s loop: %p\n", Timestamp::now().toFormattedString().c_str(), connName.c_str(), ioLoop);
  LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));
  connections_[connName] = conn; // 这里保证了conn不会自动销毁管理的对象
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe 设置连接关闭回调函数
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn)); // 连接建立之后，在ioLoop中监听连接上的事件
}

// 从服务器中删除连接
void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
  // FIXME: unsafe
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  std::cout  << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name() << std::endl;
  size_t n = connections_.erase(conn->name()); // conn引用计数减一
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn)); //这里conn的引用计数会+1，conn是shared_ptr 
}

