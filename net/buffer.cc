#include "./buffer.h"

#include "./sockets_ops.h"

#include <errno.h>
#include <sys/uio.h>

using namespace ccnet;
using namespace ccnet::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

/*

避免开巨大 Buffer 造成的内存浪费
在非阻塞网络编程中，如何设计并使用缓冲区？
一方面我们希望减少系统调用，一次读的数据越多越划算，那么似乎应该准备一个大的缓冲区。
另一方面，我们系统减少内存占用。如果有 10k 个连接，每个连接一建立就
分配 64k 的读缓冲的话，将占用 640M 内存，而大多数时候这些缓冲区的
使用率很低。 muduo 用 readv 结合栈上空间巧妙地解决了这个问题。
借助栈先读出数据，减少系统调用

*/

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read 
  // 减少了ioctl系统调用（获取有多少个可读数据）
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  // 第一缓冲区
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  // 第二个缓冲区
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0)
  {
    *savedErrno = errno;
  }
  else if (implicit_cast<size_t>(n) <= writable) // 如果第一块缓冲区足够容纳
  {
    writerIndex_ += n;
  }
  else
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  // if (n == writable + sizeof extrabuf)
  // {
  //   goto line_30;
  // }
  return n;
}

