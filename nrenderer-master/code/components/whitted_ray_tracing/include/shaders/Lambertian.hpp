#pragma once
#ifndef __LAMBERTIAN_HPP__
#define __LAMBERTIAN_HPP__

#include "Shader.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    class Lambertian : public Shader
    {
    private:
        Vec3 diffuseColor;// 漫反射颜色
    public:
        Lambertian(Material& material, vector<Texture>& textures);
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const;
    };
}

#endif