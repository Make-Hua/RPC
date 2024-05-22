## 基于 muduo + protobuf 实现的分布式通信框架(RPC)

### 一、项目说明

项目是在 Linux 环境下基于 muduo 网络库 和 protobuf 通信协议实现的 RPC 分布式通信框架，同时使用了 zookeeper 中间件，实现分布式一致性协调服务(注册服务、发现功能)。可以通过本框架将本地方法调用重构成基于 TCP 网络通信的 RPC 远程方法调用。实现了在分布式环境中的远程调用，令方法在不同机器或不同服务器上运行，以达到分布式集群效果。

### 二、项目特点

- 通过 muduo 网络库实现高并发的网络通信模块

- 通过 protobuf 实现RPC方法调用和参数的序列化与反序列化

- 通过 zookeeper 实现分布式一致性协调服务，由此提供服务注册和发现功能

- 实现了基于 TCP 传输的二进制协议、解决了粘包问题，并且能高效的数据传输服务器名、方法名及参数。

### 三、开发环境

- 操作系统：`Ubuntu 18.04.2`
- 编译器：`g++ 7.5.0`
- 编译器：`vscode`
- 版本控制：`git`
- 项目构建：`cmake 3.10.2`

### 四、RPC 通信原理以及代码交互图

#### RPC 通信原理

![](https://github.com/Make-Hua/RPC/blob/master/image/RPC%E9%80%9A%E4%BF%A1%E5%8E%9F%E7%90%86%E5%9B%BE.png)

#### 代码交互

具体图片

### 五、构建项目

直接运行脚本即可自动编译项目

```shell
./autobuild.sh
```

### 六、项目启动

#### 启动ZooKeeper

由于项目中使用 zookeeper 实现分布式一致性协调服务，所以我们需要先在 Linux 下启动 zookeeper

```shell
# 需要切换到 zookeeper 安装目录下的 bin 目录(以下为我目录的示例)
ubuntu@ ~$ cd /package/zookeeper-3.4.10/bin

# 如何启动 zookeeper 服务
ubuntu@ ~/package/zookeeper-3.4.10/bin$ ./zkServer.sh start
```



验证是否开启项目，如若开启成功，则可以在进程中找到 zookeeper

```shell
# 验证是否启动
# 方法一
ubuntu@ ~$ sudo netstat -tanp
# 显示内容中含如下内容即开启服务成功
tcp6  0  0 :::2181       :::*         LISTEN      16596/java

# 方法二
ubuntu@ ~$ ps -aux | grep zookeeper
```



在启动 zookeeper 客户端脚本，启动客户端是为了检测是否能为RPC框架插入 znode 节点信息

```shell
ubuntu% ./zkCli.sh
```

### 七、MPRPC框架示例

#### 在项目文件的`bin`目录下有提供者和消费者两个文件，分别启动

##### 启动提供者

可以看到启动后 zookeeper 已经初始化成功

```shell
# 启动服务节点
ubuntu@ ~/RPC/bin$ ./provider -i test.conf
# 出现如下内容成功
[time],883:2955(0x7fedf5e3ca00):ZOO_INFO@log_env@726: Client environment:zookeeper.version=zookeeper C client 3.4.10
......
[time],941:2955(0x7fedf33bc700):ZOO_INFO@check_events@1775: session establishment complete on server [127.0.0.1:2181], sessionId=0x18f94c85d2b0000, negotiated timeout=30000
zookeeper_init sucess!
```



回到 zookeeper 查看确认已经注册成功

```shell
# 通过 ls 命令查看注册成功的 znode 节点
[zk: localhost:2181(CONNECTED) 1] ls /
[zookeeper, FriendServiceRpc]
[zk: localhost:2181(CONNECTED) 2] ls /FriendServiceRpc
[GetFriendsList]
[zk: localhost:2181(CONNECTED) 3] ls /FriendServiceRpc/GetFriendsList
[]

```



##### 启动消费者

启动后可以看到非常多zookeeper 相关日志信息，这里看到他打印了`rpc GetFriendsList sucess!`。并且已经获取到了信息，说明RPC方法调用成功。

```shell
ubuntu@ ~/RPC/bin$ ./consumer -i test.conf
===========================================
header_size:36
rpc_header_str:
FriendServiceRpcGetFriendsList 
service_name:FriendServiceRpc
method_name:GetFriendsList
args_str:
===========================================
[time],292:3969(0x7f5818f52d00):ZOO_INFO@log_env@726: Client environment:zookeeper.version=zookeeper C client 3.4.10
......
[time],300:3969(0x7f5816cd0700):ZOO_INFO@check_events@1775: session establishment complete on server [127.0.0.1:2181], sessionId=0x18f94c85d2b0006, negotiated timeout=30000
zookeeper_init sucess!
[time],305:3969(0x7f5818f52d00):ZOO_INFO@zookeeper_close@2527: Closing zookeeper sessionId=0x18f94c85d2b0006 to [127.0.0.1:2181]

rpc GetFriendsList response success:
index1 name:he peng
index2 name:xie boss
index3 name:kun kun
```

## 项目讲解

#### [01.环境搭建](https://github.com/Make-Hua/RPC/blob/master/explain/01.%E7%8E%AF%E5%A2%83%E6%90%AD%E5%BB%BA.md)

#### [02.MprpcConfig模块](https://github.com/Make-Hua/RPC/blob/master/explain/02.MprpcConfig%E6%A8%A1%E5%9D%97.md)

#### [**03.RpcProvider模块**](https://github.com/Make-Hua/RPC/blob/master/explain/03.RpcProvider%E6%A8%A1%E5%9D%97.md)

#### [**04.MprpcController模块**]()

#### [**05.MprpcChannel模块**]()

#### [**06.Logger模块**]()

#### [**07.ZooKeeper模块**](https://github.com/Make-Hua/RPC/blob/master/explain/07.zookeeper%E6%A8%A1%E5%9D%97.md)
