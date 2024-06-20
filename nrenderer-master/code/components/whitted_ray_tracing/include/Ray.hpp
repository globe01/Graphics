#pragma once
#ifndef __RAY_HPP__
#define __RAY_HPP__

#include "geometry/vec.hpp"

#include <limits>

#define FLOAT_INF numeric_limits<float>::infinity()

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace NRenderer;
    using namespace std;


    struct Ray
    {
        Vec3 origin;
        // keep it as a unit vector
        Vec3 direction;

        void setOrigin(const Vec3& v) {
            origin = v;
        }

        void setDirection(const Vec3& v) {
            direction = glm::normalize(v);
        }

        // 获取起点
        Vec3 getOrigin() const {
            return origin;
        }

        // 获取方向
        Vec3 getDirection() const {
            return direction;
        }


        inline
        Vec3 at(float t) const {
            return origin + t*direction;// 射线是o+td，有了t就可以求出射线上的点
        }
        
        // Ray里面的origin是射线的起点，direction是射线的方向
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