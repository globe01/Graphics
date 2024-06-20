#include "shaders/Lambertian.hpp"
#include "samplers/SamplerInstance.hpp"

#include "Onb.hpp"

namespace SimplePathTracer
{
    // 计算朗伯反射的着色模型
    // Lambertian 构造函数
    Lambertian::Lambertian(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        // 获取材质的漫反射颜色属性
        auto diffuseColor = material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
        if (diffuseColor) albedo = (*diffuseColor).value;
        else albedo = {1, 1, 1};
    }

    // 实现 shade 方法
    Scattered Lambertian::shade(const Ray& ray, const Vec3& hitPoint, const Vec3& normal) const {
        Vec3 origin = hitPoint;// 设置光线起点
        Vec3 random = defaultSamplerInstance<HemiSphere>().sample3d();// 生成一个在半球上的随机散射方向
        // if (normal == Vec3{0, 0, 1}) {
        //     direction = random;
        // }
        // else if (normal == Vec3{0, 0, -1}) {
        //     direction = -random;
        // }
        // else {
        //     Vec3 z{0, 0, 1};
        //     float angle = -acos(glm::dot(z, normal));
        //     Vec3 axis =  glm::cross(normal, z);
        //     Mat4x4 rotate = glm::rotate(Mat4x4{1}, angle, axis);
        //     direction = rotate*Vec4{random, 1};
        // }
        // direction = glm::normalize(direction);

        // 使用 Onb 类将随机方向转换到世界坐标系
        Onb onb{normal};
        Vec3 direction = glm::normalize(onb.local(random));

        float pdf = 1/(2*PI);// 计算概率密度函数

        auto attenuation = albedo / PI;// 计算光线的衰减

        // 返回 Scattered 对象
        return {
            Ray{origin, direction},
            attenuation,
            Vec3{0},
            pdf
        };
    }
}