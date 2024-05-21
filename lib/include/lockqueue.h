#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>


template<typename T>
class LockQueue
{
public:

    // 处于分布式环境，多个 worker 线程均有可能同一时间写日志 m_lckQue
    void Push(const T &data)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(data);
        m_condvariable.notify_one();
    }

    T Pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty())
        {
            // 缓存队列为空，线程进入 wait 状态 (两件事 分别为：释放锁 状态变为等待状态)
            m_condvariable.wait(lock);
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }

private:
    // 日志缓冲队列包装队列
    std::queue<T> m_queue;

    // 由于处于分布式环境，则需要考虑该封装队列 LockQueue 的线程安全问题
    // （存在多个线程对消息队列进行操作）
    std::mutex m_mutex;

    // 保证当 LockQueue is Empty 时，我们需要让消息队列往 log.txt 文件的线程不去竞争锁，
    // 保证分布式环境的效率，需要用到条件变量进行通信
    std::condition_variable m_condvariable;
};