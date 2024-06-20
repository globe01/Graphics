#pragma once
#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "scene/Camera.hpp"
#include "geometry/vec.hpp"

#include "Ray.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace std;
    using namespace NRenderer;
    class Camera
    {
    private:
        const NRenderer::Camera& camera;// 引用 NRenderer 框架中的 Camera 对象
        float lenRadius;// 镜头半径
        Vec3 u, v, w;// 摄像机的三个基向量
        Vec3 vertical;// 垂直方向向量
        Vec3 horizontal;// 水平方向向量
        Vec3 lowerLeft;// 左下角
        Vec3 position;// 位置
    public:
        // 构造函数，初始化 Camera 对象
        Camera(const NRenderer::Camera& camera)
            : camera                (camera)
        {
            position = camera.position;// 初始化摄像机位置
            lenRadius = camera.aperture / 2.f;// 计算镜头半径
            auto vfov = camera.fov;// 垂直视场角
            vfov = clamp(vfov, 160.f, 20.f);// 将视场角限制在 [20, 160] 之间
            auto theta = glm::radians(vfov);// 角度转弧度
            auto halfHeight = tan(theta/2.f);// 计算视角高度的一半
            auto halfWidth = camera.aspect*halfHeight;// 计算视角宽度的一半
            Vec3 up = camera.up;// 摄像机的上方向向量
            w = glm::normalize(camera.position - camera.lookAt);// 摄像机的观察方向向量
            u = glm::normalize(glm::cross(up, w));// 摄像机的右方向向量
            v = glm::cross(w, u);// 摄像机的上方向向量

            auto focusDis = camera.focusDistance;// 获取焦距

            // 计算视角左下角位置
            lowerLeft = position - halfWidth*focusDis*u
                - halfHeight*focusDis*v
                - focusDis*w;
            horizontal = 2*halfWidth*focusDis*u;// 计算视角水平宽度向量
            vertical = 2*halfHeight*focusDis*v;// 计算视角垂直高度向量
        }

        // 从摄像机中发射光线
        Ray shoot(float s, float t) const {
            // Ray的两个参数为摄像机位置和摄像机发射方向
            return Ray{
                position,
                glm::normalize(
                    lowerLeft + s*horizontal + t*vertical - position
                )
            };
        }
    };
}

#endif