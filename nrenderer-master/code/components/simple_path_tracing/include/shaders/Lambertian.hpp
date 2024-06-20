#pragma once
#ifndef __LAMBERTIAN_HPP__
#define __LAMBERTIAN_HPP__

#include "Shader.hpp"


// Lambertian 类继承自 Shader 类，实现了朗伯（Lambertian）反射模型--理想散射
// 这种反射模型假设表面对光线的反射是均匀的，即在所有方向上的反射强度都是相同的。
// 非常适合用于模拟漫反射表面，如磨砂玻璃、瓷砖等。
namespace SimplePathTracer
{
    class Lambertian : public Shader
    {
    private:
        Vec3 albedo;// 材质的反射率，用于计算反射光的强度
    public:
        Lambertian(Material& material, vector<Texture>& textures);
        Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;// 用于计算光线在相交点处的散射情况
    };
}

#endif