#include "../event_loop.h"
#include "../poller.h"
#include "../event_loop_thread.h"
#include "../../base/current_thread.h"
#include "../../base/timestamp.h"
#include "../inet_address.h"
#include "../tcp_client.h"
#include "../channel.h"
#include <iostream>
#include <functional>
#include <stdio.h>
#include <iostream>

using namespace ccnet;
using namespace ccnet::net;
using namespace std;

class TestClient {
public:
    TestClient(EventLoop* loop, const InetAddress& serverAddr, const string& nameArg="TestClient")
        :  client_(loop, serverAddr, nameArg), 
           loop_(loop),
           input_(new Channel(loop, STDIN_FILENO))
    {
        client_.setConnectionCallback(bind(&TestClient::onConnection, this, _1));
        client_.setMessageCallback(bind(&TestClient::onMessage, this, _1, _2, _3));
        input_->setReadCallback(bind(&TestClient::send, this, _1));
        input_->enableReading();

    }

    void connect() {

        client_.connect();
    }


private:
    void onConnection(const ccnet::net::TcpConnectionPtr& conn) {
    std::cout << "EchoClient - " << conn->peerAddress().toIpPort() << " -> "
               << conn->localAddress().toIpPort() << " is "
               << (conn->connected() ? "UP" : "DOWN") << std::endl;
    }

    void onMessage(const ccnet::net::TcpConnectionPtr& conn,
                     ccnet::net::Buffer* buf,
                     ccnet::Timestamp time)
    {
        ccnet::string msg(buf->retrieveAllAsString());
        cout << msg << endl;
        //std::cout << conn->name() << " echo " << msg.size() << " bytes, "
        //       << "data received at " << time.toString() << std::endl;
        //conn->send(msg);
    }

    void send(Timestamp time) {
    {
        printf("%s\n", time.toFormattedString().c_str());
        string msg;
        std::cin>>msg;
        Buffer buf;
        buf.append(msg);
        client_.connection()->send(&buf);
    }

}

    TcpClient client_;
    EventLoop *loop_;
    std::unique_ptr<Channel> input_;
};



int main(void) {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 8888);
    TestClient client(&loop, addr);
    client.connect();

    loop.loop();
    return 0;
}