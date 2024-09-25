#pragma once

#include "group.hpp"

#include <string>
#include <vector>

using namespace std;

class GroupModel
{
public:
    //创建群组
    bool createGroup(Group& group);

    //加入群组
    void addGroup(int userid,int groupid,string  role);

    //查询用户所在群组
    vector<Group> queryGroups(int userid);

    //根据指定的groupid查询除自己以外的群组用户的id列表，主要用于群聊业务给群组其他成员群发消息
    vector<int> queryGroupUsers(int userid,int groupid);
private:

};
