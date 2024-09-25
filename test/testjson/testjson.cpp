#include "../third/json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

//json example 1
void func1()
{
    json js;
    js["id"] = {1,2,3,4,5};
    js["form"] = "kiki";
    js["to"] = "mash";
    js["msg"] = "hello mash";
    js["msg"] = "i ke";

    string buf;
    buf = js.dump();
    cout << buf.c_str() << endl;
}

//json example 2
void func2()
{
    json js;
    js["msg"]["1"] = "hello";
    js["msg"]["2"] = "sb";

    string buf = js.dump();
    cout << buf.c_str() <<endl;
}

//json example 3
string func3()
{
    json js;

    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);

    js["list"] = vec;

    map<int,string> path;
    path.insert({1,"Peking"});
    path.insert({2,"Nanking"});
    path.insert({3,"Toking"});

    js["path"] = path;

    cout << js <<endl; 

    return js.dump();
}

int main()
{
    string buf = func3();
    //反序列化
    json rent = json::parse(buf);

    cout << rent["list"] << endl;
    cout << rent["path"] << endl;

    auto alist = rent["list"];
    vector<int> vlist = rent["list"];//both ok

    return 5;
}