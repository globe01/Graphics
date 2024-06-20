#pragma once
#ifndef __PARTICAL_SYSTEM_2D_H__
#define __PARTICAL_SYSTEM_2D_H__

#include <vector>
#include <list>
#include <glm/glm.hpp>
#include "Global.h"

#include "Configure.h"

namespace FluidSimulation
{

    namespace Lagrangian2d
    {
        // ��ά������Ϣ�ṹ��
        struct ParticleInfo2d
        {
            alignas(8) glm::vec2 position;       // ����λ�ã�ʹ�� 8 �ֽڶ���
            alignas(8) glm::vec2 velocity;       // �����ٶȣ�ʹ�� 8 �ֽڶ���
            alignas(8) glm::vec2 acceleration;   // ���Ӽ��ٶȣ�ʹ�� 8 �ֽڶ���
            alignas(4) float density;            // �����ܶȣ�ʹ�� 4 �ֽڶ���
            alignas(4) float pressure;           // ����ѹǿ��ʹ�� 4 �ֽڶ���
            alignas(4) float pressDivDens2;      // ѹǿ���ܶȱ�ֵ��ƽ����ʹ�� 4 �ֽڶ���
            alignas(4) uint32_t blockId;         // ���������ı�ʶ��ʹ�� 4 �ֽڶ���
            
            alignas(8) glm::vec2 force;          //������һ����

            alignas(4) float oldDensity;         //you need to use it
            alignas(4) float newDensity;         //ԭʼ�ܶȱ��ֲ���
        };
                            

        class ParticleSystem2d
        {
        public:
            ParticleSystem2d();
            ~ParticleSystem2d();

            void setContainerSize(glm::vec2 containerCorner, glm::vec2 containerSize);
            int32_t addFluidBlock(glm::vec2 corner, glm::vec2 size, glm::vec2 v0, float particleSpace);
            uint32_t getBlockIdByPosition(glm::vec2 position);
            void updateBlockInfo();

        public:
            // ���Ӳ���
            float supportRadius = Lagrangian2dPara::supportRadius;//��Ӱ��ķ�Χ�뾶
            float supportRadius2 = supportRadius * supportRadius;//��Ӱ�췶Χ�������Σ���
            float particleRadius = Lagrangian2dPara::particleRadius;//���Ӱ뾶
            float particleDiameter = Lagrangian2dPara::particleDiameter;//ֱ��
            float particleVolume = particleDiameter * particleDiameter;//�������=ֱ��*ֱ����

            // �洢ȫ��������Ϣ
            std::vector<ParticleInfo2d> particles;

            // ��������
            glm::vec2 lowerBound = glm::vec2(FLT_MAX);
            glm::vec2 upperBound = glm::vec2(-FLT_MAX);
            glm::vec2 containerCenter = glm::vec2(0.0f);
            float scale = Lagrangian2dPara::scale;
            
            // Block�ṹ�������ٽ�������
            glm::uvec2 blockNum = glm::uvec2(0);
            glm::vec2 blockSize = glm::vec2(0.0f);
            std::vector<glm::uvec2> blockExtens;
            std::vector<int32_t> blockIdOffs;
        };
    }
}

#endif // !PARTICAL_SYSTEM_H
