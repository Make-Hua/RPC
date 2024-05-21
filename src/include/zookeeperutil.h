#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

// 封装类(原生 API 均为 c 风格接口，封装成 c++ 的)
class ZkClient
{
public:
    ZkClient();
    ~ZkClient();

    // zkclient 启动连接 zkserver
    void Start();

    // 在 zkserver 上根据指定 节点path 创建 znode节点
    void Create(const char *path, const char *data, int datalen, int state = 0);

    // 根据指定路径，获取 znode节点 的值
    std::string GetData(const char *path);

private:
    // zk 的客户端句柄
    zhandle_t *m_zhandle;
};