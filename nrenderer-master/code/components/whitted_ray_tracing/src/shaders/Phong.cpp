#include "shaders/Phong.hpp"
#include "intersections/HitRecord.hpp"

// 先沿用框架Raycast的代码结构

// Phong材质是Phong光照模型的材质，有漫反射和镜面反射
namespace WhittedRayTracing
{
    // reflect函数计算反射光线
    Vec3 reflect(const Vec3& normal, const Vec3& dir) {
        return dir - 2*glm::dot(dir, normal)*normal;// 反射光线
    }
    // Phong构造函数，初始化材质和纹理
    Phong::Phong(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)// 调用基类Shader的构造函数进行初始化
    {
        using PW = Property::Wrapper;// 使用Property::Wrapper命名空间
        // 从材质中获取漫反射颜色
        auto optDiffuseColor = material.getProperty<PW::RGBType>("diffuseColor");
        // 如果材质中有漫反射颜色，则使用材质中的漫反射颜色，否则使用默认的白色
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else diffuseColor = {1, 1, 1};
        
        // 从材质中获取镜面反射颜色
        auto optSpecularColor = material.getProperty<PW::RGBType>("specularColor");
        // 如果材质中有镜面反射颜色，则使用材质中的镜面反射颜色，否则使用默认的白色
        if (optSpecularColor) specularColor = (*optSpecularColor).value;
        else specularColor = {1, 1, 1};

        // 从材质中获取镜面反射指数，就是Phong光照模型中的n，n越大，镜面反射越集中
        auto optSpecularEx = material.getProperty<PW::FloatType>("specularEx");
        // 如果材质中有镜面反射指数，则使用材质中的镜面反射指数，否则使用默认的1
        if (optSpecularEx) specularEx = (*optSpecularEx).value;
        else specularEx = 1;

        // 新增


    }

    // in是入射光线，out是出射光线，normal是法向量，shade函数返回Phong光照模型的光照强度
    RGB Phong::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        Vec3 v = in;// 视线方向，就是入射光线
        Vec3 r = reflect(normal, out);// 反射光线
        auto diffuse = diffuseColor * glm::dot(out, normal);// 漫反射光照强度
        auto specular = specularColor * fabs(glm::pow(glm::dot(v, r), specularEx));// 镜面反射光照强度
        return diffuse + specular;// 返回Phong光照模型总的光照强度
    }
}