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

            // ���캯������ʼ�� Eulerian2dComponent �������� ID��������Ⱦ�������������������Ϊ��ָ��
            // ���� description��Eulerian2dComponent ������
            // ���� id��Eulerian2dComponent �� ID
            Eulerian2dComponent::Eulerian2dComponent(char* description, int id) {
                this->description = description; // ��������
                this->id = id; // ����ID
                renderer = NULL; // ��ʼ����Ⱦ��Ϊ��ָ��
                solver = NULL; // ��ʼ�������Ϊ��ָ��
                grid = NULL; // ��ʼ������Ϊ��ָ��
            }

            virtual void shutDown();
            virtual void init();
            virtual void simulate();
            virtual GLuint getRenderedTexture();
        };
    }
}


#endif