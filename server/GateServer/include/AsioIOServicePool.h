#pragma once
#include "Const.h"
#include "Singleton.h"

class AsioIOServicePool:public Singleton<AsioIOServicePool>
{
    friend Singleton<AsioIOServicePool>;
public:
    using IOService = net::io_context;
    using Work = net::executor_work_guard<net::io_context::executor_type>;
    using WorkPtr=std::unique_ptr<Work>;

    ~AsioIOServicePool();
    AsioIOServicePool GetIOServicePool(const AsioIOServicePool&)=delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&)=delete;

    net::io_context& GetIOService();
    void Stop();

private:
    AsioIOServicePool(std::size_t size = 2/*std::thread::hardware_concurrency()*/);
    // 创建 io_context 实例
    std::vector<IOService> _ioServices;
    // 创建 work_guard 实例
    // 每个 work_guard 实例关联一个 io_context 实例，用于管理 io 操作
    // 告诉io_context 我还在工作，事件循环不准退出，只要work_guard存在，io_context 就一直阻塞等待事件发生
    std::vector<WorkPtr> _works;
    // 创建线程
    std::vector<std::thread> _threads;
    // 记录下一个 io_context 实例的索引
    std::size_t _nextIOService;
};
