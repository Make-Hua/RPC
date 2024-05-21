#include <iostream>
#include <string>

#include "friend.pb.h"
#include "mprpcapplication.h"
#include "logger.h"

class FriendService : public fixbug::FriendServiceRpc
{
public:
    std::vector<std::string> GetFriendsList(uint32_t userid)
    {
        std::cout << "do GetFriendsList service!" << std::endl;

        // 模拟本地业务操作
        std::vector<std::string> vec;
        vec.emplace_back("he peng");
        vec.emplace_back("xie boss");
        vec.emplace_back("kun kun");

        return vec;
    }

    // 框架调用方法  重写基类方法
    void GetFriendsList(::google::protobuf::RpcController* controller,
                       const ::fixbug::GetFriendsListRequest* request,
                       ::fixbug::GetFriendsListResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t userid = request->userid();

        std::vector<std::string> friendsList = GetFriendsList(userid);

        // 写入响应
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        
        for (auto &name : friendsList)
        {
            auto *p = response->add_friends();
            *p = name;
        }


        // 执行回调操作
        done->Run();
    }

private:


};



int main(int argc, char **argv)
{
    // test
    // LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 先调用框架的初始化
    MprpcApplication::Init(argc, argv);

    // provider 是一个 rpc 网络服务对象，把 UserService 对象发布到 rpc节点上
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    // 启动一个rpc服务发布节点，Run以后，进入阻塞状态，等待远程 rpc 调用请求
    provider.Run();
    
    return 0;
}