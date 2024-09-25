#include <vector>
#include <string>

#include "db.h"
#include "user.hpp"

using namespace std;

class FriendModel
{
public:
    void insert(int userid,int friendid);

    vector<User> query(int userid);
private:

};