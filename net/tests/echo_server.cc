#include "../event_loop.h"
#include "../poller.h"
#include "../event_loop_thread.h"
#include "../../base/current_thread.h"
#include "../../base/timestamp.h"
#include "../inet_address.h"
#include "../tcp_server.h"
#include "../acceptor.h"
#include <iostream>
#include <functional>
#include <stdio.h>
#include <iostream>

using namespace ccnet;
using namespace ccnet::net;


class EchoServer
{
 public:
  EchoServer(ccnet::net::EventLoop* loop,
             const ccnet::net::InetAddress& listenAddr)
    :server_(loop, listenAddr, "EchoServer")
  {
    server_.setConnectionCallback(bind(&EchoServer::onConnection, this, _1));
    server_.setMessageCallback(bind(&EchoServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(3);
  }

  void start() {// calls server_.start();
    server_.start();
  }

 private:
  void onConnection(const ccnet::net::TcpConnectionPtr& conn) {
    std::cout << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
               << conn->localAddress().toIpPort() << " is "
               << (conn->connected() ? "UP" : "DOWN") << std::endl;
  }

  void onMessage(const ccnet::net::TcpConnectionPtr& conn,
                 ccnet::net::Buffer* buf,
                 ccnet::Timestamp time)
  {
      ccnet::string msg(buf->retrieveAllAsString());
      std::cout << conn->name() << " echo " << msg.size() << " bytes, "
               << "data received at " << time.toString() << std::endl;
      conn->send(msg);
  }

  ccnet::net::TcpServer server_;
};




int main(void) {
    printf("main() : pid = %d\n", getpid());  

    EventLoop loop;
    InetAddress addr(8888);
    EchoServer server(&loop, addr);
    
    server.start();
    loop.loop();


    return 0;
}