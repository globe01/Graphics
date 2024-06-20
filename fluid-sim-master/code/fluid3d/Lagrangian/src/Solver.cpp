#include "fluid3d/Lagrangian/include/Solver.h"
#include "Global.h"
#include <iostream>
#include <algorithm>
#include "Configure.h"
#include <cmath>
#include<omp.h>
#define NUM_THREADS 4
#include<array>
//再写最后一次
using namespace std;

namespace FluidSimulation
{

	namespace Lagrangian3d
	{
		float mass;
		double mass_2;
		double M_PI = 3.14;
		float h;
		double h2;
		double h3;
		double max_Num = 0;

		//获取附近的粒子
		std::vector<FluidSimulation::Lagrangian3d::particle3d> getNearBy(
			FluidSimulation::Lagrangian3d::particle3d& p,
			FluidSimulation::Lagrangian3d::ParticleSystem3d& mPs) {

			uint32_t blockId = mPs.getBlockIdByPosition(p.position);

			std::vector<FluidSimulation::Lagrangian3d::particle3d> blockParticles;
			if (blockId < 0) {
				return blockParticles;
			}

			for (auto& index : mPs.blockIdOffs) {
				uint32_t block_temp = blockId - index;
				if (block_temp < 0 || block_temp >= max_Num) {
					continue;
				}
				glm::vec2 range = mPs.blockExtens[block_temp];
				size_t startIndex = range.x;
				size_t endIndex = range.y;
				for (int i = startIndex; i <= endIndex; i++) {
					try {
						double distance = glm::length(mPs.particles[i].position - p.position);
						if (distance <= h) {
							blockParticles.push_back(mPs.particles[i]);
						}
					}
					catch (const std::exception& e) {
						std::cerr << "Exception occurred at index " << i << ": " << e.what() << std::endl;
						std::cerr << "Particle position: ("
							<< mPs.particles[i].position.x << ", "
							<< mPs.particles[i].position.y << ", "
							<< mPs.particles[i].position.z << ")" << std::endl;
						std::cerr << "Target position: ("
							<< p.position.x << ", "
							<< p.position.y << ", "
							<< p.position.z << ")" << std::endl;
					}
				}
			}

			return blockParticles;
		}


		//SPH核函数
		inline double SphKernel(double distance) {
			if (distance * distance >= h2)
				return 0.0;
			else {
				double x = 1.0 - distance * distance / h2;
				return 315.0f / (64.0f * M_PI * h3) * std::pow(x, 3);
			}
		}

		//SPH核函数二阶导
		inline double SphKernel_SecondDerivative(double distance) {
			if (distance * distance >= h2) {
				return 0.0;
			}
			//这个具体的核函数还可以修改
			double x = distance * distance / h2;
			return 945.0 / (32.0 * M_PI * h2 * h3) * (1 - x)*(5*x-1);
		}

		//SPH_Spi核函数二阶导
		inline double SphSpiKernel_SecondDerivative(double distance) {
			//if (distance * distance >= h2) {
			//	return 0.0;
			//}
			////这个具体的核函数还可以修改
			//double x = distance * distance / h2;
			//return 945.0 / (32.0 * M_PI * h2 * h3) * (1 - x) * (5 * x - 1);

			if (distance >= h)
				return 0.0;
			else
			{
				double x = 1.0 - distance / h;
				return 90.0 / (M_PI * h2*h3) * x;
			}
		}

		//核函数之和
		double sumOfNearby(const FluidSimulation::Lagrangian3d::particle3d& particle,
			const std::vector<FluidSimulation::Lagrangian3d::particle3d>& blockParticles) {

			double sum = 0.0;
			for (auto& p : blockParticles) {
				double distance = glm::length(p.position - particle.position);
				sum += SphKernel(distance);
			}
			return sum;
		}

		// 更新密度
		void update_Density(FluidSimulation::Lagrangian3d::ParticleSystem3d& mPs) {
			for (auto& p : mPs.particles) {

				std::vector < FluidSimulation::Lagrangian3d::particle3d> blockParticles = getNearBy(p, mPs);//这个密度也有问题

				if (blockParticles.size() != 0) {
					double sum = sumOfNearby(p, blockParticles);
					p.density = mass * sum;
				}

				//加上重力
				p.force = glm::vec3(0.0f, 0.0f, -mass * Lagrangian3dPara::gravityZ);
			}
		}

		//计算粒子压强
		double computePressure(const FluidSimulation::Lagrangian3d::particle3d& particle) {
			double p = Lagrangian3dPara::stiffness * (std::pow(1.0 * particle.density / (1.0 * 1000), 7) - 1.0f);
			if (p < 0)return 0;
			return p;
		}

		//更新粒子压强
		void update_P(FluidSimulation::Lagrangian3d::ParticleSystem3d& mPs) {
			for (auto& p : mPs.particles) {
				p.pressure = computePressure(p);
				p.pressDivDens2 = p.pressure / (p.density * p.density);
			}
		}

		glm::vec3 poly6KernelGradient2D(const glm::vec3& r, float h) {
			float h2 = h * h; // 平滑长度的平方
			float r2 = glm::dot(r, r); // 计算距离的平方
			if (r2 > h2) return glm::vec3(0.0f, 0.0f, 0.0f); // 如果距离大于平滑长度，则返回0向量
			float hr2 = h2 - r2; // 计算 (h^2 - r^2)
			float coef = -945.f / (32.f * M_PI * std::pow(h, 9)); // 计算常数系数
			return coef * hr2 * hr2 * r; // 返回梯度值
		}


		double Laplacian(double distance) {
			if (h < distance) {
				return 0;
			}
			return 45.f / (M_PI * std::pow(h, 6)) * (h - distance);
		}

		inline double firstDerivative(double distance) {
			if (distance >= h)
				return 0.0;
			else
			{
				double x = 1.0 - distance / h;
				return -45.0 / (M_PI * h2*h2) * x * x;
			}
		}

		inline glm::vec3 SphSpi_Gradient(double distance, glm::vec3& dir) {
			return glm::vec3(firstDerivative(distance)) * dir;
		}

		//执行初始化操作
		Solver::Solver(ParticleSystem3d& ps) : mPs(ps) {
			mass = mPs.particleVolume * Lagrangian3dPara::density;
			/*h = mPs.supportRadius;*/
			h = mPs.supportRadius + Lagrangian3dPara::particleRadius;
			h2 = mPs.supportRadius2;
			h3 = h * h2;
			mass_2 = std::pow(mass, 2);
			max_Num = (double)mPs.blockNum.x * (double)mPs.blockNum.y * (double)mPs.blockNum.z;

			mPs.updateBlockInfo();

#pragma omp parallel for num_threads(NUM_THREADS)
			for (auto& p : mPs.particles) {
				std::vector<FluidSimulation::Lagrangian3d::particle3d> blockParticles = getNearBy(p, mPs);
				if (blockParticles.size() != 0) {
					double sum = sumOfNearby(p, blockParticles);
					/*p.newDensity = mass * sum;*/
					p.Odensity = mass * sum;
				}
			}
		}

		void Solver::solve() {
			update_Density(mPs);

			update_P(mPs);

#pragma omp parallel for num_threads(NUM_THREADS)
			//速度和位置的更新
			for (auto& p : mPs.particles) {

				std::vector<FluidSimulation::Lagrangian3d::particle3d> blockParticles = getNearBy(p, mPs);

				//粘性力的更新
				if (blockParticles.size() != 0) {
					for (auto& nearby : blockParticles) {
						double distance = glm::length(p.position - nearby.position);
						double temp = Lagrangian3dPara::viscosity * (double)mass *  SphSpiKernel_SecondDerivative(distance) / nearby.density;
						p.force += glm::vec3(temp) * (nearby.velocity - p.velocity);
					}
				}

				//压力的更新
				if (blockParticles.size() != 0) {
					for (auto& nearby : blockParticles) {
						/*glm::vec3 r = p.position - nearby.position;
						double temp = mass_2 * (p.pressDivDens2 + nearby.pressDivDens2);
						p.force -= glm::vec3(temp) * poly6KernelGradient2D(r, h);*/
						double distance = glm::length(nearby.position - p.position);
						if (distance > 0.0) {
							glm::vec3 dir = (nearby.position - p.position) / glm::vec3(distance);
							double temp = mass_2 * (p.pressDivDens2 + nearby.pressDivDens2);
							p.force += glm::vec3(temp) * SphSpi_Gradient(distance, dir);
						}
					}
				}
			}


#pragma omp parallel for num_threads(NUM_THREADS)
			for (auto& p : mPs.particles) {
				p.accleration = p.force / mass;
				//暂时此处放置一个加速度
				p.velocity += p.accleration * Lagrangian3dPara::dt;
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
				if (p.velocity.z >= 10.0f) {
					p.velocity.z = 10.0f;
				}
				if (p.velocity.z <= -10.0f) {
					p.velocity.z = -10.0f;
				}

				glm::vec3 new_position = p.position+p.velocity * Lagrangian3dPara::dt;
				

				if (new_position.y <= mPs.lowerBound.y) {
					new_position.y = mPs.lowerBound.y + Lagrangian3dPara::eps;//Lagrangian3dPara::particleDiameter;
					p.velocity.y *= 0;
				}
				if (new_position.x <= mPs.lowerBound.x) {
					new_position.x = mPs.lowerBound.x + Lagrangian3dPara::eps;
					p.velocity.x *= 0;
				}
				if (new_position.z <= mPs.lowerBound.z) {
					new_position.z = mPs.lowerBound.z + Lagrangian3dPara::eps;
					p.velocity.z *= 0;
				}
				if (new_position.y >= mPs.upperBound.y) {
					new_position.y = mPs.upperBound.y - Lagrangian3dPara::eps;
					p.velocity.y *= 0;
				}
				if (new_position.x >= mPs.upperBound.x) {
					new_position.x = mPs.upperBound.x - Lagrangian3dPara::eps;
					p.velocity.x *= 0;
				}
				if (new_position.z >= mPs.upperBound.z) {
					new_position.z = mPs.upperBound.z - Lagrangian3dPara::eps;
					p.velocity.z *= 0;
				}

				p.position = new_position;

				// 更新块ID
				p.blockId = mPs.getBlockIdByPosition(p.position);
			}
		}
	}
}