#ifndef CCNET_BASE_LOGGING_H
#define CCNET_BASE_LOGGING_H

#include "./log_stream.h"
#include "./timestamp.h"

namespace ccnet
{

class TimeZone;

class Logger
{
 public:
  enum LogLevel
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
  };

  // compile time calculation of basename of source file
  class SourceFile
  {
   public:
    template<int N>
    inline SourceFile(const char (&arr)[N])
      : data_(arr),
        size_(N-1)
    {
      const char* slash = strrchr(data_, '/'); // builtin function
      if (slash)
      {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* filename)
      : data_(filename)
    {
      const char* slash = strrchr(filename, '/');
      if (slash)
      {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
  };

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char* func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream& stream() { return impl_.stream_; }

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);

  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc); //设置输出函数
  static void setFlush(FlushFunc); //清空缓冲
  static void setTimeZone(const TimeZone& tz);

 private:

  class Impl //这里是真正的实现
  {
   public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
    void formatTime(); // 格式化信息
    void finish(); // 完成输出，最后格式化文件名和行号到缓存
    Timestamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

  Impl impl_; //实现对象

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//
#define LOG_TRACE if (ccnet::Logger::logLevel() <= ccnet::Logger::TRACE) \
  ccnet::Logger(__FILE__, __LINE__, ccnet::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (ccnet::Logger::logLevel() <= ccnet::Logger::DEBUG) \
  ccnet::Logger(__FILE__, __LINE__, ccnet::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (ccnet::Logger::logLevel() <= ccnet::Logger::INFO) \
  ccnet::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN ccnet::Logger(__FILE__, __LINE__, ccnet::Logger::WARN).stream()
#define LOG_ERROR ccnet::Logger(__FILE__, __LINE__, ccnet::Logger::ERROR).stream()
#define LOG_FATAL ccnet::Logger(__FILE__, __LINE__, ccnet::Logger::FATAL).stream()
#define LOG_SYSERR ccnet::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL ccnet::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::ccnet::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
{
  if (ptr == NULL)
  {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

}

#endif  // CCNET_BASE_LOGGING_H
