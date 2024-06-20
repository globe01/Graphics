#pragma once
#ifndef __SIMPLE_PATH_TRACER_HPP__
#define __SIMPLE_PATH_TRACER_HPP__

#include "scene/Scene.hpp"
#include "Ray.hpp"
#include "Camera.hpp"
#include "intersections/HitRecord.hpp"

#include "shaders/ShaderCreator.hpp"

#include <tuple>
namespace SimplePathTracer
{
    using namespace NRenderer;
    using namespace std;

    // 简单路径追踪渲染器
    class SimplePathTracerRenderer
    {
    public:
    private:
        SharedScene spScene;// 共享场景指针
        Scene& scene;// 场景引用

        unsigned int width;// 渲染宽度
        unsigned int height;// 渲染高度
        unsigned int depth;// 最大追踪深度
        unsigned int samples;// 每像素采样次数

        using SCam = SimplePathTracer::Camera;
        SCam camera;// 路径追踪相机

        vector<SharedShader> shaderPrograms;// 着色器程序列表
    public:
        // 构造函数，初始化场景、摄像机和渲染参数
        SimplePathTracerRenderer(SharedScene spScene)
            : spScene               (spScene)
            , scene                 (*spScene)
            , camera                (spScene->camera)
        {
            width = scene.renderOption.width;
            height = scene.renderOption.height;
            depth = scene.renderOption.depth;
            samples = scene.renderOption.samplesPerPixel;
        }
        ~SimplePathTracerRenderer() = default;

        // 渲染结果类型定义
        using RenderResult = tuple<RGBA*, unsigned int, unsigned int>;
        RenderResult render();// 渲染方法，返回渲染结果
        void release(const RenderResult& r);// 释放渲染结果资源

    private:
        void renderTask(RGBA* pixels, int width, int height, int off, int step);// 渲染任务方法

        RGB gamma(const RGB& rgb);// 应用伽马校正
        RGB trace(const Ray& ray, int currDepth);// 递归追踪光线，计算颜色
        HitRecord closestHitObject(const Ray& r);// 找到与光线最近相交的物体
        tuple<float, Vec3> closestHitLight(const Ray& r);// 找到与光线最近相交的光源
    };
}

#endif