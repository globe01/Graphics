#include "ParticleSystem2d.h"
#include <iostream>
#include "Global.h"
#include <unordered_set>

namespace FluidSimulation
{

    namespace Lagrangian2d
    {
        ParticleSystem2d::ParticleSystem2d()
        {
        }

        ParticleSystem2d::~ParticleSystem2d()
        {
        }

        // 设置粒子系统容器的大小。
        // 参数：
        // - lower: 容器的下界，默认为 (-1.0f, -1.0f)。
        // - upper: 容器的上界，默认为 (1.0f, 1.0f)。
        void ParticleSystem2d::setContainerSize(glm::vec2 lower /*= glm::vec2(-1.0f, -1.0f)*/, glm::vec2 upper /*= glm::vec2(1.0f, 1.0f)*/)
        {
            // 根据缩放因子调整容器的大小。
            lower *= Lagrangian2dPara::scale;
            upper *= Lagrangian2dPara::scale;

            // 计算容器边界。
            lowerBound = lower - supportRadius + particleDiameter;
            upperBound = upper + supportRadius - particleDiameter;

            // 计算容器中心点。
            containerCenter = (lowerBound + upperBound) / 2.0f;

            // 计算容器尺寸。
            glm::vec2 size = upperBound - lowerBound;

            // 根据支持半径计算区块数量。
            blockNum.x = floor(size.x / supportRadius);
            blockNum.y = floor(size.y / supportRadius);

            // 计算区块大小。
            blockSize = glm::vec2(size.x / blockNum.x, size.y / blockNum.y);

            // 初始化区块 ID 偏移数组。
            blockIdOffs.resize(9);
            int p = 0;
            for (int j = -1; j <= 1; j++)
            {
                for (int i = -1; i <= 1; i++)
                {
                    blockIdOffs[p] = blockNum.x * j + i;
                    p++;
                }
            }

            // 清空粒子数组。
            particles.clear();
        }

        // 向粒子系统添加一个流体块。
        // 参数：
        // - lowerCorner: 流体块的下角落位置。
        // - upperCorner: 流体块的上角落位置。
        // - v0: 流体块内粒子的初始速度。
        // - particleSpace: 粒子之间的间隔。
        // 返回：
        // 成功添加的粒子数量。
        int ParticleSystem2d::addFluidBlock(glm::vec2 lowerCorner, glm::vec2 upperCorner, glm::vec2 v0, float particleSpace)
        {
            // 根据缩放因子调整流体块的大小。
            lowerCorner *= Lagrangian2dPara::scale;
            upperCorner *= Lagrangian2dPara::scale;

            // 计算流体块的尺寸。
            glm::vec2 size = upperCorner - lowerCorner;

            // 检查流体块是否在容器内部。
            if (lowerCorner.x < lowerBound.x ||
                lowerCorner.y < lowerBound.y ||
                upperCorner.x > upperBound.x ||
                upperCorner.y > upperBound.y)
            {
                return 0; // 如果流体块超出容器边界，则返回添加的粒子数量为0。
            }

            // 计算流体块内的粒子数量。
            glm::uvec2 particleNum = glm::uvec2(size.x / particleSpace, size.y / particleSpace);
            std::vector<ParticleInfo2d> tempParticles(particleNum.x * particleNum.y);

            Glb::RandomGenerator rand;
            int p = 0;
            // 在流体块内生成粒子。
            for (int idX = 0; idX < particleNum.x; idX++)
            {
                for (int idY = 0; idY < particleNum.y; idY++)
                {
                    // 根据粒子间隔随机生成粒子位置。
                    float x = (idX + rand.GetUniformRandom()) * particleSpace;
                    float y = (idY + rand.GetUniformRandom()) * particleSpace;

                    // 设置粒子的位置、所属区块 ID、密度和初始速度。
                    tempParticles[p].position = lowerCorner + glm::vec2(x, y);
                    tempParticles[p].blockId = getBlockIdByPosition(tempParticles[p].position);
                    tempParticles[p].density = Lagrangian2dPara::density;
                    tempParticles[p].velocity = v0;
                    p++;
                }
            }

            // 将生成的粒子添加到粒子数组中。
            particles.insert(particles.end(), tempParticles.begin(), tempParticles.end());

            // 返回成功添加的粒子数量。
            return particles.size();
        }

        // 根据位置获取所属区块的 ID。
        // 参数：
        // - position: 待查询的位置。
        // 返回：
        // 所属区块的 ID，若位置不在容器内则返回 -1。
        uint32_t ParticleSystem2d::getBlockIdByPosition(glm::vec2 position)
        {
            // 检查位置是否在容器边界内。
            if (position.x < lowerBound.x ||
                position.y < lowerBound.y ||
                position.x > upperBound.x ||
                position.y > upperBound.y)
            {
                return -2; // 如果位置超出容器边界，则返回 -1。
            }

            // 计算位置相对于容器左下角的偏移量。
            glm::vec2 deltePos = position - lowerBound;

            // 计算所属区块的行列索引。
            uint32_t c = floor(deltePos.x / blockSize.x);
            uint32_t r = floor(deltePos.y / blockSize.y);

            //// 计算所属区块的 ID 并返回。
            //std::cout << "行索引:" << c << "\t列索引:" << r << "\t块Num:" 
            //    << blockNum.x << " " << blockNum.y << std::endl;
            return r * blockNum.x + c;
        }

        // 更新区块信息。
        void ParticleSystem2d::updateBlockInfo()
        {
            // 按照粒子所属区块 ID 对粒子数组进行排序。
            std::sort(particles.begin(), particles.end(),
                [=](ParticleInfo2d& first, ParticleInfo2d& second)
                {
                    return first.blockId < second.blockId;
                });

            // 初始化区块扩展数组。
            blockExtens = std::vector<glm::uvec2>(blockNum.x * blockNum.y, glm::uvec2(0, 0));
            int curBlockId = 0;
            int left = 0;
            int right;
            // 遍历粒子数组，更新区块的扩展信息。
            for (right = 0; right < particles.size(); right++)
            {
                if (particles[right].blockId != curBlockId)
                {
                    blockExtens[curBlockId] = glm::uvec2(left, right);
                    left = right;
                    curBlockId = particles[right].blockId;
                }
            }
            blockExtens[curBlockId] = glm::uvec2(left, right);

            
            //std::cout << "更新成功\t" << std::endl;
        }
    }
}
