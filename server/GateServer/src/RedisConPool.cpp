#include "RedisConPool.h"

RedisConPool::RedisConPool(size_t poolSize, const char *host, int port, const char *pwd)
    : poolSize_(poolSize), host_(host), port_(port), b_stop_(false)
{
    for (size_t i = 0; i < poolSize_; ++i)
    {
        auto *context = redisConnect(host, port);
        if (context == nullptr || context->err != 0)
        {
            if (context != nullptr)
            {
                redisFree(context);
            }
            continue;
        }

        auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd);
        if (reply == nullptr || reply->type == REDIS_REPLY_ERROR)
        {
            std::cout << "认证失败" << std::endl;
            freeReplyObject(reply);
            continue;
        }

        freeReplyObject(reply);
        std::cout << "认证成功" << std::endl;
        connections_.push(context);
    }

    check_thread_ = std::thread([this]()
                                {
			while (!b_stop_) {
				counter_++;
				if (counter_ >= 60) {
					checkThreadPro();
					counter_ = 0;
				}

				std::this_thread::sleep_for(std::chrono::seconds(1));
			} });
}

RedisConPool::~RedisConPool()
{
    Close();
}

void RedisConPool::ClearConnections()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty())
    {
        auto context = connections_.front();
        connections_.pop();
        redisFree(context);
    }
}

// 阻塞获取连接，没连接就等待，有连接就返回
redisContext *RedisConPool::getConnection()
{
    // 1. 加互斥锁：保证多线程下操作连接池的线程安全
    // unique_lock 比 lock_guard 更灵活，支持等待条件变量时自动解锁/重新加锁
    std::unique_lock<std::mutex> lock(mutex_);

    // 2. 条件变量等待：阻塞线程，直到满足条件才继续执行 返回true时停止等待，继续往下跑，返回false时阻塞等待
    cond_.wait(lock, [this]
            { 
        // Lambda表达式：返回true时停止等待，返回false时继续阻塞
        if (b_stop_) {
            // 条件1：如果连接池已停止，直接唤醒线程
            return true;
        }
        // 条件2：如果连接池非空（有可用连接），唤醒线程
        return !connections_.empty(); });

    // 3. 被唤醒后，先判断是否是停止信号
    // 如果是停止状态，直接返回空指针，不获取连接
    if (b_stop_)
    {
        return nullptr;
    }

    // 4. 从连接池队列头部取出一个Redis连接
    auto *context = connections_.front();
    // 5. 将该连接从连接池队列中移除（表示已被占用）
    connections_.pop();

    // 6. 返回获取到的Redis连接句柄
    return context;
}

// 非阻塞获取连接，有连接就返回，没有连接就返回空指针
redisContext *RedisConPool::getConNonBlock()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (b_stop_)
    {
        return nullptr;
    }
    if (connections_.empty())
    {
        return nullptr;
    }
    auto *context = connections_.front();
    connections_.pop();
    return context;
}

// 归还连接到连接池
void RedisConPool::returnConnection(redisContext *context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    connections_.push(context);
    cond_.notify_one();
}

void RedisConPool::Close()
{
    // 防止重复关闭
    if (b_stop_)
    {
        return;
    }

    b_stop_ = true;
    cond_.notify_all();

    // 等待检查线程结束
    if (check_thread_.joinable())
    {
        check_thread_.join();
    }

    // 清空连接池
    ClearConnections();
}

bool RedisConPool::reconnect()
{
    auto context = redisConnect(host_, port_);
    if (context == nullptr || context->err != 0)
    {
        if (context != nullptr)
        {
            redisFree(context);
        }
        return false;
    }

    auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd_);
    if (reply->type == REDIS_REPLY_ERROR)
    {
        std::cout << "认证失败" << std::endl;
        // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(reply);
        redisFree(context);
        return false;
    }

    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "认证成功" << std::endl;
    returnConnection(context);
    return true;
}

void RedisConPool::checkThreadPro()
{
    size_t pool_size;
    {
        // 先拿到当前连接数
        std::lock_guard<std::mutex> lock(mutex_);
        pool_size = connections_.size();
    }

    for (int i = 0; i < pool_size && !b_stop_; ++i)
    {
        redisContext *ctx = nullptr;
        // 1) 取出一个连接(持有锁)
        bool bsuccess = false;
        auto *context = getConNonBlock();
        if (context == nullptr)
        {
            break;
        }

        redisReply *reply = nullptr;
        try
        {
            reply = (redisReply *)redisCommand(context, "PING");
            // 2. 先看底层 I/O／协议层有没有错
            if (context->err)
            {
                std::cout << "Connection error: " << context->err << std::endl;
                if (reply)
                {
                    freeReplyObject(reply);
                }
                redisFree(context);
                fail_count_++;
                continue;
            }

            // 3. 再看 Redis 自身返回的是不是 ERROR
            if (!reply || reply->type == REDIS_REPLY_ERROR)
            {
                std::cout << "reply is null, redis ping failed: " << std::endl;
                if (reply)
                {
                    freeReplyObject(reply);
                }
                redisFree(context);
                fail_count_++;
                continue;
            }
            // 4. 如果都没问题，则还回去
            // std::cout << "connection alive" << std::endl;
            freeReplyObject(reply);
            returnConnection(context);
        }
        catch (std::exception &exp)
        {
            if (reply)
            {
                freeReplyObject(reply);
            }

            redisFree(context);
            fail_count_++;
        }
    }

    // 执行重连操作
    while (fail_count_ > 0)
    {
        auto res = reconnect();
        if (res)
        {
            fail_count_--;
        }
        else
        {
            // 留给下次再重试
            break;
        }
    }
}

void RedisConPool::checkThread()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_)
    {
        return;
    }
    auto pool_size = connections_.size();
    for (int i = 0; i < pool_size && !b_stop_; i++)
    {
        auto *context = connections_.front();
        connections_.pop();
        try
        {
            auto reply = (redisReply *)redisCommand(context, "PING");
            if (!reply)
            {
                std::cout << "reply is null, redis ping failed: " << std::endl;
                connections_.push(context);
                continue;
            }
            freeReplyObject(reply);
            connections_.push(context);
        }
        catch (std::exception &exp)
        {
            std::cout << "Error keeping connection alive: " << exp.what() << std::endl;
            redisFree(context);
            context = redisConnect(host_, port_);
            if (context == nullptr || context->err != 0)
            {
                if (context != nullptr)
                {
                    redisFree(context);
                }
                continue;
            }

            auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd_);
            if (reply->type == REDIS_REPLY_ERROR)
            {
                std::cout << "认证失败" << std::endl;
                // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
                freeReplyObject(reply);
                continue;
            }

            // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
            freeReplyObject(reply);
            std::cout << "认证成功" << std::endl;
            connections_.push(context);
        }
    }
}
