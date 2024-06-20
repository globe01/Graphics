#pragma once
#ifndef __DIELECTRIC_HPP__
#define __DIELECTRIC_HPP__

#include "Shader.hpp"
#include "Refracted.hpp"




// 加一个Dielectric材质，这个有折射，才能完整实现Whitted-style光线追踪算法
// 照搬另外2个材质的大致结构
namespace WhittedRayTracing
{
    class Dielectric : public Shader
    {
    private:
        float refractiveIndex;// 折射率

        Vec3 diffuseColor;// 漫反射颜色
        Vec3 reflectColor;// 反射颜色
        float reflectEx;// 反射指数
        Vec3 refractColor;// 折射颜色
        

    public:
        Dielectric(Material& material, vector<Texture>& textures);
        
        // virtual Refracted shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const;
        virtual RGB shade(const Vec3& in, const Vec3& out, const Vec3& normal) const;
    };
}


#endif// 这个忘加就报错