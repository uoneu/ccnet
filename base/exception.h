#ifndef CCNET_BASE_EXCEPTION_H
#define CCNET_BASE_EXCEPTION_H

#include "./types.h"
#include <exception>

namespace ccnet
{
///
///  带stack trace 的异常基类
///

class Exception : public std::exception
{
 public:
  explicit Exception(const char* what);
  explicit Exception(const string& what);
  virtual ~Exception() throw(); // 表示不允许抛出任何异常，即函数是异常安全的
  virtual const char* what() const throw();
  const char* stackTrace() const throw();

 private:
  void fillStackTrace();

  string message_; // 异常信息
  string stack_;
};

}

#endif  // CCNET_BASE_EXCEPTION_H
