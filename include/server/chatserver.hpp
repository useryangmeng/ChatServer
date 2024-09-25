#pragma once

#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer
{
public:
    ChatServer(EventLoop *loop,
               const InetAddress &listenAddr,
               const string &nameArg);
    // 启动服务
    void start();

private:
    // 连接回调函数
    void onConnection(const TcpConnectionPtr &);
    // 读写回调函数
    void onMessage(const TcpConnectionPtr &,
                   Buffer *,
                   Timestamp);

private:
    TcpServer _server; // 组合的muduo库，实现服务器功能的类对象
    EventLoop *_loop;  // 指向事件循环的指针
};
