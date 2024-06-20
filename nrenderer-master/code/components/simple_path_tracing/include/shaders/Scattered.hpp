#pragma once
#ifndef __SCATTERED_HPP__
#define __SCATTERED_HPP__

#include "Ray.hpp"

namespace SimplePathTracer
{
    // 用于存储光线散射信息的结构体
    struct Scattered
    {
        Ray ray = {};// 散射后的光线
        Vec3 attenuation = {};// 散射后的衰减
        Vec3 emitted = {};// 散射后的发射光
        float pdf = {0.f};// 概率密度函数
    };
    
}

#endif