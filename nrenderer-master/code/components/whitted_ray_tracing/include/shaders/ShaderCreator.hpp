#pragma once
#ifndef __SHADER_CREATOR_HPP__
#define __SHADER_CREATOR_HPP__

#include "Shader.hpp"
#include "Lambertian.hpp"
#include "Phong.hpp"
#include "Dielectric.hpp"// 新增Dielectric材质如玻璃

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    class ShaderCreator
    {
    public:
        ShaderCreator() = default;
        SharedShader create(Material& material, vector<Texture>& t) {
            SharedShader shader{nullptr};
            switch (material.type)
            {
            case 0:
                shader = make_shared<Lambertian>(material, t);
                break;
            case 1:
                shader = make_shared<Phong>(material, t);
                break;
            case 2:// 新增Dielectric材质
                shader = make_shared<Dielectric>(material, t);
                break;
            default:
                shader = make_shared<Lambertian>(material, t);
                break;
            }
            return shader;
        }
    };
}

#endif