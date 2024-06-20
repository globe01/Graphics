#pragma once
#ifndef __LAGRANGIAN_2D_COMPONENT_H__
#define __LAGRANGIAN_2D_COMPONENT_H__

#include "Renderer.h"
#include "Solver.h"
#include "ParticleSystem2d.h"

#include "Component.h"
#include "Configure.h"
#include "Logger.h"

namespace FluidSimulation
{
    namespace Lagrangian2d
    {
        // Lagrangian2dComponent 类是 Glb::Component 的子类，用于管理二维拉格朗日流体模拟组件。
        class Lagrangian2dComponent : public Glb::Component
        {
        public:
            // 渲染器，用于在屏幕上绘制流体效果。
            Renderer* renderer;

            // Solver 对象，用于解决流体模拟的方程。
            Solver* solver;

            // ParticleSystem2d 对象，管理二维拉格朗日粒子系统。
            ParticleSystem2d* ps;

            // Lagrangian2dComponent 类的构造函数，初始化描述和 ID。
            // 参数：
            // - description: 描述该组件的字符串。
            // - id: 组件的唯一标识符。
            Lagrangian2dComponent(char* description, int id)
            {
                this->description = description;
                this->id = id;
                renderer = NULL;
                solver = NULL;
                ps = NULL;
            }

            // 关闭该组件。此函数应该释放任何分配的资源。
            virtual void shutDown();

            // 初始化该组件，准备开始模拟流体。
            virtual void init();

            // 执行流体模拟的一步。这包括更新粒子位置、解决流体方程等操作。
            virtual void simulate();

            // 获取用于渲染的纹理。
            // 返回：
            // 用于渲染的纹理的 OpenGL 纹理 ID。
            virtual GLuint getRenderedTexture();
        };
    }
}


#endif