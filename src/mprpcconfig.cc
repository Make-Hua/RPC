#include <iostream>
#include <string>

#include "mprpcconfig.h"


// 解析加载文件配置
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << "is note exist!" << std::endl;
        exit(EXIT_FAILURE);
    }

    // 1.注释    2.配置项    3.去掉开头的多余的空格
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        std::string read_buf(buf);

        // 判断注释
        if (read_buf[0] == '#' || read_buf.empty()) 
        {
            continue;
        }

        Trim(read_buf);

        // 解析配置项
        int idx = read_buf.find("=");
        if (idx == -1) 
        {
            continue; // 不合法
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx);
        Trim(key);
        int end_idx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, end_idx - idx - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }

    // for (auto &mp : m_configMap) {
    //     std::cout << "{" << mp.first << "," << mp.second << "}" << std::endl;
    // }
}

// 查询配置项信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}


// 去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf)
{
    // 去除之前的字符串
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        // 配置字符串之前有空格 则去除
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }

    // 去除之后的字符串
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        // 说明字符串后面有空格
        src_buf = src_buf.substr(0, idx + 1);
    }
}