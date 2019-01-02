#ifndef CCNET_NET_CHANNEL_H
#define CCNET_NET_CHANNEL_H

#include "../base/timestamp.h"

#include <functional>
#include <memory>

namespace ccnet
{
namespace net
{

class EventLoop;
/// Channel 是 selectable IO channel，负责注册与响应 IO 事件，
/// 它不拥有 file descriptor。它是 Acceptor、Connector、EventLoop、TimerQueue、TcpConnection 的成员，生命期由后者控制。 
/// 像公路的管道一样，一端负责注册IO事件， 一端负责分发（响应）IO事件
/// A selectable I/O channel.
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : noncopyable
{
 public:
  typedef std::function<void()> EventCallback; //事件回调处理  
  typedef std::function<void(Timestamp)> ReadEventCallback;  //读事件的回调处理，传一个时间戳  
  
  Channel(EventLoop* loop, int fd);  //一个Channel在一个EventLoop， 构造函数为loop和文件描述符 
  ~Channel();

  void handleEvent(Timestamp receiveTime);    // 处理事件  
  void setReadCallback(ReadEventCallback cb)  // 设置各种回调函数  
  { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb)
  { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb)
  { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb)
  { errorCallback_ = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const std::shared_ptr<void>&); // 与TcpConnection有关，防止事件被销毁。 

  int fd() const { return fd_; } // 描述符  
  int events() const { return events_; } // 注册的事件  
  void set_revents(int revt) { revents_ = revt; } // used by pollers
  bool isNoneEvent() const { return events_ == kNoneEvent; } //判断是否无关注事件类型，events为0 

  void enableReading() { events_ |= kReadEvent; update(); } //或上事件，就是关注可读事件，注册到EventLoop，通过它注册到Poller中  
  void disableReading() { events_ &= ~kReadEvent; update(); }  //取消关注读 
  void enableWriting() { events_ |= kWriteEvent; update(); }  //关注可写事件
  void disableWriting() { events_ &= ~kWriteEvent; update(); } //取消关注读 
  void disableAll() { events_ = kNoneEvent; update(); } // 取消关注所有事件, 从epoll内核事件表中取消关注该fd
  bool isWriting() const { return events_ & kWriteEvent; }  // 是否关注了写  
  bool isReading() const { return events_ & kReadEvent; }  // 是否关注读   

  // for Poller
  int index() { return index_; } // 返回通道状态 
  void set_index(int idx) { index_ = idx; }

  // for debug
  string reventsToString() const;  // 事件转化为字符串，方便打印调试 
  string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();  // 从epoll监听列表中移除channel，确保调用前disableAll  




 private:
  static string eventsToString(int fd, int ev);

  void update(); //更新事件类型  
  void handleEventWithGuard(Timestamp receiveTime);

  static const int kNoneEvent; // 没有事件
  static const int kReadEvent; // 可读事件
  static const int kWriteEvent; // 可写事件

  EventLoop* loop_;     // 所属的唯一Eventloop  
  const int  fd_;       // 所关注的文件描述符fd，但不负责关闭改描述符  
  int        events_;   // 关注的事件  
  int        revents_;  // epoll/poll返回事件
  int        index_;    // used by Poller，表示通道的状态kNew、kAdded、kDeleted
  bool       logHup_;    //for POLLHUP  

  std::weak_ptr<void> tie_; // 弱引用，用于对象生命期的控制（连接）
  bool tied_;		    // 生存期控制
  bool eventHandling_;      // 是否处于处理事件中  
  bool addedToLoop_; // 
  ReadEventCallback readCallback_;  // 几种事件处理回调  
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}
}
#endif  // MUDUO_NET_CHANNEL_H
