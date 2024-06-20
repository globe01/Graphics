#pragma once
#ifndef __RAY_HPP__
#define __RAY_HPP__

#include "geometry/vec.hpp"

#include <limits>

#define FLOAT_INF numeric_limits<float>::infinity()
namespace SimplePathTracer
{
    using namespace NRenderer;
    using namespace std;


    struct Ray
    {
        Vec3 origin;// 光线的起点
        // keep it as a unit vector
        Vec3 direction;// 光线的方向

        void setOrigin(const Vec3& v) {
            origin = v;// 光线的起点
        }

        void setDirection(const Vec3& v) {
            direction = glm::normalize(v);// 光线的方向
        }

        inline
        Vec3 at(float t) const {
            return origin + t*direction;// 计算光线上某一点t的坐标
        }

        Ray(const Vec3& origin, const Vec3& direction)
            : origin                (origin)
            , direction             (direction)
        {}
    
        Ray()
            : origin        {}
            , direction     {}
        {}
    };
}

#endif