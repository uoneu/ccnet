// 参考文档： https://www.cnblogs.com/Solstice/archive/2011/06/16/2082590.html
#include "sudoku.h"

#include "../../base/atomic.h"
#include "../../base/logging.h"
#include "../../base/thread.h"
#include "../../base/thread_pool.h"
#include "../event_loop.h"
#include "../inet_address.h"
#include "../tcp_server.h"

#include <utility>

#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace ccnet;
using namespace ccnet::net;

class SudokuServer
{
 public:
  SudokuServer(EventLoop* loop, const InetAddress& listenAddr, int numThreads)
    : server_(loop, listenAddr, "SudokuServer"),
      numThreads_(numThreads),
      startTime_(Timestamp::now())
  {
    server_.setConnectionCallback(
        std::bind(&SudokuServer::onConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&SudokuServer::onMessage, this, _1, _2, _3));
    server_.setThreadNum(8);  
  }

  void start()
  {
    LOG_INFO << "starting " << numThreads_ << " threads.";
    threadPool_.start(numThreads_);
    server_.start();
  }

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_TRACE << conn->peerAddress().toIpPort() << " -> "
        << conn->localAddress().toIpPort() << " is "
        << (conn->connected() ? "UP" : "DOWN");
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
  {
    static int aa=0;
    LOG_DEBUG << conn->name();
    //printf("%s: %s\n",conn->name().c_str(), buf->retrieveAllAsString().c_str());
    
    size_t len = buf->readableBytes();
    //printf("%d\n", len);
    while (len >= kCells + 2)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        ++aa;
        LOG_INFO << "[-------" << aa;
        string request(buf->peek(), crlf);
        //LOG_INFO << request;
        buf->retrieveUntil(crlf + 2);
        len = buf->readableBytes();
        if (!processRequest(conn, request))
        {
          conn->send("Bad Request!\r\n");
          conn->shutdown();
          break;
        }
      }
      else if (len > 100) // id + ":" + kCells + "\r\n"
      {
        conn->send("Id too long!\r\n");
        conn->shutdown();
        break;
      }
      else
      {
        LOG_FATAL << "CRLF";
        break;
      }
    }
  }

  bool processRequest(const TcpConnectionPtr& conn, const string& request)
  {
    string id;
    string puzzle;
    bool goodRequest = true;

    string::const_iterator colon = find(request.begin(), request.end(), ':');
    if (colon != request.end())
    {
      id.assign(request.begin(), colon);
      puzzle.assign(colon+1, request.end());
    }
    else
    {
      puzzle = request;
    }

    if (puzzle.size() == implicit_cast<size_t>(kCells))
    {
      threadPool_.run(std::bind(&solve, conn, puzzle, id));
    }
    else
    {
      goodRequest = false;
    }
    return goodRequest;
  }

  static void solve(const TcpConnectionPtr& conn,
                    const string& puzzle,
                    const string& id)
  {
    //LOG_DEBUG << conn->name();
    string result = solveSudoku(puzzle);
    if (id.empty())
    {
      conn->send(result+"\r\n");
    }
    else
    {
      conn->send(id+":"+result+"\r\n");
    }
  }

  TcpServer server_;
  ThreadPool threadPool_;
  int numThreads_;
  Timestamp startTime_;
};


int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid() << ", tid = " << current_thread::tid();
  int numThreads = 0;
  if (argc > 1)
  {
    numThreads = atoi(argv[1]);
  }
  EventLoop loop;
  InetAddress listenAddr(9981);
  SudokuServer server(&loop, listenAddr, numThreads);

  server.start();

  loop.loop();
}

