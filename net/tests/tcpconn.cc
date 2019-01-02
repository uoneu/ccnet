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

using namespace ccnet;
using namespace ccnet::net;

class TestServer {
public :
    TestServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads=0)
        : loop_(loop),
          server_(loop, listenAddr, "TestServer"),
          numThreads_(numThreads),
          msg1(100, 'A'), msg2(200, 'B')
    {
        server_.setMessageCallback(bind(&TestServer::onMessage, this, _1, _2, _3));
        server_.setConnectionCallback(bind(&TestServer::onConnect, this, _1));


    }

    void start() {
        server_.start();
    }

private:
    void onMessage(const TcpConnectionPtr &conn, Buffer *buf, Timestamp time) {

    }
    void onConnect(const TcpConnectionPtr& conn) {
        printf("onConnect\n");
        conn->send(msg1);
        conn->send(msg2);
        conn->shutdown();
        printf("onConnect finish\n");

    }

    string msg1, msg2;

    TcpServer server_;
    EventLoop *loop_;
    int numThreads_;

};

int main(void) {
    InetAddress addr(9981);
    EventLoop loop;
    TestServer server(&loop, addr);

    server.start();
    loop.loop();
}