#ifndef CCNET_NET_INETADDRESS_H
#define CCNET_NET_INETADDRESS_H

#include "../base/copyable.h"
#include "../base/string_piece.h"
#include "./sockets_ops.h"

#include <netinet/in.h>

namespace ccnet
{
///
/// socket地址封装, 仅仅支持ipv4
/// InetAddress addr("127.0.0.1", 1234);
/// addr.toIpPort();
namespace net
{
///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress : public ccnet::copyable
{
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(StringArg ip, uint16_t port);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InetAddress(const struct sockaddr_in& addr)
    : addr_(addr)
  { }



  sa_family_t family() const { return addr_.sin_family; }
  string toIp() const;
  string toIpPort() const;
  uint16_t toPort() const;

  // default copy/assignment are Okay
  const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr_); }
  void setSockAddrInet(const struct sockaddr_in &addr) { addr_ = addr; }

  uint32_t ipNetEndian() const;
  uint16_t portNetEndian() const { return addr_.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(StringArg hostname, InetAddress* result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t port = 0);

 private:
  struct sockaddr_in addr_;

};

}
}

#endif  // CCNET_NET_INETADDRESS_H
