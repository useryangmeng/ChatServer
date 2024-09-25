#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"

#include <functional>

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg) : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册回调
    this->_server.setConnectionCallback(bind(&ChatServer::onConnection, this, std::placeholders::_1));
    this->_server.setMessageCallback(bind(&ChatServer::onMessage, this, std::placeholders::_1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr & conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        //异常退出
        ChatService::instance()->clientCloseException(conn);
    }
}

void ChatServer::onMessage(const TcpConnectionPtr & conn,
                           Buffer * buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //反序列化
    json js = json::parse(buf);
    //为达到完全解耦网络模块和业务模块
    //通过json的msgid获取对应的事件处理器handler
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    //回调消息绑定好的事件处理器，执行相应的业务处理
    msgHandler(conn,js,time);
}