#pragma once
#ifndef __SAMPLER_2D_HPP__
#define __SAMPLER_2D_HPP__

#include "Sampler.hpp"

#include <random>
#include "geometry/vec.hpp"

namespace SimplePathTracer
{
    using NRenderer::Vec2;
    class Sampler2d : public Sampler
    {
    public:
        Sampler2d() = default;
        virtual Vec2 sample2d() = 0;// 抽象方法，返回一个二维向量样本
    };
}

#endif