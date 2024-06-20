#include "Lagrangian/include/Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>
#include "Configure.h"
#include <cmath>
#include<array>
#include<omp.h>
#define NUM_THREADS 2
using namespace std;
namespace FluidSimulation
{
    namespace Lagrangian2d
    {   
        //һЩ����������
        float mass;
        double mass_2;
        double M_PI = 3.14;
        float h;
        double h2;
        double h3;
        double max_Num = 0;

        //----------------------------------------��ʼ��һЩ���ӵ�����----------------------------------------
        
        Solver::Solver(ParticleSystem2d& ps) : mPs(ps)
        {
            mass = mPs.particleVolume * Lagrangian2dPara::density;
            //��ѡ��h=sR+pR����h=sR
            //h = mPs.supportRadius;
            h = mPs.supportRadius+mPs.particleRadius;
            h2 = mPs.supportRadius2;
            h3 = h * h2;
            mass_2 = std::pow(mass, 2);
            max_Num = (double)mPs.blockNum.x * (double)mPs.blockNum.y;
        }

        //----------------------------------------��ȡ��Χ����----------------------------------------
        
        std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> getNearBy(
            const FluidSimulation::Lagrangian2d::ParticleInfo2d& p,
            FluidSimulation::Lagrangian2d::ParticleSystem2d& mPs) {

            // ��ȡ��������λ�õ�����ID
            uint32_t blockId = mPs.getBlockIdByPosition(p.position);

            std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles;

            if (blockId < 0) {//�Ҳ�����Ӧ��ID
                return blockParticles;
            }

            for (auto& index : mPs.blockIdOffs) {
                uint32_t block_temp = blockId - index;

                // �������ID��Ч������������
                if (block_temp < 0 || block_temp > max_Num) {
                    continue;
                }

                // ��ȡ��ǰ��������ӷ�Χ
                glm::vec2 range = mPs.blockExtens[block_temp];
                size_t startIndex = range.x;
                size_t endIndex = range.y;

                // ���������е���������
                for (int i = startIndex; i < endIndex; i++) {
                    try {
                        // ���㵱ǰ������Ŀ�����ӵľ���
                        double distance = glm::length(mPs.particles[i].position - p.position);

                        // �������С��һ����ֵ���򽫸�������ӵ���������������
                        if (distance <= h) {
                            blockParticles.push_back(mPs.particles[i]);
                        }
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Exception occurred at index " << i << ": " << e.what() << std::endl;
                        std::cerr << "Particle position: ("
                            << mPs.particles[i].position.x << ", "
                            << mPs.particles[i].position.y << ")" << std::endl;
                        std::cerr << "Target position: ("
                            << p.position.x << ", "
                            << p.position.y << ")" << std::endl;
                    }
                }
            }

            return blockParticles;
        }
    
        //----------------------------------------�����ܶ����õĹ�ʽ----------------------------------------
        
        //�˺���������άPoly6 Kernel�˺���
        inline double SphKernel(double distance) {
            if (distance * distance >= h2)
                return 0.0;
            else {
                double result = 4.0 / (M_PI * std::pow(h2, 4)) * std::pow(h2 - distance * distance, 3);
                return result;
            }
        }

        //�˺���֮��
        double sumOfNearby(const FluidSimulation::Lagrangian2d::ParticleInfo2d& particle,
            const std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d>& blockParticles) {

            double sum = 0.0;
            for (auto& p : blockParticles) {
                double distance = glm::length(p.position - particle.position);
                sum += SphKernel(distance);
            }
            return sum;
        }

        //�����ܶ�
        void update_D(FluidSimulation::Lagrangian2d::ParticleSystem2d& mPs) {
            for (auto& p : mPs.particles) {
                std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles = getNearBy(p, mPs);
                if (blockParticles.size() != 0) {
                    double sum = sumOfNearby(p, blockParticles);
                    p.density = mass * sum;
                }
            }
        }

        //----------------------------------------����ѹǿ���õĹ�ʽ----------------------------------------
        
        //��������ѹǿ
        double computePressure(const FluidSimulation::Lagrangian2d::ParticleInfo2d& particle) {
            double p = Lagrangian2dPara::stiffness * (std::pow(1.0 * particle.density / (1.0 * 1000), 7) - 1.0f);
            if (p < 0)return 0;
            return p;
        }

        //��������ѹǿ
        void update_P(FluidSimulation::Lagrangian2d::ParticleSystem2d& mPs) {
#pragma omp parallel for num_threads(NUM_THREADS)
            /*for (auto& p : mPs.particles) {
                p.pressure = computePressure(p);
                p.pressDivDens2 = p.pressure / (p.density * p.density);
            }*/
            for (size_t i = 0; i < mPs.particles.size(); ++i) {
                auto& p = mPs.particles[i];
                p.pressure = computePressure(p);
                p.pressDivDens2 = p.pressure / (p.density * p.density);
            }
        }

        //----------------------------------------����ѹ�����õĹ�ʽ----------------------------------------
        
        // ��ά Poly6 �˺����ݶ�ʵ��
        glm::vec2 poly6KernelGradient2D(const glm::vec2& r, float h) {
            float h2 = h * h; // ƽ�����ȵ�ƽ��
            float r2 = glm::dot(r, r); // ��������ƽ��
            if (r2 > h2) return glm::vec2(0.0f, 0.0f); // ����������ƽ�����ȣ��򷵻�0����
            float hr2 = h2 - r2; // ���� (h^2 - r^2)
            float coef = -24.0f / (M_PI * std::pow(h, 8)); // ���㳣��ϵ��
            return coef * hr2 * hr2 * r; // �����ݶ�ֵ
        }

        //----------------------------------------����ճ�����õĹ�ʽ----------------------------------------
        
        //������˹����
        double Laplacian(double distance) {
            if (h < distance) {
                return 0;
            }
            return 30.0 / (M_PI * h2 * h2 * h) * (h - distance);
            //���������Poly6�󵼵�����
            //return 24 / (M_PI * std::pow(h2, 4)) * (h2 - 5 * distance * distance) * (h2 - distance * distance);
        }

        //----------------------------------------�����ģ��----------------------------------------
        void Solver::solve()
        {
            //�ܶȼ���
            update_D(mPs);

            //ѹǿ����
            update_P(mPs);

            //������
#pragma omp parallel for num_threads(NUM_THREADS)
            for (size_t i = 0; i < mPs.particles.size(); ++i) {
                auto& p = mPs.particles[i];
            /*for (auto& p : mPs.particles) {*/
                //����
                p.force = glm::vec2(0.0f, -Lagrangian2dPara::gravityY * mass);

                //��Χ����
                std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles = getNearBy(p, mPs);
                
                //ѹ��
                if (blockParticles.size() != 0) {
                    for (auto& nearby : blockParticles) {
                        if (&p != &nearby) {
                            glm::vec2 r = p.position - nearby.position;
                            double temp = mass_2 * (p.pressDivDens2 + nearby.pressDivDens2);
                            p.force -= glm::vec2(temp) * poly6KernelGradient2D(r, h);

                           /* glm::vec2 r = p.position - nearby.position;
                            double distance = glm::length(r);
                            glm::vec2 dir = r / glm::vec2(distance);*/
                        }
                    }
                }

                //ճ����
                if (blockParticles.size() != 0) {
                    for (auto& nearby : blockParticles) {
                        if (&p != &nearby) {
                            double distance = glm::length(p.position - nearby.position);
                            double temp = Lagrangian2dPara::viscosity * (double)mass * Laplacian(distance) / nearby.density;
                            p.force += glm::vec2(temp) * (nearby.velocity - p.velocity);
                        }
                    }
                }
            }

#pragma omp parallel for num_threads(NUM_THREADS)
            //�������Ӽ��ٶȡ��ٶȡ�λ��
            for (size_t i = 0; i < mPs.particles.size(); ++i) {
                auto& p = mPs.particles[i];
            /*for (auto& p : mPs.particles) {*/
                //���ٶ�
                p.acceleration = p.force / mass;

                //�ٶ�
                p.velocity += p.acceleration * Lagrangian2dPara::dt;

                //�˴�ѡ���Ƿ������ٶȵ����ֵ
                if (p.velocity.x >= 10.0f) {
                    p.velocity.x = 10.0f;
                }
                if (p.velocity.x <= -10.0f) {
                    p.velocity.x = -10.0f;
                }
                if (p.velocity.y >= 10.0f) {
                    p.velocity.y = 10.0f;
                }
                if (p.velocity.y <= -10.0f) {
                    p.velocity.y = -10.0f;
                }

                //ȷ����������͸
                glm::vec2 new_position = p.position + p.velocity * Lagrangian2dPara::dt;
                if (new_position.y <= mPs.lowerBound.y) {
                    new_position.y = mPs.lowerBound.y + Lagrangian2dPara::eps;//Lagrangian2dPara::particleDiameter;
                    p.velocity.y = 0;
                     //p.velocity.x *= 0.7;
                }
                if (new_position.x <= mPs.lowerBound.x) {
                    new_position.x = mPs.lowerBound.x + Lagrangian2dPara::eps;
                    p.velocity.x = 0;
                    //p.velocity.y *= 0.7;
                }
                if (new_position.y >= mPs.upperBound.y) {
                    new_position.y = mPs.upperBound.y - Lagrangian2dPara::eps;
                    p.velocity.y = 0;
                    //p.velocity.x *= 0.7;
                }
                if (new_position.x >= mPs.upperBound.x) {
                    new_position.x = mPs.upperBound.x - Lagrangian2dPara::eps;
                    p.velocity.x = 0;
                    //p.velocity.y *= 0.7;
                }

                //����������λ��
                p.position = new_position;

                // ���¿�ID
                p.blockId = mPs.getBlockIdByPosition(p.position);
            }
        }
    }
}