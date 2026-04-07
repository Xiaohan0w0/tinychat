#pragma once
#include "Const.h"
#include "Singleton.h"

class RedisConPool
{
public:
    RedisConPool(size_t poolSize, const char *host, int port, const char *pwd);
    ~RedisConPool();
    void ClearConnections();
    redisContext *getConnection();
    redisContext *getConNonBlock();
    void returnConnection(redisContext *context);
    void Close();

private:
    bool reconnect();
    void checkThreadPro();
    void checkThread();

    // 是否已经停止服务
    std::atomic<bool> b_stop_;
    size_t poolSize_;
    const char *host_;
    const char *pwd_;
    int port_;
    
    /* Context for a connection to Redis */
    std::queue<redisContext *> connections_;
    std::atomic<int> fail_count_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::thread check_thread_;
    int counter_;
};