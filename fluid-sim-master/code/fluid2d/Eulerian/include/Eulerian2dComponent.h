#pragma once
#ifndef __EULERIAN_2D_COMPONENT_H__
#define __EULERIAN_2D_COMPONENT_H__

#include "Renderer.h"
#include "Solver.h"
#include "MACGrid2d.h"

#include "Component.h"
#include "Configure.h"

#include "Logger.h"

namespace FluidSimulation {
    namespace Eulerian2d {
        class Eulerian2dComponent : public Glb::Component {
        public:
            Renderer* renderer;
            Solver* solver;
            MACGrid2d* grid;

            // 构造函数，初始化 Eulerian2dComponent 的描述和 ID，并将渲染器、求解器和网格设置为空指针
            // 参数 description：Eulerian2dComponent 的描述
            // 参数 id：Eulerian2dComponent 的 ID
            Eulerian2dComponent::Eulerian2dComponent(char* description, int id) {
                this->description = description; // 设置描述
                this->id = id; // 设置ID
                renderer = NULL; // 初始化渲染器为空指针
                solver = NULL; // 初始化求解器为空指针
                grid = NULL; // 初始化网格为空指针
            }

            virtual void shutDown();
            virtual void init();
            virtual void simulate();
            virtual GLuint getRenderedTexture();
        };
    }
}


#endif