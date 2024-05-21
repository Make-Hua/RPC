#pragma once

#include "mprpcconfig.h"
#include "mprpccontroller.h"
#include "rpcprovider.h"
#include "mprpcchannel.h"
#include "friend.pb.h"


// mprpc 框架基础类(负责框架初始化)
// 由于框架只需要一个，则设计为单例模式
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();

private:
    // 配置文件类
    static MprpcConfig m_config;

    MprpcApplication();
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};