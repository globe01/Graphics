#pragma once
#ifndef __SAMPLER_HPP__
#define __SAMPLER_HPP__

#include <mutex>

namespace SimplePathTracer
{
    using std::mutex;
    // 抽象基类 Sampler
    class Sampler
    {
    protected:
        // 生成唯一种子的静态方法
        static int insideSeed() {
            static mutex m;// 互斥锁
            static int seed = 0;// 种子
            m.lock();// 加锁
            seed++;// 种子自增
            m.unlock();// 解锁
            return seed;// 返回种子
        }
    public:
        virtual ~Sampler() = default;// 虚析构函数
        Sampler() = default;// 默认构造函数
    };
}

#endif