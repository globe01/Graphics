#pragma once
#ifndef __HIT_RECORD_HPP__
#define __HIT_RECORD_HPP__

#include <optional>

#include "geometry/vec.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace NRenderer;
    using namespace std;
    struct HitRecordBase
    {
        float t;// t是光线参数，用于计算击中点
        Vec3 hitPoint;// 击中点
        Vec3 normal;// 法向量
        Handle material;// 材质

        // 新增
        // float refLaction;// 反射
        // float refRaction;// 折射
        // float ior;// 折射率

    };

    // optional 是 C++17 引入的标准库类型，定义在 <optional> 头文件中，表示一个值可能存在或不存在
    // 从而避免使用指针来表示可能不存在的值
    using HitRecord = optional<HitRecordBase>;// 击中记录

    // 获取未击中记录
    inline
    HitRecord getMissRecord() {
        return nullopt;// 未击中就直接返回空
    }

    // 获取击中记录，相交的t值（t就是射线o+td的那个t，击中点，法向量，材质）
    inline
    HitRecord getHitRecord(float t, const Vec3& hitPoint, const Vec3& normal, Handle material) {
        return make_optional<HitRecordBase>(t, hitPoint, normal, material);
    }
}

#endif