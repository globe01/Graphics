#pragma once
#ifndef __HIT_RECORD_HPP__
#define __HIT_RECORD_HPP__

#include <optional>

#include "geometry/vec.hpp"

namespace SimplePathTracer
{
    using namespace NRenderer;
    using namespace std;
    // 存储光线与物体相交时的信息
    struct HitRecordBase
    {
        float t;// 光线参数t
        Vec3 hitPoint;// 相交点坐标
        Vec3 normal;// 相交点法向量
        Handle material;// 相交点材质
    };
    // 用optional包装HitRecordBase，以便返回miss的情况, 表示光线是否与物体相交,
    // 如果相交，返回HitRecordBase，否则返回nullopt
    using HitRecord = optional<HitRecordBase>;

    // 返回miss的HitRecord
    inline
    HitRecord getMissRecord() {
        return nullopt;
    }

    // 返回相交的HitRecord
    inline
    HitRecord getHitRecord(float t, const Vec3& hitPoint, const Vec3& normal, Handle material) {
        return make_optional<HitRecordBase>(t, hitPoint, normal, material);
    }
}

#endif