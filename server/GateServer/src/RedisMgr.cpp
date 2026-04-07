#include "RedisMgr.h"

RedisMgr::RedisMgr()
{

}
RedisMgr::~RedisMgr()
{
}

// 连接Redis服务器
bool RedisMgr::Connect(const std::string &host, int port)
{
    this->_connect = redisConnect(host.c_str(), port);
    if(this->_connect == nullptr)
    {
        return false;
    }
    if (this->_connect != nullptr && this->_connect->err)
    {
        std::cout << "connect error " << this->_connect->errstr << std::endl;
        return false;
    }
    return true;
}

// 获取Redis中key对应的value
bool RedisMgr::Get(const std::string &key, std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "GET %s", key.c_str());
    if(this->_reply == nullptr)
    {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_STRING)
    {
        std::cout << "[ GET  " << key << " ] failed" << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    value = this->_reply->str;
    freeReplyObject(this->_reply);
    std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
    return true;
}

// 设置Redis中key对应的value值
bool RedisMgr::Set(const std::string &key, const std::string &value)
{
    // 执行redis命令行
    this->_reply = (redisReply *)redisCommand(this->_connect, "SET %s %s", key.c_str(), value.c_str());
    // 如果返回NULL则说明执行失败
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    // 如果执行失败则释放连接
    if (!(this->_reply->type == REDIS_REPLY_STATUS && (strcmp(this->_reply->str, "OK") == 0 || strcmp(this->_reply->str, "ok") == 0)))
    {
        std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(this->_reply);
    std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
    return true;
}

// Redis认证密码
bool RedisMgr::Auth(const std::string &password)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "AUTH %s", password.c_str());
    if (this->_reply->type == REDIS_REPLY_ERROR)
    {
        std::cout << "认证失败" << std::endl;
        // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(this->_reply);
        return false;
    }
    else
    {
        // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
        freeReplyObject(this->_reply);
        std::cout << "认证成功" << std::endl;
        return true;
    }
}

// 双端队列操作
// 左侧push
bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "LPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0)
    {
        std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 左侧pop
bool RedisMgr::LPop(const std::string &key, std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "LPOP %s ", key.c_str());
    if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    value = _reply->str;
    std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 右侧push
bool RedisMgr::RPush(const std::string &key, const std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "RPUSH %s %s", key.c_str(), value.c_str());
    if (NULL == this->_reply)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    if (this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer <= 0)
    {
        std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 右侧pop
bool RedisMgr::RPop(const std::string &key, std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "RPOP %s ", key.c_str());
    if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL)
    {
        std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    value = _reply->str;
    std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 设置二级key(field)对应的value 方便处理字符串
bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
    if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 设置二级key(field)对应的value 方便处理二进制数据
bool RedisMgr::HSet(const char *key, const char *hkey, const char *hvalue, size_t hvaluelen)
{
    const char *argv[4];
    size_t argvlen[4];
    argv[0] = "HSET";
    argvlen[0] = 4;
    argv[1] = key;
    argvlen[1] = strlen(key);
    argv[2] = hkey;
    argvlen[2] = strlen(hkey);
    argv[3] = hvalue;
    argvlen[3] = hvaluelen;
    this->_reply = (redisReply *)redisCommandArgv(this->_connect, 4, argv, argvlen);
    if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 获取二级key(field)对应的value
std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
    const char *argv[3];
    size_t argvlen[3];
    argv[0] = "HGET";
    argvlen[0] = 4;
    argv[1] = key.c_str();
    argvlen[1] = key.length();
    argv[2] = hkey.c_str();
    argvlen[2] = hkey.length();
    this->_reply = (redisReply *)redisCommandArgv(this->_connect, 3, argv, argvlen);
    if (this->_reply == nullptr || this->_reply->type == REDIS_REPLY_NIL)
    {
        freeReplyObject(this->_reply);
        std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
        return "";
    }
    std::string value = this->_reply->str;
    freeReplyObject(this->_reply);
    std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
    return value;
}

// 删除Redis中某个key
bool RedisMgr::Del(const std::string &key)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "DEL %s", key.c_str());
    if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER)
    {
        std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 判断Redis中key是否存在
bool RedisMgr::ExistsKey(const std::string &key)
{
    this->_reply = (redisReply *)redisCommand(this->_connect, "exists %s", key.c_str());
    if (this->_reply == nullptr || this->_reply->type != REDIS_REPLY_INTEGER || this->_reply->integer == 0)
    {
        std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
        freeReplyObject(this->_reply);
        return false;
    }
    std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
    freeReplyObject(this->_reply);
    return true;
}

// 关闭Redis连接
void RedisMgr::Close()
{
    redisFree(this->_connect);
}


