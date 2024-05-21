#pragma once

#include <iostream>

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/TcpConnection.h>
#include <iostream>
#include <string>
#include <functional>
#include <google/protobuf/descriptor.h>
#include <unordered_map>

#include "google/protobuf/service.h"


// 框架提供的专门发布 rpc 服务的网络对象类
class RpcProvider
{
public:
    // 这里是框架提供给外部使用的，
    void NotifyService(google::protobuf::Service *service);

    // 启动 rpc 服务节点，开始提供 rpc 远程调用服务
    void Run();

private:
    // 组合后的 EventLoop
    muduo::net::EventLoop m_eventLoop;

    // service 服务类型信息
    struct ServiceInfo
    {
        // 保存服务对象
        google::protobuf::Service *m_service;

        // 保存服务方法
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> m_mathodMap;
    };

    // 存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 新 socket 连接回调
    void OnConnection(const muduo::net::TcpConnectionPtr&);

    // 建立连接用户的读写事件回调
    void OnMessage(const muduo::net::TcpConnectionPtr&, muduo::net::Buffer*, muduo::Timestamp);

    // Closure 的回调操作，用于序列话 rpc 的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr&, google::protobuf::Message*);
};