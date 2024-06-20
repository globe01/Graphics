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

        // ��������ϵͳ�����Ĵ�С��
        // ������
        // - lower: �������½磬Ĭ��Ϊ (-1.0f, -1.0f)��
        // - upper: �������Ͻ磬Ĭ��Ϊ (1.0f, 1.0f)��
        void ParticleSystem2d::setContainerSize(glm::vec2 lower /*= glm::vec2(-1.0f, -1.0f)*/, glm::vec2 upper /*= glm::vec2(1.0f, 1.0f)*/)
        {
            // �����������ӵ��������Ĵ�С��
            lower *= Lagrangian2dPara::scale;
            upper *= Lagrangian2dPara::scale;

            // ���������߽硣
            lowerBound = lower - supportRadius + particleDiameter;
            upperBound = upper + supportRadius - particleDiameter;

            // �����������ĵ㡣
            containerCenter = (lowerBound + upperBound) / 2.0f;

            // ���������ߴ硣
            glm::vec2 size = upperBound - lowerBound;

            // ����֧�ְ뾶��������������
            blockNum.x = floor(size.x / supportRadius);
            blockNum.y = floor(size.y / supportRadius);

            // ���������С��
            blockSize = glm::vec2(size.x / blockNum.x, size.y / blockNum.y);

            // ��ʼ������ ID ƫ�����顣
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

            // ����������顣
            particles.clear();
        }

        // ������ϵͳ���һ������顣
        // ������
        // - lowerCorner: �������½���λ�á�
        // - upperCorner: �������Ͻ���λ�á�
        // - v0: ����������ӵĳ�ʼ�ٶȡ�
        // - particleSpace: ����֮��ļ����
        // ���أ�
        // �ɹ���ӵ�����������
        int ParticleSystem2d::addFluidBlock(glm::vec2 lowerCorner, glm::vec2 upperCorner, glm::vec2 v0, float particleSpace)
        {
            // �����������ӵ��������Ĵ�С��
            lowerCorner *= Lagrangian2dPara::scale;
            upperCorner *= Lagrangian2dPara::scale;

            // ���������ĳߴ硣
            glm::vec2 size = upperCorner - lowerCorner;

            // ���������Ƿ��������ڲ���
            if (lowerCorner.x < lowerBound.x ||
                lowerCorner.y < lowerBound.y ||
                upperCorner.x > upperBound.x ||
                upperCorner.y > upperBound.y)
            {
                return 0; // �������鳬�������߽磬�򷵻���ӵ���������Ϊ0��
            }

            // ����������ڵ�����������
            glm::uvec2 particleNum = glm::uvec2(size.x / particleSpace, size.y / particleSpace);
            std::vector<ParticleInfo2d> tempParticles(particleNum.x * particleNum.y);

            Glb::RandomGenerator rand;
            int p = 0;
            // ����������������ӡ�
            for (int idX = 0; idX < particleNum.x; idX++)
            {
                for (int idY = 0; idY < particleNum.y; idY++)
                {
                    // �������Ӽ�������������λ�á�
                    float x = (idX + rand.GetUniformRandom()) * particleSpace;
                    float y = (idY + rand.GetUniformRandom()) * particleSpace;

                    // �������ӵ�λ�á��������� ID���ܶȺͳ�ʼ�ٶȡ�
                    tempParticles[p].position = lowerCorner + glm::vec2(x, y);
                    tempParticles[p].blockId = getBlockIdByPosition(tempParticles[p].position);
                    tempParticles[p].density = Lagrangian2dPara::density;
                    tempParticles[p].velocity = v0;
                    p++;
                }
            }

            // �����ɵ�������ӵ����������С�
            particles.insert(particles.end(), tempParticles.begin(), tempParticles.end());

            // ���سɹ���ӵ�����������
            return particles.size();
        }

        // ����λ�û�ȡ��������� ID��
        // ������
        // - position: ����ѯ��λ�á�
        // ���أ�
        // ��������� ID����λ�ò����������򷵻� -1��
        uint32_t ParticleSystem2d::getBlockIdByPosition(glm::vec2 position)
        {
            // ���λ���Ƿ��������߽��ڡ�
            if (position.x < lowerBound.x ||
                position.y < lowerBound.y ||
                position.x > upperBound.x ||
                position.y > upperBound.y)
            {
                return -2; // ���λ�ó��������߽磬�򷵻� -1��
            }

            // ����λ��������������½ǵ�ƫ������
            glm::vec2 deltePos = position - lowerBound;

            // �����������������������
            uint32_t c = floor(deltePos.x / blockSize.x);
            uint32_t r = floor(deltePos.y / blockSize.y);

            //// ������������� ID �����ء�
            //std::cout << "������:" << c << "\t������:" << r << "\t��Num:" 
            //    << blockNum.x << " " << blockNum.y << std::endl;
            return r * blockNum.x + c;
        }

        // ����������Ϣ��
        void ParticleSystem2d::updateBlockInfo()
        {
            // ���������������� ID �����������������
            std::sort(particles.begin(), particles.end(),
                [=](ParticleInfo2d& first, ParticleInfo2d& second)
                {
                    return first.blockId < second.blockId;
                });

            // ��ʼ��������չ���顣
            blockExtens = std::vector<glm::uvec2>(blockNum.x * blockNum.y, glm::uvec2(0, 0));
            int curBlockId = 0;
            int left = 0;
            int right;
            // �����������飬�����������չ��Ϣ��
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

            
            //std::cout << "���³ɹ�\t" << std::endl;
        }
    }
}
