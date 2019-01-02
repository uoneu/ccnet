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
public:  
    TestServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)  
        : loop_(loop),   
          server_(loop, listenAddr, "TestServer"),  
          numThreads_(numThreads){  
        server_.setConnectionCallback(bind(&TestServer::onConnect, this, _1));  
        server_.setMessageCallback(bind(&TestServer::onMessage, this, _1, _2, _3));
        server_.setThreadNum(numThreads);  
    }     
  
    void start(){  
        server_.start();  
    }

private:  
    void onMessage(const TcpConnectionPtr& tpr,
                    Buffer* buf,
                    Timestamp) {
        printf("%s: %s\n",tpr->name().c_str(), buf->retrieveAllAsString().c_str());
    }

    void onConnect(const TcpConnectionPtr& ptr) {
        if (ptr->connected()) {
            printf("onConnect: %s\n", ptr->name().c_str());
        } else {
            printf("onConnect: %s  close!\n", ptr->name().c_str());
        }
    }   
      
    EventLoop* loop_;  
    TcpServer server_;  
    int numThreads_;  
};    
  


int main()  
{  
    printf("main() : pid = %d\n", getpid());  
  
    InetAddress listenAddr(8888);  
    EventLoop loop;  
  
    TestServer server(&loop, listenAddr, 4);  
    server.start();  
  
    loop.loop();  
}  