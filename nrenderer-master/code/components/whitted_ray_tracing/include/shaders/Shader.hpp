#pragma once
#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include "geometry/vec.hpp"
#include "common/macros.hpp"
#include "scene/Scene.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace NRenderer;
    using namespace std;

    constexpr float PI = 3.1415926535898f;// 圆周率

    // Shader类，用于定义不同类型的着色器
    class Shader
    {
    protected:
        Material& material;// 材质
        vector<Texture>& textureBuffer;// 纹理缓冲区
    public:
        Shader(Material& material, vector<Texture>& textures)
            : material              (material)
            , textureBuffer         (textures)
        {}
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const = 0;
    };
    SHARE(Shader);
}

#endif