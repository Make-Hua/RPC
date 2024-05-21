#include <iostream>
#include <unistd.h>

#include "mprpcapplication.h"
#include "logger.h"

MprpcConfig MprpcApplication::m_config;

MprpcApplication::MprpcApplication() {};


void ShowArgsHelp()
{
    // std::cout << "format: command -i <configfile>" << std::endl;
    LOG_ERR("format: command -i <configfile>");
}


void MprpcApplication::Init(int argc, char **argv)
{
    if (argc < 2) 
    {
        // 修改成日志
        ShowArgsHelp();
        exit(EXIT_FAILURE); 
    }

    int c = 0;
    std::string config_file;
    while (-1 != (c = getopt(argc, argv, "i:")))
    {
        switch (c)
        {
        case 'i':
            config_file = optarg;
            break;
        
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE); 

        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE); 

        default:
            break;
        }
    }
    
    /* 开始加载配置文件(配置文件格式)
        rpcserver_ip=
        rpcserver_port=
        zookeeper_ip =
        zookeeper_port= 
    */ 
   m_config.LoadConfigFile(config_file.c_str());
   
//    std::cout << "rpcserverip" << m_config.Load("rpcserverip") << std::endl;
//    std::cout << "rpcserverprot" << m_config.Load("rpcserverprot") << std::endl;
//    std::cout << "zookeeperip" << m_config.Load("zookeeperip") << std::endl;
//    std::cout << "zookeeperprot" << m_config.Load("zookeeperprot") << std::endl;


}


MprpcApplication& MprpcApplication::GetInstance()
{
    static MprpcApplication app;
    return app;
};

MprpcConfig& MprpcApplication::GetConfig()
{
    return m_config;
}