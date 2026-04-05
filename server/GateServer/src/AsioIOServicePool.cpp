#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size)
:_ioServices(size),
_works(size),
_nextIOService(0)
{
    for(std::size_t i = 0;i<size;++i){
        _works[i] = std::unique_ptr<Work>(new Work(_ioServices[i].get_executor()));
    }

    for (std::size_t i = 0; i < size;i++)
    {
        _threads.emplace_back([this, i]()
                                { _ioServices[i].run(); });
    }
}
AsioIOServicePool::~AsioIOServicePool()
{
    std::cout << "AsioIOServicePool destruct" << std::endl;
    Stop();
}

net::io_context& AsioIOServicePool::GetIOService()
{
    auto& service= _ioServices[_nextIOService++];
    // 循环使用 io_context 实例
    // 避免每个连接都创建一个 io_context 实例，导致内存泄漏
    // 同时也可以避免 io_context 实例数量超过硬件并发数，导致性能下降
    if(_nextIOService >= _ioServices.size()){
        _nextIOService = 0;
    }
    return service;
}
void AsioIOServicePool::Stop()
{
    // 因为仅仅执行work.reset并不能让iocontext从run的状态中退出
    // 当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
    for (auto &work : _works)
    {
        // 把服务先停止
        _ioServices[&work - &_works[0]].stop();
        work.reset();
    }

    for (auto &t : _threads)
    {
        t.join();
    }
}