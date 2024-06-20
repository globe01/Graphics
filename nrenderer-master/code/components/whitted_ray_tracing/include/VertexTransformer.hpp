#pragma once
#ifndef __VERTEX_TRANSFORM_HPP__
#define __VERTEX_TRANSFORM_HPP__

#include "scene/Scene.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    using namespace NRenderer;
    // 由局部坐标转换为世界坐标
    class VertexTransformer
    {
    private:
    public:
        void exec(SharedScene spScene);
    };
}

#endif