#pragma once
#ifndef __UNIFORM_IN_CIRCLE_HPP__
#define __UNIFORM_IN_CIRCLE_HPP__

#include "Sampler2d.hpp"

namespace SimplePathTracer
{
    using namespace std;
    // 用于在单位圆内均匀采样的类 UniformInCircle
    class UniformInCircle : public Sampler2d
    {
    private:
        // 随机数生成器和均匀分布
        default_random_engine e;
        uniform_real_distribution<float> u;
    public:
        // 构造函数，初始化随机数生成器和均匀分布
        UniformInCircle()
            : e               ((unsigned int)time(0) + insideSeed())
            , u               (-1, 1)
        {}
        // 重写抽象基类 Sampler2d 的采样方法
        Vec2 sample2d() override {
            float x{0}, y{0};
            do {
                x = u(e);
                y = u(e);
            } while((x*2 + y*2) > 1);// 确保点在单位圆内
            return { x, y };
        }
    
    };
}

#endif