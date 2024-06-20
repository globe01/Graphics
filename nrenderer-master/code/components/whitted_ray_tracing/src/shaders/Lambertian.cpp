#include "shaders/Lambertian.hpp"
#include "intersections/HitRecord.hpp"

// 先沿用框架Raycast的代码结构
// Lambertian材质就是漫反射材质，只有漫反射，没有镜面反射，模拟粗糙表面
// Lambertian材质的光照模型是：Lambert定律，即光照强度与入射光线与法向量的夹角的余弦成正比
// 要想实现Whitted-style递归光线追踪算法的话，用Lambertian材质不太行，因为没有镜面反射
namespace WhittedRayTracing
{
    // material是材质，textures是纹理
    Lambertian::Lambertian(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        // 从材质中获取漫反射颜色
        auto optDiffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
        // 如果材质中有漫反射颜色，则使用材质中的漫反射颜色，否则使用默认的白色
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else diffuseColor = {1, 1, 1};

        // 新增

    }
    // in是入射光线，out是出射光线，normal是法向量，shade函数返回漫反射光照强度
    RGB Lambertian::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        return diffuseColor * glm::dot(out, normal);// 返回漫反射光照强度
    }
}