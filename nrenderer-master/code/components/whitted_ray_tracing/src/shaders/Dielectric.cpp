#include "shaders/Dielectric.hpp"
#include "intersections/HitRecord.hpp"

#include "shaders/Refracted.hpp"

namespace WhittedRayTracing
{
    // 反射的在Phong里面有，不重复了


    // 计算折射光线，直接找公式和GPT，之前忘了全反射了，看起来应该是对的
    // Vec3 refract(const Vec3& normal, const Vec3& dir, float refractiveIndex) {
    //     float cosTheta = glm::dot(-dir, normal); // cos(θ)
    //     float sinTheta2 = 1.0f - cosTheta * cosTheta; // sin^2(θ)
    //     float sinPhi2 = refractiveIndex * refractiveIndex * sinTheta2; // sin^2(φ)
        
    //     // 全反射情况
    //     if (sinPhi2 > 1.0f) {
    //         return Vec3(0.0f); // 全反射，返回零向量或处理反射情况
    //     }
        
    //     float cosPhi = glm::sqrt(1.0f - sinPhi2); // cos(φ)
    //     return refractiveIndex * dir + (refractiveIndex * cosTheta - cosPhi) * normal; // 折射光线
    // }
    
    // 计算折射，直接找公式和GPT，之前忘了全反射了，看起来应该是对的
    Vec3 refract(const Vec3& I, const Vec3& N, float etaI, float etaT) {
        float cosi = glm::clamp(glm::dot(-I, N), -1.0f, 1.0f); // 入射角的余弦值
        float eta = etaI / etaT; // 折射率之比
        float k = 1.0f - eta * eta * (1.0f - cosi * cosi); // sin^2(θ_t)
        if (k < 0.0f) {
            return Vec3(0); // 全反射
        } else {
            return eta * I + (eta * cosi - sqrtf(k)) * N; // 折射方向的公式
        }
    }

    // material是材质，textures是纹理
    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        using PW = Property::Wrapper;// 使用Property::Wrapper命名空间
        // 从材质中获取折射率
        auto optRefractiveIndex = material.getProperty<PW::FloatType>("refractiveIndex");
        // 如果材质中有折射率，则使用材质中的折射率，不然就直接玻璃的1.5，感觉直接1.5也行
        if (optRefractiveIndex) refractiveIndex = (*optRefractiveIndex).value;
        else refractiveIndex = 1.5;

        // 从材质中获取漫反射颜色
        auto optDiffuseColor = material.getProperty<PW::RGBType>("diffuseColor");
        // 如果材质中有漫反射颜色，则使用材质中的漫反射颜色，否则使用默认的白色
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else diffuseColor = {1, 1, 1};

        // 从材质中获取反射颜色
        auto optReflectColor = material.getProperty<PW::RGBType>("reflectColor");
        // 如果材质中有反射颜色，则使用材质中的反射颜色，否则使用默认的白色
        if (optReflectColor) reflectColor = (*optReflectColor).value;
        else reflectColor = {1, 1, 1};

        // 从材质中获取反射指数
        auto optReflectEx = material.getProperty<PW::FloatType>("reflectEx");
        // 如果材质中有反射指数，则使用材质中的反射指数，否则使用默认的1
        if (optReflectEx) reflectEx = (*optReflectEx).value;
        else reflectEx = 1;

        // 从材质中获取折射颜色
        auto optRefractColor = material.getProperty<PW::RGBType>("refractColor");
        // 如果材质中有折射颜色，则使用材质中的折射颜色，否则使用默认的白色
        if (optRefractColor) refractColor = (*optRefractColor).value;
        else refractColor = {1, 1, 1};


    }
    // shade函数返回折射光照强度，折射也交给GPT了
    // in是入射光线，out是出射光线，normal是法向量
    RGB Dielectric::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        Vec3 v = in;// 视线方向，就是入射光线
        Vec3 r = reflect(normal, out);// 反射光线
        Vec3 t = refract(normal, out, refractiveIndex);// 折射光线
        auto diffuse = diffuseColor * glm::dot(out, normal);// 漫反射光照强度
        auto reflect = reflectColor * fabs(glm::pow(glm::dot(v, r), reflectEx));// 反射光照强度
        auto refract = refractColor * fabs(glm::pow(glm::dot(v, t), reflectEx));// 折射光照强度
        return diffuse + reflect + refract;// 返回Phong光照模型总的光照强度
    }
    // 仿照Phong，这个好像没效果

    // 折射也交给GPT了
    // Refracted shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const{
    //     Vec3 in = glm::normalize(ray.direction);
    //     Vec3 n = normal;
    //     float etaI = 1.0f;
    //     float etaT = refractiveIndex;

    //     // 确定入射和折射的折射率
    //     if (glm::dot(in, n) > 0) {
    //         n = -n;
    //         std::swap(etaI, etaT);
    //     }

    //     // 计算反射方向
    //     Vec3 reflectDir = reflect(in, n);

    //     // 计算折射方向
    //     Vec3 refractDir = refract(in, n, etaI, etaT);

    //     // 使用 Schlick 近似计算反射率，公式为
    //     float cosi = glm::clamp(glm::dot(in, n), -1.0f, 1.0f);
    //     float R0 = (etaI - etaT) / (etaI + etaT);
    //     R0 = R0 * R0;
    //     float reflectance = R0 + (1 - R0) * pow((1 - cosi), 5);

    //     Refracted refracted;
    //     refracted.reflectRay.origin = hitPoint;
    //     refracted.reflectRay.direction = reflectDir;
    //     refracted.reflectAttenuation = reflectColor;

    //     refracted.refractRay.origin = hitPoint;
    //     refracted.refractRay.direction = refractDir;
    //     refracted.refractAttenuation = refractColor;

    //     return refracted;


    // }


}