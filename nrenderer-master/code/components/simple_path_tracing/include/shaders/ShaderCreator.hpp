#pragma once
#ifndef __SHADER_CREATOR_HPP__
#define __SHADER_CREATOR_HPP__

#include "Shader.hpp"
#include "Lambertian.hpp"

namespace SimplePathTracer
{
    // 着色器创建器类，用于创建不同类型的着色器实例
    class ShaderCreator
    {
    public:
        ShaderCreator() = default;// 默认构造函数
        // 创建着色器方法，根据材质类型创建相应的着色器实例
        SharedShader create(Material& material, vector<Texture>& t) {
            SharedShader shader{nullptr};
            switch (material.type)
            {
            case 0:// 如果材质类型为 0，则创建 Lambertian 着色器
                shader = make_shared<Lambertian>(material, t);
                break;
            default:// 默认也是创建 Lambertian 着色器
                shader = make_shared<Lambertian>(material, t);
                break;
            }
            return shader;
        }
    };
}

#endif