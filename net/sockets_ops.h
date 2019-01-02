#ifndef CCNET_NET_SOCKETSOPS_H
#define CCNET_NET_SOCKETSOPS_H

#include <arpa/inet.h>
///
///
/// socket的一些基础api
///
///


namespace ccnet
{
namespace net
{
namespace sockets
{

///
/// Creates a non-blocking socket file descriptor,
/// abort if any error.
int createNonblockingOrDie(sa_family_t family); // 创建一个非阻塞IO

int  connect(int sockfd, const struct sockaddr* addr); // 发起连接
void bindOrDie(int sockfd, const struct sockaddr* addr); // 绑定socket
void listenOrDie(int sockfd); // 监听
int  accept(int sockfd, struct sockaddr_in* addr); // 接受连接, 返回的是非阻塞fd
ssize_t read(int sockfd, void *buf, size_t count); // 文件读数据
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt); // 分散内存块读
ssize_t write(int sockfd, const void *buf, size_t count); // 文件写数据
void close(int sockfd); //关闭文件描述符
void shutdownWrite(int sockfd); //只关闭写的一半  


//将ip和port的字符串放在buffer中，点分十进制 
void toIpPort(char* buf, size_t size,
	  const struct sockaddr* addr); 

void toIp(char* buf, size_t size,
          const struct sockaddr* addr);


//从ip和port转化sockadd_in类型  
void fromIpPort(const char* ip, uint16_t port,
                struct sockaddr_in* addr);

  
//返回socket错误  
int getSocketError(int sockfd);

// socket地址之间转换
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);


struct sockaddr_in getLocalAddr(int sockfd); // 获得本地地址
struct sockaddr_in getPeerAddr(int sockfd); // 获取对等方地址, 比如获得客户端地址
bool isSelfConnect(int sockfd);

}
}
}

#endif  // CCNET_NET_SOCKETSOPS_H
