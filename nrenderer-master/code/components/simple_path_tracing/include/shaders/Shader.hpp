#pragma once
#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include "geometry/vec.hpp"
#include "common/macros.hpp"
#include "scene/Scene.hpp"

#include "Scattered.hpp"

namespace SimplePathTracer
{
    using namespace NRenderer;
    using namespace std;

    constexpr float PI = 3.1415926535898f;

    class Shader
    {
    protected:
        Material& material;// 材质引用，用于获取材质属性
        vector<Texture>& textureBuffer;// 纹理缓冲区引用，用于获取纹理数据
    public:
        Shader(Material& material, vector<Texture>& textures)
            : material              (material)
            , textureBuffer         (textures)
        {}

        // 纯虚函数shade，计算光线与物体表面相交后的散射行为
        virtual Scattered shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const = 0;
    };
    SHARE(Shader);
}

#endif