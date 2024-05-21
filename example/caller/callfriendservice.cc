#include <iostream>

#include "mprpcapplication.h"


int main(int argc, char **argv) 
{
    // 整个程序启动以后 想使用 mprpc 框架来享受 rpc 服务调用，一定要先调用框架的初始化函数（one）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的 rpc 方法 Login
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());
    fixbug::GetFriendsListRequest request;

    // rpc 方法请求参数
    request.set_userid(1000);

    // rpc 方法的响应 
    fixbug::GetFriendsListResponse response;

    // 定义一个控制对象 (可以通过 contreller 来了解调用过程中的一些状态信息)
    MprpcController contreller;

    // 最终都会以右示方式调用：RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和网络发送
    stub.GetFriendsList(&contreller, &request, &response, nullptr);

    // 一次 rpc 方法调用完成，读调用的结果
    // 过程中状态不对，则打印状态
    if (contreller.Failed()) 
    {
        std::cout << contreller.ErrorText() << std::endl;
    }
    // 表示 rpc 方法完全正确调用完成
    else
    {
        if (0 == response.result().errcode())
        {
            std::cout << "rpc GetFriendsList response success:" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i) 
            {
                std::cout << "index" << (i + 1) << " name:" << response.friends(i) << std::endl;
            }
        } 
        else 
        {
            std::cout << "rpc GetFriendsList response error:" << response.result().errmsg() << std::endl;
        }
    }

    return 0;
}