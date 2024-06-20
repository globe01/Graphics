#pragma once
#ifndef __PHONG_HPP__
#define __PHONG_HPP__

#include "Shader.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    class Phong : public Shader
    {
    private:
        Vec3 diffuseColor;// 漫反射颜色
        Vec3 specularColor;// 镜面反射颜色
        float specularEx;// 镜面反射指数
    public:
        Phong(Material& material, vector<Texture>& textures);// 构造函数，初始化材质和纹理
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const;// 着色函数，计算Phong光照模型的光照强度
    };
}

#endif