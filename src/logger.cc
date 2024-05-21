#include <iostream>
#include <string>
#include <time.h> 

#include "logger.h"

// 获取日志的单例
Logger& Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    // 启动专门的写日志的线程
    std::thread writeLogTask([&](){
        // 获取当前日期   取出缓冲队列中的日志信息    拼接好后写入对应日志文件中
        for (;;)
        {
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            // 打开文件
            FILE *pf = fopen(file_name, "a+");
            if (nullptr == pf)
            {
                std::cout << "logger file : " << file_name << " open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            
            std::string msg = m_lckQue.Pop();

            // 加入 具体时间信息
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d => [%s]",
                    nowtm->tm_hour,
                    nowtm->tm_min,
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

            // 写入数据
            fputs(msg.c_str(), pf);

            // 关闭文件
            fclose(pf);
        }
    });
    // 分离线程，该线程为 守护线程 专门在后台进行写入 log.txt 的操作
    writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel = level;
}

// 写日志 将日志信息写入 缓存队列 LckQue；
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}