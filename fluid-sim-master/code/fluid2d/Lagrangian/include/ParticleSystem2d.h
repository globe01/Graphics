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
        // 二维粒子信息结构体
        struct ParticleInfo2d
        {
            alignas(8) glm::vec2 position;       // 粒子位置，使用 8 字节对齐
            alignas(8) glm::vec2 velocity;       // 粒子速度，使用 8 字节对齐
            alignas(8) glm::vec2 acceleration;   // 粒子加速度，使用 8 字节对齐
            alignas(4) float density;            // 粒子密度，使用 4 字节对齐
            alignas(4) float pressure;           // 粒子压强，使用 4 字节对齐
            alignas(4) float pressDivDens2;      // 压强与密度比值的平方，使用 4 字节对齐
            alignas(4) uint32_t blockId;         // 所在网格块的标识，使用 4 字节对齐
            
            alignas(8) glm::vec2 force;          //给它加一个力

            alignas(4) float oldDensity;         //you need to use it
            alignas(4) float newDensity;         //原始密度保持不变
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
            // 粒子参数
            float supportRadius = Lagrangian2dPara::supportRadius;//受影响的范围半径
            float supportRadius2 = supportRadius * supportRadius;//受影响范围（正方形？）
            float particleRadius = Lagrangian2dPara::particleRadius;//粒子半径
            float particleDiameter = Lagrangian2dPara::particleDiameter;//直径
            float particleVolume = particleDiameter * particleDiameter;//粒子体积=直径*直径？

            // 存储全部粒子信息
            std::vector<ParticleInfo2d> particles;

            // 容器参数
            glm::vec2 lowerBound = glm::vec2(FLT_MAX);
            glm::vec2 upperBound = glm::vec2(-FLT_MAX);
            glm::vec2 containerCenter = glm::vec2(0.0f);
            float scale = Lagrangian2dPara::scale;
            
            // Block结构（加速临近搜索）
            glm::uvec2 blockNum = glm::uvec2(0);
            glm::vec2 blockSize = glm::vec2(0.0f);
            std::vector<glm::uvec2> blockExtens;
            std::vector<int32_t> blockIdOffs;
        };
    }
}

#endif // !PARTICAL_SYSTEM_H
