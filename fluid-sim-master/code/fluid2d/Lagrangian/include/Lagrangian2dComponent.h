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
        // Lagrangian2dComponent ���� Glb::Component �����࣬���ڹ����ά������������ģ�������
        class Lagrangian2dComponent : public Glb::Component
        {
        public:
            // ��Ⱦ������������Ļ�ϻ�������Ч����
            Renderer* renderer;

            // Solver �������ڽ������ģ��ķ��̡�
            Solver* solver;

            // ParticleSystem2d ���󣬹����ά������������ϵͳ��
            ParticleSystem2d* ps;

            // Lagrangian2dComponent ��Ĺ��캯������ʼ�������� ID��
            // ������
            // - description: ������������ַ�����
            // - id: �����Ψһ��ʶ����
            Lagrangian2dComponent(char* description, int id)
            {
                this->description = description;
                this->id = id;
                renderer = NULL;
                solver = NULL;
                ps = NULL;
            }

            // �رո�������˺���Ӧ���ͷ��κη������Դ��
            virtual void shutDown();

            // ��ʼ���������׼����ʼģ�����塣
            virtual void init();

            // ִ������ģ���һ�����������������λ�á�������巽�̵Ȳ�����
            virtual void simulate();

            // ��ȡ������Ⱦ������
            // ���أ�
            // ������Ⱦ������� OpenGL ���� ID��
            virtual GLuint getRenderedTexture();
        };
    }
}


#endif