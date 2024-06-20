#pragma once
#ifndef __CAMERA_HPP__
#define __CAMERA_HPP__

#include "scene/Camera.hpp"
#include "geometry/vec.hpp"

#include "samplers/SamplerInstance.hpp"

#include "Ray.hpp"
// 在路径追踪算法中，摄像机用于生成从视点通过像素的光线，进而计算场景中的光线传播和交互
namespace SimplePathTracer
{
    using namespace std;
    using namespace NRenderer;

    // 摄像机类，用于生成光线
    class Camera
    {
    private:
        const NRenderer::Camera& camera;// 引用基础摄像机对象
        float lenRadius;// 镜头半径
        Vec3 u, v, w;// 摄像机坐标系
        Vec3 vertical;// 摄像机视野垂直方向
        Vec3 horizontal;// 摄像机视野水平方向
        Vec3 lowerLeft;// 摄像机视野左下角
        Vec3 position;// 摄像机位置
    public:
        Camera(const NRenderer::Camera& camera)
            : camera                (camera)
        {
            position = camera.position;// 摄像机位置
            lenRadius = camera.aperture / 2.f;// 半径
            auto vfov = camera.fov;// 垂直视野角度
            vfov = clamp(vfov, 160.f, 20.f);// 视野角度限制在20-160度之间
            auto theta = glm::radians(vfov);// 视野角度转弧度
            auto halfHeight = tan(theta/2.f);// 视野高度的一半
            auto halfWidth = camera.aspect*halfHeight;// 视野宽度的一半
            Vec3 up = camera.up;// 摄像机向上方向
            w = glm::normalize(camera.position - camera.lookAt);// 摄像机观察方向
            u = glm::normalize(glm::cross(up, w));// 摄像机坐标系x轴
            v = glm::cross(w, u);// 摄像机坐标系y轴

            auto focusDis = camera.focusDistance;// 焦距

            // 摄像机视野的左下角
            lowerLeft = position - halfWidth*focusDis*u
                - halfHeight*focusDis*v
                - focusDis*w;
            horizontal = 2*halfWidth*focusDis*u;// 摄像机视野水平方向
            vertical = 2*halfHeight*focusDis*v;// 摄像机视野垂直方向
        }

        // 从摄像机中发射光线
        Ray shoot(float s, float t) const {
            auto r = defaultSamplerInstance<UniformInCircle>().sample2d();// 在单位圆内均匀采样
            float rx = r.x * lenRadius;// x方向采样
            float ry = r.y * lenRadius;// y方向采样
            Vec3 offset = u*rx + v*ry;// 偏移量
            // 返回光线
            return Ray{
                position + offset,
                glm::normalize(
                    lowerLeft + s*horizontal + t*vertical - position - offset
                )
            };
        }
    };
}

#endif