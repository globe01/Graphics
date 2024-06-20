#pragma once
#ifndef __REFRACTED_HPP__
#define __REFRACTED_HPP__

// 还是得再加一个这个文件，仿照Scattered写

#include "Ray.hpp"


namespace WhittedRayTracing
{
    // 用来放Dielectric材质的反射折射的变量
    struct Refracted
    {
        // 反射
        Ray refLectRay = {};
        Vec3 refLect_ratio = {};

        // 折射
        Ray refRectRay = {};
        Vec3 refRect_ratio = {};

    };// 分号

}



#endif