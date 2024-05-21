#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "mprpcchannel.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

/*          数据发送格式
    header_size + service_name      method_name         args_size + args
*/
// 所有通过 stub 代理对象调用的 rpc 方法，都走到这里了，统一做 rpc 方法调用的数据数据序列化和网络发送
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller, 
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response, 
                            google::protobuf::Closure* done)
{
    const google::protobuf::ServiceDescriptor* sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // 获取参数序列化的长度 args_size
    uint32_t args_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        args_size = args_str.size();
    }
    else
    {
        // std::cout << "serialize request error!" << std::endl;
        controller->SetFailed("serialize request error!");
        return ;
    }

    // 定义 rpc 的请求 header
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(args_size);

    // 序列化
    uint32_t header_size = 0;
    std::string rpc_header_str;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout << "serialize rpc header error!" << std::endl;
        controller->SetFailed("serialize rpc header error!");
        return ;
    }

    // 拼接最终发送字符串
    std::string send_rpc_str;

    // 前 4 个字节确定为整数
    send_rpc_str.insert(0, std::string((char*)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;


    /* 调试 */
    std::cout << "===========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "===========================================" << std::endl;

    // 相当于客户端，无需考虑高并发,直接使用 tcp 编程，完成远程调用
    // 创建一个新的套接字描述符     AF_INET： IPv4      SOCK_STREAM：   面向连接，属于 TCP 协议     0：一般均填0
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        //std::cout << "create socket error! errno:" << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    /* // Tcp 编程
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t prot = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverprot").c_str());
    */
    /*
    struct sockaddr_in :            IPv4 的套接字地址
    server_addr.sin_family:         设置地址族,表示是一个 IPv4地址
    server_addr.sin_port:           设置端口号
    server_addr.sin_addr.s_addr:    设置 IP 地址
    */
    ZkClient zkCli;
    zkCli.Start();

    //    /UserServiceRpc/Login
    std::string method_path = "/" + service_name + "/" + method_name;

    // 127.0.0.1:8000
    std::string host_data = zkCli.GetData(method_path.c_str());
    if ("" == host_data)
    {
        controller->SetFailed(method_path + " is not exist!");
        return ;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + " address is invalid!");
        return ;
    }
    std::string ip = host_data.substr(0, idx);
    uint32_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());
    
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 连接 rpc 服务节点
    if (-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        // std::cout << "connect error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    // 发送 rpc 请求
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {   
        // std::cout << "send error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    // 接受 rpc 请求的响应值
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // std::cout << "recv error! errno:" << errno << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno:%d", errno);
        controller->SetFailed(errtxt);
        return ;
    }

    // 反序列化 rpc 调用后的响应数据
    // std::string response_str(recv_buf, 0, recv_size);
    // if (!response->ParseFromString(response_str))
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error! response_str:" << recv_buf << std::endl;
        close(clientfd);
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! errno:%s", recv_buf);
        controller->SetFailed(errtxt);
        return ;
    }

    close(clientfd);
}