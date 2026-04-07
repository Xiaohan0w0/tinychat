#include "ConfigMgr.h"
#include "CServer.h"
#include "RedisMgr.h"
#include "Const.h"

void TestRedis()
{
    // 连接redis 需要启动才可以进行连接
    // redis默认监听端口为6380 可以再配置文件中修改
    redisContext *c = redisConnect("127.0.0.1", 6380);
    if (c == NULL || c->err)
    {
        printf("Connect to redisServer faile:%s\n", c ? c->errstr : "NULL context");
        if (c) redisFree(c);
        return;
    }
    printf("Connect to redisServer Success\n");

    std::string redis_password = "123456";
    redisReply *r = (redisReply *)redisCommand(c, "AUTH %s", redis_password.c_str());
    if (r == NULL)
    {
        printf("Redis认证命令执行失败！\n");
        redisFree(c);
        return;
    }
    if (r->type == REDIS_REPLY_ERROR)
    {
        printf("Redis认证失败！\n");
    }
    else
    {
        printf("Redis认证成功！\n");
    }
    freeReplyObject(r);

    // 为redis设置key
    const char *command1 = "set stest1 value1";

    // 执行redis命令行
    r = (redisReply *)redisCommand(c, command1);

    // 如果返回NULL则说明执行失败
    if (NULL == r)
    {
        printf("Execut command1 failure\n");
        redisFree(c);
        return;
    }

    // 如果执行失败则释放连接
    if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
    {
        printf("Failed to execute command[%s]\n", command1);
        freeReplyObject(r);
        redisFree(c);
        return;
    }

    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command1);

    const char *command2 = "strlen stest1";
    r = (redisReply *)redisCommand(c, command2);

    // 如果返回类型不是整形 则释放连接
    if (r->type != REDIS_REPLY_INTEGER)
    {
        printf("Failed to execute command[%s]\n", command2);
        freeReplyObject(r);
        redisFree(c);
        return;
    }

    // 获取字符串长度
    int length = r->integer;
    freeReplyObject(r);
    printf("The length of 'stest1' is %d.\n", length);
    printf("Succeed to execute command[%s]\n", command2);

    // 获取redis键值对信息
    const char *command3 = "get stest1";
    r = (redisReply *)redisCommand(c, command3);
    if (r->type != REDIS_REPLY_STRING)
    {
        printf("Failed to execute command[%s]\n", command3);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    printf("The value of 'stest1' is %s\n", r->str);
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command3);

    const char *command4 = "get stest2";
    r = (redisReply *)redisCommand(c, command4);
    if (r->type != REDIS_REPLY_NIL)
    {
        printf("Failed to execute command[%s]\n", command4);
        freeReplyObject(r);
        redisFree(c);
        return;
    }
    freeReplyObject(r);
    printf("Succeed to execute command[%s]\n", command4);

    // 释放连接资源
    redisFree(c);
}

void TestRedisMgr()
{
    assert(RedisMgr::GetInstance()->Set("blogwebsite", "llfc.club"));
    std::string value = "";
    assert(RedisMgr::GetInstance()->Get("blogwebsite", value));
    assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
    assert(RedisMgr::GetInstance()->HSet("bloginfo", "blogwebsite", "llfc.club"));
    assert(RedisMgr::GetInstance()->HGet("bloginfo", "blogwebsite") != "");
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->Del("bloginfo"));
    assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
    assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
    assert(RedisMgr::GetInstance()->LPop("lpushkey2", value) == false);
    RedisMgr::GetInstance()->Close();
}

int main()
{
    // TestRedis();
    TestRedisMgr();

    // 获取 ConfigMgr 实例
    ConfigMgr& gCfgMgr = ConfigMgr::GetInstance();
    // 从配置文件中获取 GateServer 端口
    // 并将其转换为 unsigned short 类型
    std::string gate_root_str = gCfgMgr["GateServer"]["Port"];
    unsigned short gate_port = atoi(gate_root_str.c_str());

    // io_context 是跨平台的"异步 I/O 管理器"，用于管理异步 I/O 操作
    // epoll 是 Linux 下的异步 I/O 操作，用于监听多个文件描述符上的 I/O 事件
    try{
        // 监听 gate_port 端口
        // 创建 io_context 实例
        net::io_context ioc{1};
        // 创建信号集，用于监听 SIGINT 和 SIGTERM 信号
        boost::asio::signal_set signals{ioc,SIGINT,SIGTERM};
        // 异步等待信号集中的信号
        // 当收到 SIGINT 或 SIGTERM 信号时，会调用 lambda 表达式中的函数
        // 该函数会停止 io_context 实例，导致 io_context.run() 函数返回
        // 从而退出程序
        signals.async_wait([&ioc](const boost::system::error_code& error,int  signal_number){
            if(error){
                return;
            }
            ioc.stop();
        });
        // 创建 CServer 实例
        // 并启动监听连接
        std::make_shared<CServer>(ioc, gate_port)->Start();
        std::cout<<"GateServer listen on port: "<<gate_port<<std::endl;
        // 运行 io_context 实例，等待 I/O 事件发生
        ioc.run();
    }catch(std::exception const& e){
        std::cout << "Error:  " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}