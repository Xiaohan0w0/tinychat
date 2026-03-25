#ifndef SINGLETON_H
#define SINGLETON_H
#include "global.h"

// 懒汉式单例模式，唯一的实例对象，直到第一次获取才产生，线程不安全，需要双重判断，加锁
template<typename T>
class Singleton
{
public:
    static std::shared_ptr<T> GetInstance()
    {
        if (_instance == nullptr) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_instance == nullptr) {
                // 创建一个 shared_ptr 智能指针，指向一个动态分配的 T 类型对象
                _instance = std::shared_ptr<T>(new T());
            }
        }
        return _instance;
    }
    virtual ~Singleton() { std::cout << "单例模式析构" << std::endl; }
    void PrintAddress() { std::cout << _instance.get() << std::endl; }

protected:
    Singleton() = default;
    Singleton(const Singleton<T> &) = delete;
    Singleton<T> &operator=(const Singleton<T> &) = delete;

private:
    static std::shared_ptr<T> _instance;
    static std::mutex _mutex;
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;

template<typename T>
std::mutex Singleton<T>::_mutex;

#endif // SINGLETON_H
