#ifndef CCNET_NET_POLLER_EPOLLPOLLER_H
#define CCNET_NET_POLLER_EPOLLPOLLER_H

#include "./poller.h"

#include <vector>

struct epoll_event;

namespace ccnet
{
namespace net
{

/// 
/// IO Multiplexing with epoll(4).
/// epoll的封装
///
//
class EPollPoller : public Poller
{
 public:
  EPollPoller(EventLoop* loop);
  virtual ~EPollPoller();

   // 调用epoll_wait,得到就绪事件，然后调用fillActiveChannels，用activeChannels返回活动的通道
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels);

  virtual void updateChannel(Channel* channel); // 在epoller中注册或者更新通道, 适用epoll_ctl控制
  
  virtual void removeChannel(Channel* channel); // 从epoller中移除通道，poller中记录了所有关注的Channel



 private:
  static const int kInitEventListSize = 16; //初始事件表大小

  static const char* operationToString(int op);

  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const; // 将活跃的通道加入到活跃事件列表
  void update(int operation, Channel* channel); 

  typedef std::vector<struct epoll_event> EventList; 

  int epollfd_; //内核事件表fd
  EventList events_; // 事件列表大小，用于返回事件
};

}
}
#endif  // CCNET_NET_POLLER_EPOLLPOLLER_H
