#include "fluid3d/Eulerian/include/Solver.h"
#include "Configure.h"
#include "Global.h"
#define NUM_THREADS 16
using namespace std;

namespace FluidSimulation
{
    namespace Eulerian3d
    {
        
        void advection(MACGrid3d& mGrid) {
            // 创建新密度和温度网格数据
            Glb::CubicGridData3d newDensity = mGrid.mD;
            Glb::CubicGridData3d newTemperature = mGrid.mT;

            // 创建新的速度网格数据
            Glb::GridData3dX newU = mGrid.mU;
            Glb::GridData3dY newV = mGrid.mV;
            Glb::GridData3dZ newW = mGrid.mW;

            // 并行处理速度场的对流更新
#pragma omp parallel for collapse(3) num_threads(NUM_THREADS)
            for (int i = 0; i < mGrid.dim[0]; ++i) {
                for (int j = 0; j < mGrid.dim[1]; ++j) {
                    for (int k = 0; k < mGrid.dim[2]; ++k) {
                        glm::vec3 pt = mGrid.getCenter(i, j, k); // 获取当前单元中心点
                        glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // 通过半拉格朗日方法获取前一个位置
                        newDensity(i, j, k) = mGrid.mD.interpolate(lastPos); // 插值计算新的密度值
                        newTemperature(i, j, k) = mGrid.mT.interpolate(lastPos); // 插值计算新的温度值

                        if (i < mGrid.dim[0]) {
                            glm::vec3 pt = mGrid.getBack(i, j, k); // 获取后边界点
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // 获取前一个位置
                            newU(i, j, k) = mGrid.getVelocityX(lastPos); // 插值计算新的x方向速度
                        }

                        if (j < mGrid.dim[1]) {
                            glm::vec3 pt = mGrid.getLeft(i, j, k); // 获取左边界点
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // 获取前一个位置
                            newV(i, j, k) = mGrid.getVelocityY(lastPos); // 插值计算新的y方向速度
                        }

                        if (k < mGrid.dim[2]) {
                            glm::vec3 pt = mGrid.getBottom(i, j, k); // 获取下边界点
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // 获取前一个位置
                            newW(i, j, k) = mGrid.getVelocityZ(lastPos); // 插值计算新的z方向速度
                        }
                    }
                }
            }

            // 更新速度场
            mGrid.mU = newU;
            mGrid.mV = newV;
            mGrid.mW = newW;

            // 更新网格数据
            mGrid.mD = newDensity;
            mGrid.mT = newTemperature;
        }

        //施加外力的影响
        void external_forces(MACGrid3d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_FACE{
                glm::vec3 center = mGrid.getCenter(i, j,k);
                //glm::vec2 center(i*mGrid.cellSize, j* mGrid.cellSize);
                mGrid.mW(i, j,k) += mGrid.getBoussinesqForce(center) * Lagrangian3dPara::dt;
            }
        }

        void projection(MACGrid3d& mGrid) {
            int iterations = 50; // 最大迭代次数
            double dt = Lagrangian3dPara::dt; // 时间步长
            double dx = mGrid.cellSize; // 空间步长
            double p = 1.3; // 空气密度
            double alpha = -p * dx * dx * dx / dt; // 迭代系数
            double tolerance = 1e-6; // 收敛阈值

            std::vector<std::vector<std::vector<double>>> pressure(Eulerian3dPara::theDim3d[MACGrid3d::X],
                std::vector<std::vector<double>>(Eulerian3dPara::theDim3d[MACGrid3d::Y],
                    std::vector<double>(Eulerian3dPara::theDim3d[MACGrid3d::Z], 0.0)));

            for (int k = 0; k < iterations; k++) {
                double maxResidual = 0.0;
                std::vector<std::vector<std::vector<double>>> newPressure = pressure;

                FOR_EACH_CELL{
                    if (mGrid.isSolidCell(i, j, k)) continue; // 跳过固体单元
                    double div = mGrid.checkDivergence(i, j, k); // 获取当前位置的散度
                    double p_neighbors = 0.0;
                    int num_neighbors = 0;

                    // 计算相邻网格单元的压力值和压力系数
                    if (k + 1 < Eulerian3dPara::theDim3d[MACGrid3d::Z] && !mGrid.isSolidCell(i, j, k + 1)) {
                        p_neighbors += pressure[i][j][k + 1];
                        num_neighbors++;
                    }
                    if (k - 1 >= 0 && !mGrid.isSolidCell(i, j, k - 1)) {
                        p_neighbors += pressure[i][j][k - 1];
                        num_neighbors++;
                    }
                    if (j + 1 < Eulerian3dPara::theDim3d[MACGrid3d::Y] && !mGrid.isSolidCell(i, j + 1, k)) {
                        p_neighbors += pressure[i][j + 1][k];
                        num_neighbors++;
                    }
                    if (j - 1 >= 0 && !mGrid.isSolidCell(i, j - 1, k)) {
                        p_neighbors += pressure[i][j - 1][k];
                        num_neighbors++;
                    }
                    if (i + 1 < Eulerian3dPara::theDim3d[MACGrid3d::X] && !mGrid.isSolidCell(i + 1, j, k)) {
                        p_neighbors += pressure[i + 1][j][k];
                        num_neighbors++;
                    }
                    if (i - 1 >= 0 && !mGrid.isSolidCell(i - 1, j, k)) {
                        p_neighbors += pressure[i - 1][j][k];
                        num_neighbors++;
                    }

                    if (num_neighbors > 0) {
                        newPressure[i][j][k] = (div * alpha + p_neighbors) / num_neighbors;
                    }

                    // 计算残差
                    double residual = fabs(newPressure[i][j][k] - pressure[i][j][k]);
                    if (residual > maxResidual) {
                        maxResidual = residual;
                    }
                }

                pressure = newPressure;

                // 检查收敛条件
                if (maxResidual < tolerance) {
                    break;
                }
            }

#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{
                if (!mGrid.isSolidCell(i, j, k)) {
                    // 更新速度
                    double p_x = 0.0;
                    double p_y = 0.0;
                    double p_z = 0.0;

                    if (i - 1 >= 0 && !mGrid.isSolidCell(i - 1, j, k)) {
                        p_x = pressure[i][j][k] - pressure[i - 1][j][k];
                    }
                    if (j - 1 >= 0 && !mGrid.isSolidCell(i, j - 1, k)) {
                        p_y = pressure[i][j][k] - pressure[i][j - 1][k];
                    }
                    if (k - 1 >= 0 && !mGrid.isSolidCell(i, j, k - 1)) {
                        p_z = pressure[i][j][k] - pressure[i][j][k - 1];
                    }

                    mGrid.mU(i, j, k) -= dt / (p * dx) * p_x;
                    mGrid.mV(i, j, k) -= dt / (p * dx) * p_y;
                    mGrid.mW(i, j, k) -= dt / (p * dx) * p_z;
                }
            }

                // 输出压力场
                //outputPressureField(pressure);
        }


        Solver::Solver(MACGrid3d &grid) : mGrid(grid)//初始化
        {
            mGrid.reset();
           
        }

        void Solver::solve()//平流、求外力、投影——速度在边界，温度、密度在内部
        {
            //平流操作

            //获取的是当前位置的“粒子”上一个时间步的位置
            
            advection(mGrid);

            //test(mGrid);

            external_forces(mGrid);

            //test(mGrid);

            //此处是否需要有一个扩散步骤

            projection(mGrid);

            //test(mGrid);
        }
    }
}
