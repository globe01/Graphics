#pragma once
#ifndef __RAY_CAST_HPP__
#define __RAY_CAST_HPP__

#include "scene/Scene.hpp"

#include "Camera.hpp"
#include "intersections/intersections.hpp"

#include "shaders/ShaderCreator.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace NRenderer;
    class WhittedRayTracingRenderer
    {
    private:
        SharedScene spScene;// 共享指针，指向场景对象
        Scene& scene;// 场景的引用，通过spScene初始化
        WhittedRayTracing::Camera camera;// 相机

        vector<SharedShader> shaderPrograms;// 着色器程序
    public:
        // 构造函数，初始化共享场景和相机
        WhittedRayTracingRenderer(SharedScene spScene)
            : spScene               (spScene)
            , scene                 (*spScene)
            , camera                (spScene->camera)
        {}
        ~WhittedRayTracingRenderer() = default;

        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;// 结果是包含像素数据指针、宽度和高度的元组
        RenderResult render();// 渲染函数，返回渲染结果
        void release(const RenderResult& r);// 释放渲染结果

    private:
        RGB gamma(const RGB& rgb);// gamma校正
        RGB trace(const Ray& r, int recursionDepth, float attenuation);// 递归光线追踪，加个递归深度参数来限制递归次数，再加个衰减参数控制衰减
        HitRecord closestHit(const Ray& r);// 找光线击中的最近物体
    };
}

#endif