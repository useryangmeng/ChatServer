#pragma once

#include "user.hpp"

//User表的数据操作类
class UserModel
{
public:
    //User表的增加
    bool insert(User&);

    //根据主键用户id查询信息
    User query(int id);

    //更新用户的状态信息
    bool updateState(User user);

    //重置用户的状态信息
    void resetState();
};