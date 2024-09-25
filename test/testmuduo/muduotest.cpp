#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>

using namespace std;
using namespace muduo;
using namespace muduo::net;

/*muduo网络库的开发步骤
1、组合TcpServer对象
2、创建EventLoop事件循环对象的指针
3、明确TcpServer的构造函数需要的参数，输出ChatServer的构造函数
4、在当前服务器类的构造函数中，注册处理连接的回调函数和处理读写事件的回调函数
5、设置合适的服务端线程数量，moduo库会自己分配IO线程和worker线程

*/

class ChatServer
{
public:
    ChatServer(EventLoop *loop,               // 事件循环
               const InetAddress &listenAddr, // IP + port
               const string &nameArg)         // 服务器/线程名字
        : _server(loop, listenAddr, nameArg), _loop(loop)
    {
        //注册用户连接的创建和断开回调
        _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
        //注册读写事件回调
        _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,_2,_3));
        
        //设置服务器的线程数量，main和sub反应堆模型,1IO线程和工作线程
        _server.setThreadNum(4);
    }

    //开启事件循环
    void start()
    {
        _server.start();
    }

private:
    // 用于处理用户的连接和断开的回调函数
    void onConnection(const TcpConnectionPtr & conn) // 隐含this参数
    {
        if(conn->connected())
        {
            cout << "建立连接" << endl;
        }
        else
        {
            cout << "关闭连接" << endl;
            conn->shutdown();
        }
    }
    //处理用户的读写事件
    void onMessage(const TcpConnectionPtr & conn,//连接
                   Buffer *buffer,//缓冲区 
                   Timestamp time)//接收到数据的时间信息
    {
        string buf = buffer->retrieveAllAsString();
        cout << "revc data:" << buf << "Time : " << time.toFormattedString() << endl;
        conn->send(buf);
    }

    TcpServer _server;
    EventLoop *_loop;
};

int main()
{
    EventLoop loop;//epoll
    InetAddress addr("127.0.0.1",8888);
    ChatServer server(&loop,addr,"ChatServer");

    server.start();//listenfd epoll_ctl->添加epoll
    loop.loop();//epoll_wait阻塞

    return 0;
}