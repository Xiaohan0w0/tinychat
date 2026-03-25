#pragma once
#include "Const.h"
#include "Singleton.h"
#include "HttpConnection.h"


class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem : public Singleton<LogicSystem>
{
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem() = default;
    // 处理GET请求
    bool HandleGet(std::string url, std::shared_ptr<HttpConnection> connection);
    // 处理POST请求
    bool HandlePost(std::string url, std::shared_ptr<HttpConnection> connection);
    // 注册GET请求处理函数
    void RegGet(std::string url, HttpHandler handler);
    // 注册POST请求处理函数
    void RegPost(std::string url, HttpHandler handler);

private:
    LogicSystem();
    // 注册POST请求处理函数
    std::unordered_map<std::string, HttpHandler> _post_handlers;
    // 注册GET请求处理函数
    std::unordered_map<std::string, HttpHandler> _get_handlers;
};