#include "../event_loop.h"
#include "../poller.h"
#include "../event_loop_thread.h"
#include "../../base/current_thread.h"
#include "../../base/timestamp.h"
#include "../inet_address.h"
#include "../acceptor.h"
#include <iostream>
#include <functional>
#include <stdio.h>

using namespace ccnet;
using namespace ccnet::net;

void new_connecton(int sockfd, const InetAddress& addr) {
    printf("%s\n", addr.toIpPort().c_str());
    fflush(stdout);
    sockets::write(sockfd,"How are you?\n", 13);
    sockets::close(sockfd);
}

int main(void) {
    EventLoop loop;
    InetAddress addr(8888);
    Acceptor accp(&loop, addr, true);
    accp.setNewConnectionCallback(new_connecton);
    accp.listen();

    loop.loop();

}
