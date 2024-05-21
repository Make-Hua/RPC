#pragma once

#include <unordered_map>

// 配置文件类
/* (配置文件格式)
        rpcserver_ip=
        rpcserver_port=
        zookeeper_ip =
        zookeeper_port= 
    */ 
class MprpcConfig
{
public:
    // 解析加载文件配置
    void LoadConfigFile(const char *config_file);

    // 查询配置项信息
    std::string Load(const std::string &key);

private:
    // 存储配置信息的键值对（由于框架只需要 init 一次，所以不需要考虑线程安全的问题）
    std::unordered_map<std::string, std::string> m_configMap;

    // 去掉字符串前后的空格
    void Trim(std::string &src_buf);
};