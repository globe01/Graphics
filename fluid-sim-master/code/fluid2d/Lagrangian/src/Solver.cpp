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
        //一些参数的设置
        float mass;
        double mass_2;
        double M_PI = 3.14;
        float h;
        double h2;
        double h3;
        double max_Num = 0;

        //----------------------------------------初始化一些粒子的数据----------------------------------------
        
        Solver::Solver(ParticleSystem2d& ps) : mPs(ps)
        {
            mass = mPs.particleVolume * Lagrangian2dPara::density;
            //可选：h=sR+pR或者h=sR
            //h = mPs.supportRadius;
            h = mPs.supportRadius+mPs.particleRadius;
            h2 = mPs.supportRadius2;
            h3 = h * h2;
            mass_2 = std::pow(mass, 2);
            max_Num = (double)mPs.blockNum.x * (double)mPs.blockNum.y;
        }

        //----------------------------------------获取周围粒子----------------------------------------
        
        std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> getNearBy(
            const FluidSimulation::Lagrangian2d::ParticleInfo2d& p,
            FluidSimulation::Lagrangian2d::ParticleSystem2d& mPs) {

            // 获取粒子所在位置的区块ID
            uint32_t blockId = mPs.getBlockIdByPosition(p.position);

            std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles;

            if (blockId < 0) {//找不到对应的ID
                return blockParticles;
            }

            for (auto& index : mPs.blockIdOffs) {
                uint32_t block_temp = blockId - index;

                // 如果区块ID无效，跳过该区块
                if (block_temp < 0 || block_temp > max_Num) {
                    continue;
                }

                // 获取当前区块的粒子范围
                glm::vec2 range = mPs.blockExtens[block_temp];
                size_t startIndex = range.x;
                size_t endIndex = range.y;

                // 遍历区块中的所有粒子
                for (int i = startIndex; i < endIndex; i++) {
                    try {
                        // 计算当前粒子与目标粒子的距离
                        double distance = glm::length(mPs.particles[i].position - p.position);

                        // 如果距离小于一定阈值，则将该粒子添加到附近粒子向量中
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
    
        //----------------------------------------更新密度所用的公式----------------------------------------
        
        //核函数——二维Poly6 Kernel核函数
        inline double SphKernel(double distance) {
            if (distance * distance >= h2)
                return 0.0;
            else {
                double result = 4.0 / (M_PI * std::pow(h2, 4)) * std::pow(h2 - distance * distance, 3);
                return result;
            }
        }

        //核函数之和
        double sumOfNearby(const FluidSimulation::Lagrangian2d::ParticleInfo2d& particle,
            const std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d>& blockParticles) {

            double sum = 0.0;
            for (auto& p : blockParticles) {
                double distance = glm::length(p.position - particle.position);
                sum += SphKernel(distance);
            }
            return sum;
        }

        //更新密度
        void update_D(FluidSimulation::Lagrangian2d::ParticleSystem2d& mPs) {
            for (auto& p : mPs.particles) {
                std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles = getNearBy(p, mPs);
                if (blockParticles.size() != 0) {
                    double sum = sumOfNearby(p, blockParticles);
                    p.density = mass * sum;
                }
            }
        }

        //----------------------------------------更新压强所用的公式----------------------------------------
        
        //计算粒子压强
        double computePressure(const FluidSimulation::Lagrangian2d::ParticleInfo2d& particle) {
            double p = Lagrangian2dPara::stiffness * (std::pow(1.0 * particle.density / (1.0 * 1000), 7) - 1.0f);
            if (p < 0)return 0;
            return p;
        }

        //更新粒子压强
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

        //----------------------------------------更新压力所用的公式----------------------------------------
        
        // 二维 Poly6 核函数梯度实现
        glm::vec2 poly6KernelGradient2D(const glm::vec2& r, float h) {
            float h2 = h * h; // 平滑长度的平方
            float r2 = glm::dot(r, r); // 计算距离的平方
            if (r2 > h2) return glm::vec2(0.0f, 0.0f); // 如果距离大于平滑长度，则返回0向量
            float hr2 = h2 - r2; // 计算 (h^2 - r^2)
            float coef = -24.0f / (M_PI * std::pow(h, 8)); // 计算常数系数
            return coef * hr2 * hr2 * r; // 返回梯度值
        }

        //----------------------------------------更新粘度所用的公式----------------------------------------
        
        //拉普拉斯算子
        double Laplacian(double distance) {
            if (h < distance) {
                return 0;
            }
            return 30.0 / (M_PI * h2 * h2 * h) * (h - distance);
            //下面这个是Poly6求导得来的
            //return 24 / (M_PI * std::pow(h2, 4)) * (h2 - 5 * distance * distance) * (h2 - distance * distance);
        }

        //----------------------------------------具体的模拟----------------------------------------
        void Solver::solve()
        {
            //密度计算
            update_D(mPs);

            //压强计算
            update_P(mPs);

            //计算力
#pragma omp parallel for num_threads(NUM_THREADS)
            for (size_t i = 0; i < mPs.particles.size(); ++i) {
                auto& p = mPs.particles[i];
            /*for (auto& p : mPs.particles) {*/
                //重力
                p.force = glm::vec2(0.0f, -Lagrangian2dPara::gravityY * mass);

                //周围粒子
                std::vector<FluidSimulation::Lagrangian2d::ParticleInfo2d> blockParticles = getNearBy(p, mPs);
                
                //压力
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

                //粘性力
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
            //计算粒子加速度、速度、位置
            for (size_t i = 0; i < mPs.particles.size(); ++i) {
                auto& p = mPs.particles[i];
            /*for (auto& p : mPs.particles) {*/
                //加速度
                p.acceleration = p.force / mass;

                //速度
                p.velocity += p.acceleration * Lagrangian2dPara::dt;

                //此处选择是否限制速度的最大值
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

                //确保不发生穿透
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

                //更新粒子新位置
                p.position = new_position;

                // 更新块ID
                p.blockId = mPs.getBlockIdByPosition(p.position);
            }
        }
    }
}