#include "Eulerian/include/Solver.h"
#include "Configure.h"
#include<omp.h>
#define NUM_THREADS 0
using namespace std;
namespace FluidSimulation
{
    namespace Eulerian2d
    {

        //单元测试
        void test(MACGrid2d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{
                if (mGrid.mV(i,j) != 0 || mGrid.mU(i,j) != 0)cout << i << "\t" << j << "\t" << mGrid.mV(i,j) << "\t" << mGrid.mU(i,j) << endl;
            }
        }

        void advection(MACGrid2d& mGrid) {
            // 创建新密度和温度网格数据
            Glb::CubicGridData2d newDensity = mGrid.mD;
            Glb::CubicGridData2d newTempeture = mGrid.mT;

            //对流体进行对流

            Glb::GridData2dX newU = mGrid.mU;
            Glb::GridData2dY newV = mGrid.mV;

            // 并行处理速度场的对流更新
#pragma omp parallel for num_threads(NUM_THREADS)
            for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y] + 1; j++)
                for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X] + 1; i++) {
                    glm::vec2 pt = mGrid.getCenter(i, j); // 获取当前单元中心点
                    glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 通过半拉格朗日方法获取前一个位置
                    newDensity(i, j) = mGrid.mD.interpolate(lastPos); // 插值计算新的密度值
                    newTempeture(i, j) = mGrid.mT.interpolate(lastPos); // 插值计算新的温度值

                    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
                        glm::vec2 pt = mGrid.getLeft(i, j); // 获取左边界点
                        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 获取前一个位置
                        newU(i, j) = mGrid.getVelocityX(lastPos); // 插值计算新的x方向速度
                    }

                    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
                        glm::vec2 pt = mGrid.getBottom(i, j); // 获取下边界点
                        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 获取前一个位置
                        newV(i, j) = mGrid.getVelocityY(lastPos); // 插值计算新的y方向速度
                    }
                }

            //FOR_EACH_LINE{//i和j的设计应该保证了对应位置访问的正确性

            //    glm::vec2 pt = mGrid.getCenter(i, j); // 获取当前单元中心点
            //    glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 通过半拉格朗日方法获取前一个位置
            //    newDensity(i, j) = mGrid.mD.interpolate(lastPos); // 插值计算新的密度值
            //    newTempeture(i, j) = mGrid.mT.interpolate(lastPos); // 插值计算新的温度值

            //    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
            //        glm::vec2 pt = mGrid.getLeft(i, j); // 获取左边界点
            //        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 获取前一个位置
            //        newU(i, j) = mGrid.getVelocityX(lastPos); // 插值计算新的x方向速度
            //    }

            //    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
            //        glm::vec2 pt = mGrid.getBottom(i, j); // 获取下边界点
            //        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // 获取前一个位置
            //        newV(i, j) = mGrid.getVelocityY(lastPos); // 插值计算新的y方向速度
            //    }
            //}

            mGrid.mU = newU;
            mGrid.mV = newV;

            // 更新网格数据
            mGrid.mD = newDensity;
            mGrid.mT = newTempeture;

        }

        //施加外力的影响
        void external_forces(MACGrid2d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_LINE{
                glm::vec2 center=mGrid.getCenter(i, j);
                //glm::vec2 center(i*mGrid.cellSize, j* mGrid.cellSize);
                mGrid.mV(i, j) += mGrid.getBoussinesqForce(center) * Lagrangian2dPara::dt;
            }
        }

        //施加重力的影响
        void projection(MACGrid2d& mGrid) {
            int iterations = 50; // 最大迭代次数
            double dt = Lagrangian2dPara::dt; // 时间步长
            double dx = mGrid.cellSize; // 空间步长
            double p = 1.3;//空气密度
            double alpha = -p * dx * dx / dt; // 迭代系数
            double tolerance = 1e-6; // 收敛阈值

            std::vector<std::vector<double>> pressure(Eulerian2dPara::theDim2d[MACGrid2d::X],
                std::vector<double>(Eulerian2dPara::theDim2d[MACGrid2d::Y], 0.0));

            for (int k = 0; k < iterations; k++) {
                double maxResidual = 0.0;
                std::vector<std::vector<double>> newPressure = pressure;
#pragma omp parallel for num_threads(NUM_THREADS)
                for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y] + 1; j++)
                    for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X] + 1; i++) {
                    if (mGrid.isSolidCell(i, j)) continue; // 跳过固体单元
                    double div = mGrid.checkDivergence(i, j); // 获取当前位置的散度
                    double p_neighbors = 0.0;
                    int num_neighbors = 0;

                    // 计算相邻网格单元的压力值和压力系数
                    if (j + 1 < Eulerian2dPara::theDim2d[MACGrid2d::Y] && !mGrid.isSolidCell(i, j + 1)) {
                        p_neighbors += pressure[i][j + 1];
                        num_neighbors++;
                    }
                    if (j - 1 >= 0 && !mGrid.isSolidCell(i, j - 1)) {
                        p_neighbors += pressure[i][j - 1];
                        num_neighbors++;
                    }
                    if (i + 1 < Eulerian2dPara::theDim2d[MACGrid2d::X] && !mGrid.isSolidCell(i + 1, j)) {
                        p_neighbors += pressure[i + 1][j];
                        num_neighbors++;
                    }
                    if (i - 1 >= 0 && !mGrid.isSolidCell(i - 1, j)) {
                        p_neighbors += pressure[i - 1][j];
                        num_neighbors++;
                    }

                    if (num_neighbors > 0) {
                        newPressure[i][j] = (div * alpha + p_neighbors) / num_neighbors;
                    }

                    // 计算残差
                    double residual = fabs(newPressure[i][j] - pressure[i][j]);
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
            for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y]; j++) 
                for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X]; i++) {
                    //        //更新速度
                    double p_x = 0.0;
                    double p_y = 0.0;

                    if (i - 1 >= 0 && !mGrid.isSolidCell(i - 1, j)) {
                        p_x = pressure[i][j] - pressure[i - 1][j];
                    }

                    if (j - 1 >= 0 && !mGrid.isSolidCell(i, j - 1)) {
                        p_y = pressure[i][j] - pressure[i][j - 1];
                    }

                    mGrid.mU(i, j) -= dt / (p * dx) * p_x;
                    mGrid.mV(i, j) -= dt / (p * dx) * p_y;
                }
        
            //FOR_EACH_CELL{
            //    if (!mGrid.isSolidCell(i,j)) {
            //        //更新速度
            //        double p_x = 0.0;
            //        double p_y = 0.0;

            //        if (i - 1 >= 0 && !mGrid.isSolidCell(i - 1, j)) {
            //            p_x = pressure[i][j] - pressure[i - 1][j];
            //        }

            //        if (j - 1 >= 0 && !mGrid.isSolidCell(i, j - 1)) {
            //            p_y = pressure[i][j] - pressure[i][j - 1];
            //        }

            //        mGrid.mU(i, j) -= dt / (p * dx) * p_x;
            //        mGrid.mV(i, j) -= dt / (p * dx) * p_y;
            //    }
            //}

            //// 更新速度场
            //FOR_EACH_CELL{
            //    if (!mGrid.isSolidCell(i, j)) {
            //        double p_x = 0.0;
            //        double p_y = 0.0;

            //        if (i + 1 < Eulerian2dPara::theDim2d[MACGrid2d::X] && !mGrid.isSolidCell(i + 1, j)) {
            //            p_x = pressure[i + 1][j] - pressure[i][j];
            //        }
            //        if (j + 1 < Eulerian2dPara::theDim2d[MACGrid2d::Y] && !mGrid.isSolidCell(i, j + 1)) {
            //            p_y = pressure[i][j + 1] - pressure[i][j];
            //        }

            //        mGrid.mU(i, j) -= dt / (p * dx) * p_x;
            //        mGrid.mV(i, j) -= dt / (p * dx) * p_y;
            //    }
            //}
                // 输出压力场
            //outputPressureField(pressure);
        }

        void diffusion(MACGrid2d& mGrid, float alpha = 0.01) {
            // 创建新的密度网格数据用于更新
            Glb::CubicGridData2d newDensity = mGrid.mD;

            // 遍历网格的每一个点
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{ // i和j的设计应该保证了对应位置访问的正确性
                if (i > 0 && !mGrid.isSolidCell(i - 1, j)) {
                    newDensity(i - 1, j) += alpha * mGrid.mD(i, j);
                }
                if (i < mGrid.dim[0] - 1 && !mGrid.isSolidCell(i + 1, j)) {
                    newDensity(i + 1, j) += alpha * mGrid.mD(i, j);
                }
                if (j > 0 && !mGrid.isSolidCell(i, j - 1)) {
                    newDensity(i, j - 1) += alpha * mGrid.mD(i, j);
                }
                if (j < mGrid.dim[1] - 1 && !mGrid.isSolidCell(i, j + 1)) {
                    newDensity(i, j + 1) += alpha * mGrid.mD(i, j);
                }

                // 保持当前网格的密度，减去扩散出去的部分
                newDensity(i, j) -= 4 * alpha * mGrid.mD(i, j);
            }

                // 更新网格数据
            mGrid.mD = newDensity;
        }




        //构建求解器
        Solver::Solver(MACGrid2d& grid) : mGrid(grid) {
            mGrid.reset();

            mGrid.updateSources();

            /*glm::vec2 pot = mGrid.getCenter(10, 10);
            cout << pot.x << "\t" << pot.y << endl;
            test(mGrid);*/
        }

        void Solver::solve()
        {

            //return;

            advection(mGrid);

            //test(mGrid);

            external_forces(mGrid);

            //test(mGrid);

            //此处是否需要有一个扩散步骤

            projection(mGrid);

            //test(mGrid);

            //diffusion(mGrid);
            
        }
    }
}
