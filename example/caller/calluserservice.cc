#include <iostream>

#include "mprpcapplication.h"
#include "mprpcchannel.h"
#include "user.pb.h"

int main(int argc, char **argv) 
{
    // 整个程序启动以后 想使用 mprpc 框架来享受 rpc 服务调用，一定要先调用框架的初始化函数（one）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的 rpc 方法 Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());
    fixbug::LoginRequest request;

    // rpc 方法请求参数
    request.set_name("zhang san");
    request.set_pwd("123456");

    // rpc 方法的响应 
    fixbug::LoginResponse response;

    // 定义一个控制对象 (可以通过 contreller 来了解调用过程中的一些状态信息)
    MprpcController contreller;

    // 最终都会以右示方式调用：RpcChannel -> RpcChannel::callMethod 集中来做所有 rpc 方法调用的参数序列化和网络发送
    stub.Login(&contreller, &request, &response, nullptr);

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
            std::cout << "rpc login response success:" << response.sucess() << std::endl;
        } 
        else 
        {
            std::cout << "rpc login response error:" << response.result().errmsg() << std::endl;
        }
    }

    // 演示调用 rpc 的 Register 方法
    fixbug::RegisterRequest req;
    req.set_id(2000);
    req.set_name("mprpc");
    req.set_pwd("666666");
    fixbug::RegisterResponse rsp;

    // 定义一个控制对象 (可以通过 contreller 来了解调用过程中的一些状态信息)
    MprpcController contreller_reg;

    // 以同步的方式发起 rpc 调用请求，等待返回结果
    stub.Register(&contreller_reg, &req, &rsp, nullptr);

    if (contreller_reg.Failed()) 
    {
        std::cout << contreller_reg.ErrorText() << std::endl;
    } 
    else
    {
        if (0 == rsp.result().errcode()) 
        {
            std::cout << "rpc Register response success:" << response.sucess() << std::endl;
        }
        else 
        {
            std::cout << "rpc Register response error:" << response.result().errmsg() << std::endl;
        }
    }
    

    return 0;
}