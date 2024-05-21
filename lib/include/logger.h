#pragma once

#include "lockqueue.h"


/*
* @ INFO        普通信息
* @ ERROR       错误信息
*/
enum LogLevel
{
    INFO,
    ERROR,
};

// Mprpc 框架提供的日志系统  (单例模式)
class Logger
{
public:
    
    // 获取日志的单例
    static Logger& GetInstance();

    // 设置日志级别
    void SetLogLevel(LogLevel level);

    // 写日志
    void Log(std::string msg);

private:

    // 记录日志级别
    int m_loglevel;

    // 日志缓冲队列
    LockQueue<std::string> m_lckQue;

    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
};

// 定义宏  以便使用
#define LOG_INFO(logmsgformat, ...)                         \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::GetInstance();             \
        logger.SetLogLevel(INFO);                           \
        char c[1024] = {0};                                 \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);     \
        logger.Log(c);                                      \
    } while (0);

#define LOG_ERR(logmsgformat, ...)                          \
    do                                                      \
    {                                                       \
        Logger &logger = Logger::GetInstance();             \
        logger.SetLogLevel(ERROR);                          \
        char c[1024] = {0};                                 \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__);     \
        logger.Log(c);                                      \
    } while (0);

    