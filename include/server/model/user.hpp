#pragma once

#include <string>

using namespace std;

//匹配User表的ORM（映射关系）类
class User
{
public:
    User(int id = -1, string name = "", string password = "", string states = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = password;
        this->states = states;
    }

    void setId(int id) { this->id = id; }
    void setName(string name) { this->name = name; }
    void setPwd(string pwd) { this->password = pwd; }
    void setState(string states) { this->states = states; }

    int getId() { return this->id; }
    string getName() { return this->name; }
    string getPwd() { return this->password; }
    string getState() { return this->states; }

private:
    int id;
    string name;
    string password;
    string states;
};
