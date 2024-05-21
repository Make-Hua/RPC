#include <iostream>
#include <string>

#include "user.pb.h"
#include "mprpcapplication.h"
#include "logger.h"



/*
    UserService 是本地服务，提供了进程内的本地方法 Login.

*/
class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    /*
        重写了基类的 UserServiceRpc 的虚函数 以下方法都是框架，直接调用的 
        1. caller  ->  Login(LoginRequest)  ->  protobuf(序列化)  ->  muduo  ->  protobuf(反序列化)  callee
        2. callee  ->  Login(LoginRequest)  -> 交到以下重写后的方法运行

    */
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,    // 序列化参数需要拿出
                       ::fixbug::LoginResponse* response,        // 响应参数需要写入
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求的参数 LoginRequest, 应用获取相关数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);

        // 把响应写入
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_sucess(login_result);

        // 执行回调操作
        done->Run();
    }


    // 示例 
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id:" << id <<  " name:" << name << " pwd:" << pwd << std::endl;
        return true;
    }

    // 框架调用，需要重写该方法
    void Register(google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool ret = Register(id, name, pwd);
        
        // 写入响应
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_sucess(ret);


        // 执行回调操作
        done->Run();
    }

};

/*
    为了框架操作简单化，则我们使用下面步骤使用框架
*/
int main(int argc, char **argv)
{
    // test
    // LOG_INFO("first log message!");
    // LOG_ERR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 先调用框架的初始化
    MprpcApplication::Init(argc, argv);

    // provider 是一个 rpc 网络服务对象，把 UserService 对象发布到 rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    // 启动一个rpc服务发布节点，Run以后，进入阻塞状态，等待远程 rpc 调用请求
    provider.Run();
    
    return 0;
}