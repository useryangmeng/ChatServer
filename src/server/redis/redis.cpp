#include "redis.hpp"

#include <iostream>

using namespace std;

Redis::Redis()
{
    _publish_context = nullptr;
    _subscribe_context = nullptr;
}
Redis::~Redis()
{
    if(_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }
    if(_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }
}

// 连接redis服务器
bool Redis::connect()
{
    //publish的上下文
    _publish_context = redisConnect("127.0.0.1",6379);
    if(!_publish_context)
    {
        cerr << "connect redis error" << endl;
        return false;
    }

    //subscribe的上下文
    _subscribe_context = redisConnect("127.0.0.1",6379);
    if(!_subscribe_context)
    {
        cerr << "connect redis error" << endl;
        return false;
    }

    thread t([&](){
        observer_channel_message();
    });
    t.detach();

    cout << "connect redis success" << endl;
    
    return true;
}

// 向指定的通道channel发送信息
bool Redis::publish(int channel, string message)
{
    redisReply* reply = static_cast<redisReply*>(redisCommand(_publish_context,"PUBLISH %d %s",channel,message.c_str()));

    if(reply == nullptr)
    {
        cerr << "publish command failded" << endl;
        return false;
    }

    freeReplyObject(reply);

    return true;
}

// 向通道subscribe订阅消息
bool Redis::subscribe(int channel)
{
    //由于subscribe命令本身是阻塞的，所以这里只做订阅通道，不接收通道消息
    //通道消息的接收专门放在observer_channel_message函数中独立地进行
    //只负责发送命令，不阻塞redis server响应消息，否则和notifyMsg线程抢占资源

    //以下三个函数是redisCommand函数的内部三个调用过程

    if(REDIS_ERR == redisAppendCommand(_subscribe_context,"SUBSCRIBE %d",channel))
    {
        cerr << "subscribe command error" << endl;
        return false;
    }

    //redieBufferWrite可以循环发送缓冲区，知道缓冲区数据发送完毕（done被设置为1）
    int done = 0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(_subscribe_context,&done))
        {
            cerr << "subscribe command error" << endl;
            return false;
        }
    }

    //redisGetReply(阻塞函数)
    
    return true;
}

// 向通道unsubscribe取消订阅消息
bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR == redisAppendCommand(_subscribe_context,"UNSUBSCRIBE %d",channel))
    {
        cerr << "unsubscribe command error" << endl;
        return false;
    }

    //redieBufferWrite可以循环发送缓冲区，知道缓冲区数据发送完毕（done被设置为1）
    int done = 0;
    while(!done)
    {
        if(REDIS_ERR == redisBufferWrite(_subscribe_context,&done))
        {
            cerr << "unsubscribe command error" << endl;
            return false;
        }
    }

    //redisGetReply(阻塞)
    
    return true;
}

// 在独立的线程中接受订阅通道的消息
void Redis::observer_channel_message()
{
    redisReply* reply = nullptr;
    while(REDIS_OK == redisGetReply(_subscribe_context,(void **)&reply))
    {
        //订阅收到的是一个三元组
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            //给业务层上报通道层发生的消息
            _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }

        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<<" << endl;
}

// 初始化向业务层上报通道消息的回调对象
void Redis::init_notify_handler(function<void(int, string)> fn)
{
    _notify_message_handler = fn;
}