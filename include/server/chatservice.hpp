#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "json.hpp"
#include "public.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;

using json = nlohmann::json;
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &json, Timestamp time)>; // 处理消息的事件回调函数类型

class ChatService
{
public:
    static ChatService *instance();

    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp timev);

    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp timev);

    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp timev);

    // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp timev);

    //处理注销业务
    void loginOut(const TcpConnectionPtr &conn, json &js, Timestamp timev);

    //从redis订阅的消息队列中获取订阅的消息
    void handleRedisSubscirbeMessage(int userid,string msg);

    // 服务器异常，业务重置方法
    void reset();

    // 获取消息对应的处理器
    MsgHandler getHandler(int);

    // 处理用户的异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

private:
    ChatService();

    // 存储消息id和其对应的处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接对象
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap的线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    //redis 操作对象
    Redis _redis;
};
