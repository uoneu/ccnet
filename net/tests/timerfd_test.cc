#include "../event_loop.h"
#include "../poller.h"
#include "../channel.h"
#include "../../base/current_thread.h"
#include "../../base/timestamp.h"
#include <iostream>
#include <functional>
#include<sys/timerfd.h>

using namespace ccnet::net;
using namespace ccnet;

EventLoop *g_loop = nullptr;
int timerfd;
Channel *cc;


void timeouts(Timestamp rec) {
    printf("timeout: %s\n", rec.toString().c_str());
    uint64_t howlong;
    ::read(timerfd, &howlong, sizeof howlong);
    cc->disableAll();
    cc->remove();
    g_loop->quit();
}

int main(void) {
    EventLoop loop;
    g_loop = &loop;

    timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(timeouts);
    channel.enableReading();
    cc = &channel;

    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 1;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);


    loop.loop();


    ::close(timerfd);


    return 0;
}