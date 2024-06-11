#include "rpcheader.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "zookeeperutil.h"
#include "logger.h"

/*
service_name => service描述
                        => service* 记录服务对象
                        method_name  =>  method 方法对象
*/
// 这里是框架提供给外部使用的
// 作用: 发布 RPC方法的站点
void RpcProvider::NotifyService(google::protobuf::Service *service)
{
    ServiceInfo service_info;

    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    // 获取服务的名字
    std::string service_name = pserviceDesc->name();

    // 获取服务对象service的方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service_name:" << service_name << std::endl;  -> 转为日志
    LOG_INFO("service_name:%s", service_name.c_str());
    

    for (int i = 0; i < methodCnt; ++i) 
    {
        // 获取了服务对象指定下标的服务方法的描述（抽象描述）
        // MethodDescriptor 具体对应的子类方法，例如需要调用的 Login 方法（将框架以抽象设计，以开源至所有人均可以用）
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc -> method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_mathodMap.insert({method_name, pmethodDesc});

        // std::cout << "method_name:" << method_name << std::endl;
        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动 rpc 服务节点，开始提供 rpc 远程调用服务
void RpcProvider::Run()
{
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverprot").c_str());

    muduo::net::InetAddress address(ip, port);

    // 创建 Tcpserver 对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 绑定连接回调和消息读写回调方法(解耦网络和业务代码)
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置 muduo 库的线程数量
    server.setThreadNum(4);


    // 把当前 rpc 节点上要发布的服务全部注册到 zk 上面， 让 rpc client 可以从 zk 上发现服务
    ZkClient zkCli;
    zkCli.Start();

    // service_name 为永久节点
    // method_name  为临时节点
    for (auto &sp : m_serviceMap)
    {
        // path: /service_name(永久节点)  and    /UserServiceRpc(临时节点)
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_mathodMap)
        {
            //  /service_name/method_name       /UserServiceRpc/Login  存储当前 rpc 服务节点的主机 ip and port
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL); // ZOO_EPHEMERAL 临时节点
        }
    }


    std::cout << "RpcProvider start service at ip:" << ip << " port:" << port << std::endl;

    // 启动网络服务
    server.start();

    // 以阻塞的方式等待连接，当 Tcp 三次握手建立连接成功时，会自动回调 bind 所绑定的 OnConnection 函数
    m_eventLoop.loop();
}

 // 新 socket 连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn)
{
    if (!conn->connected())
    {
        // 和 rpc client 的连接断开了
        conn->shutdown();
    }
}

/*
在框架内部，RpcProvider 和 RpcConsumer 需要协商好之间的通信用的 protobuf 数据类型
service_name method_name ages    定义 proto 的 message 类型，进行数据的序列化和反序列化
                                    service_name mtrhod_name args_size
16UserServiceLoginzhang san123456

herder_size(4字节) + herder_str + arges_str

不能直接转 int -> string
10 -> '10'      √
10000 -> '10000'    ×
所以我们直接按照二进制的存储方式 
*/
// 建立连接用户的读写事件回调const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn,
                            muduo::net::Buffer* buffer,
                            muduo::Timestamp)
{
    // 从网络上接受的远程 rpc 调用请求的字符流   Login args
    std::string recv_buf = buffer->retrieveAllAsString();

    // 从字符流中读取前4个字节的内容  长度
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);

    // 根据 header_size 读取数据头的原始字符流，反序列化数据，得到rpc请求详细信息  具体内容
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;

    // 反序列化操作  zParseFromString
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        LOG_ERR("rec_header_str:%sparse error!", rpc_header_str.c_str());
        return ;
    }

    // 获取 rpc 方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);


    /* 调试 */
    std::cout << "===========================================" << std::endl;
    std::cout << "header_size:" << header_size << std::endl;
    std::cout << "rpc_header_str:" << rpc_header_str << std::endl;
    std::cout << "service_name:" << service_name << std::endl;
    std::cout << "method_name:" << method_name << std::endl;
    std::cout << "args_str:" << args_str << std::endl;
    std::cout << "===========================================" << std::endl;

    // 获取 service 对象和 method 对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        LOG_ERR("%sis not exist", service_name.c_str());
        return ;
    }

    auto mit = it->second.m_mathodMap.find(method_name);
    if (mit == it->second.m_mathodMap.end())
    {
        
        LOG_ERR("%s:%sis not exist", service_name.c_str(), method_name.c_str());
        return ;
    }


    // void Login(::google::protobuf::RpcController* controller,
    //                    const ::fixbug::LoginRequest* request,    // 序列化参数需要拿出
    //                    ::fixbug::LoginResponse* response,        // 响应参数需要写入
    //                    ::google::protobuf::Closure* done)

    // 获取 service 对象(new UserService) and 获取 mrthod 对象(具体rpc方法)
    google::protobuf::Service* service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成 rpc 方法调用的请求 request 和响应 response 参数
    //  GetRequestPrototype           GetResponsePrototype
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    if (!request->ParseFromString(args_str))
    {
        LOG_ERR("request parse error! content:%s", args_str.c_str());
        return ;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的 method 方法的调用，绑定一个 Closure 的回调函数
    google::protobuf::Closure *done = 
        google::protobuf::NewCallback<RpcProvider, const muduo::net::TcpConnectionPtr&, google::protobuf::Message*>
            (this, &RpcProvider::SendRpcResponse, conn, response);
    
    // 在框架上根据远端 rpc 请求，调用当前 rpc 节点上发布的方法
    // new UserService().Login(constroller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);

    /* 上述 CallMethod 其实最终执行的是具体的 实现方法
    void FriendServiceRpc::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                                  ::google::protobuf::RpcController* controller,
                                  const ::google::protobuf::Message* request,
                                  ::google::protobuf::Message* response,
                                  ::google::protobuf::Closure* done) {
    // 调用具体的方法实现
    if (method->name() == "GetFriendsList") {
        // 实际调用 GetFriendsList 方法
        GetFriendsList(controller, static_cast<const GetFriendsListRequest*>(request),
                    static_cast<GetFriendsListResponse*>(response), done);
    }
    }
    */


}


// Closure 的回调操作，用于序列话 rpc 的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, google::protobuf::Message* response)
{
    std::string response_str;

    // response 进行序列化
    if (response->SerializePartialToString(&response_str))
    {   
        // 序列化成功后，将序列话结果通过网络把 rpc 方法执行结果发送给 rpc 调用方
        conn->send(response_str);
    }
    else
    {
        LOG_ERR("serialize response_str error!");
    }

    // 模拟 http 短链接服务，由 rpcprovider 主动断开连接
    conn->shutdown();
}