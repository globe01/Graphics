#pragma once
#ifndef __SAMPLER_INSTANCE_HPP__
#define __SAMPLER_INSTANCE_HPP__

#include "HemiSphere.hpp"
#include "Marsaglia.hpp"
#include "UniformSampler.hpp"
#include "UniformInCircle.hpp"
#include "UniformInSquare.hpp"

namespace SimplePathTracer
{
    // 模板函数，返回采样器的单例实例
    template<typename T>
    T& defaultSamplerInstance() {
        // 编译时检查模板参数是否为合法的采样器类型
        static_assert(
            is_base_of<Sampler1d, T>::value ||
            is_base_of<Sampler2d, T>::value ||
            is_base_of<Sampler3d, T>::value, "Not a sampler type.");
        thread_local static T t{};// 线程局部存储，确保每个线程都有自己的采样器实例
        return t;
    }
}

#endif