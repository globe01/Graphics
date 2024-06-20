#pragma once
#ifndef __HEMI_SPHERE_HPP__
#define __HEMI_SPHERE_HPP__

#include "Sampler3d.hpp"
#include <ctime>

namespace SimplePathTracer
{
    using namespace std;
    class HemiSphere : public Sampler3d
    {
    private:
        constexpr static float C_PI = 3.14159265358979323846264338327950288f;// 定义pai常量
        
        default_random_engine e;// 随机数生成器
        uniform_real_distribution<float> u;// 生成均匀分布的随机数
    
    public:
        HemiSphere()
            : e               ((unsigned int)time(0) + insideSeed())
            , u               (0, 1)
        {}
        // 使用当前时间和一个内部种子函数 insideSeed() 的和来初始化
        // u 被初始化为在 [0, 1) 区间内均匀分布的随机数

        // 重写抽象基类 Sampler3d 的采样方法
        Vec3 sample3d() override {
            float epsilon1 = u(e);// 生成一个均匀分布的随机数
            float epsilon2 = u(e);// 生成一个均匀分布的随机数
            float r = sqrt(1 - epsilon1 * epsilon1);// 计算半径
            float x = cos(2*C_PI*epsilon2) * r;// 通过计算 cos 和 sin 函数，生成在单位圆周上的点
            float y = sin(2*C_PI*epsilon2) * r;
            float z = epsilon1;// 垂直方向的高度直接使用 epsilon1
            return { x, y, z };// 返回生成的三维向量,表示在半球表面上的随机点
        }
    };
}

#endif