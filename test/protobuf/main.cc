#include "test.pb.h"

#include <iostream>
#include <string>

using namespace fixbug;

int main(){
    fixbug::GetFriendListsResponse rsp;
    fixbug::Resultcode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    fixbug::User *users1 = rsp.add_users();
    users1->set_name("zhang san");
    users1->set_age(20);
    users1->set_sex(fixbug::User::MAN);

    fixbug::User *users2 = rsp.add_users();
    users2->set_name("li si");
    users2->set_age(22);
    users2->set_sex(fixbug::User::MAN);

    fixbug::User *users3 = rsp.add_users();
    users3->set_name("li si");
    users3->set_age(22);
    users3->set_sex(fixbug::User::MAN);

    std::cout << rsp.users_size() << std::endl;

    return 0;
}

int main1()
{

    // 封装了login请求对象的数据 
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    // 序列化对象 -》 char*
    // 调用 Serialize______ 函数进行序列化
    std::string send_str;
    if (req.SerializeToString(&send_str)) 
    {
        std::cout << send_str.c_str() << std::endl;
    }

    // 反序列化
    // 调用 ParseFrom______ 函数进行反序列化
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str))
    {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }


    return 0;
}