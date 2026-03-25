#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "CServer.h"

int main()
{
    // io_context 是跨平台的"异步 I/O 管理器"，用于管理异步 I/O 操作
    // epoll 是 Linux 下的异步 I/O 操作，用于监听多个文件描述符上的 I/O 事件
    try{
        // 监听端口
        unsigned short port = static_cast<unsigned short>(8080);
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
        std::make_shared<CServer>(ioc, port)->Start();
        // 运行 io_context 实例，等待 I/O 事件发生
        ioc.run();
    }catch(std::exception const& e){
        std::cout << "Error:  " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}