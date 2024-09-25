#include "chatservice.hpp"
#include "public.hpp"

#include <unordered_map>
#include <muduo/base/Logging.h>
#include <string>
#include <vector>

using namespace muduo;
using namespace std;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, bind(&ChatService::login, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, bind(&ChatService::reg, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, bind(&ChatService::oneChat, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, bind(&ChatService::addFriend, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, bind(&ChatService::createGroup, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, bind(&ChatService::addGroup, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, bind(&ChatService::groupChat, this, std::placeholders::_1, _2, _3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, bind(&ChatService::loginOut, this, std::placeholders::_1, _2, _3)});

    //连接redis服务器
    if(_redis.connect())
    {
        //设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscirbeMessage,this,std::placeholders::_1,_2));
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && pwd == user.getPwd())
    {
        json response;
        if (user.getState() == "online")
        {
            // 用户重复登录
            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 2;
            response["errmsg"] = "该账号已经登录，请重新输入新账号";
        }
        else
        {
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }

            //id用户登录成功后，向redis订阅channel
            _redis.subscribe(id);  

            // 登录成功，更新用户状态
            user.setState("online");
            _userModel.updateState(user);

            response["msgid"] = LOGIN_MSG_ACK;
            response["erron"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;
                // 读取离线消息后，删除该用户的离线消息
                _offlineMsgModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (auto user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();

                    vec2.push_back(js.dump());
                }

                response["friends"] = vec2;
            }

            // 查询该用户的群组信息并返回
            vector<Group> groupVec = _groupModel.queryGroups(id);
            if (!groupVec.empty())
            {
                vector<string> vec2;
                for (auto group : groupVec)
                {
                    json js;
                    js["groupid"] = group.getId();
                    js["groupname"] = group.getName();
                    js["groupdesc"] = group.getDesc();
                    vector<GroupUser> users = group.getUsers();
                    vector<string> usermsg;
                    for (GroupUser user : users)
                    {
                        json msg;
                        msg["name"] = user.getName();
                        msg["id"] = user.getId();
                        msg["role"] = user.getRole();
                        msg["state"] = user.getState();

                        usermsg.push_back(msg.dump());
                    }
                    js["users"] = usermsg;

                    vec2.push_back(js.dump());
                }

                response["groups"] = vec2;
            }
        }

        conn->send(response.dump());
    }
    else
    {
        // 用户账号密码 登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["erron"] = 1;
        response["errmsg"] = "用户名或者密码错误";

        conn->send(response.dump());
    }
}

void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string password = js["password"];

    User user;
    user.setName(name);
    user.setPwd(password);

    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["erron"] = 0;
        response["id"] = user.getId();

        conn->send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["erron"] = 1;

        conn->send(response.dump());
    }
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志：没有对应的处理函数
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认处理器，空操作
        return [=](const TcpConnectionPtr &conn, json &json, Timestamp time)
        {
            LOG_ERROR << "msgid: " << msgid << " can not find handler!";
        };
    }

    return _msgHandlerMap[msgid];
}

// 处理用户的异常退出
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }

    //用户异常注销，取消redis中对应的订阅通道
    _redis.unsubscribe(user.getId());

    if (user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if (it != _userConnMap.end())
        {
            // toid 在线，转发消息 服务器将消息推送到在线用户
            it->second->send(js.dump());
            return;
        }
    }

    //查询在其他服务器中是否在线
    User toUser = _userModel.query(toid);
    if(toUser.getState() == "online")
    {
        _redis.publish(toid,js.dump());
        return;
    }

    // toid不在线，存贮离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把所有的用户的状态设置为offline
    _userModel.resetState();
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp timev)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    _friendModel.insert(userid, friendid);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp timev)
{
    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];

    // 存储新创建的群组信息
    Group group(-1, name, desc);
    if (_groupModel.createGroup(group))
    {
        // 存贮群组创建人信息
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp timev)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();

    _groupModel.addGroup(userid, groupid, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp timev)
{
    int userid = js["id"].get<int>();
    int groupid = js["group"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 在线则转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //查询用户是否在线
            User toUser = _userModel.query(id);
            if(toUser.getState() == "online")
            {
                _redis.publish(id,js.dump());
                break; 
            }
            _offlineMsgModel.insert(id, js.dump());
        }
    }
}

// 处理注销业务
void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp timev)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        _userConnMap.erase(userid);
    }

    //用户注销，取消redis中对应的通道
    _redis.unsubscribe(userid);

    User user;
    user.setId(userid);
    user.setState("offline");
    _userModel.updateState(user);
}

//从redis订阅的消息队列中获取订阅的消息
void ChatService::handleRedisSubscirbeMessage(int userid,string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    _offlineMsgModel.insert(userid,msg);
}