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
            // �������ܶȺ��¶���������
            Glb::CubicGridData3d newDensity = mGrid.mD;
            Glb::CubicGridData3d newTemperature = mGrid.mT;

            // �����µ��ٶ���������
            Glb::GridData3dX newU = mGrid.mU;
            Glb::GridData3dY newV = mGrid.mV;
            Glb::GridData3dZ newW = mGrid.mW;

            // ���д����ٶȳ��Ķ�������
#pragma omp parallel for collapse(3) num_threads(NUM_THREADS)
            for (int i = 0; i < mGrid.dim[0]; ++i) {
                for (int j = 0; j < mGrid.dim[1]; ++j) {
                    for (int k = 0; k < mGrid.dim[2]; ++k) {
                        glm::vec3 pt = mGrid.getCenter(i, j, k); // ��ȡ��ǰ��Ԫ���ĵ�
                        glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // ͨ�����������շ�����ȡǰһ��λ��
                        newDensity(i, j, k) = mGrid.mD.interpolate(lastPos); // ��ֵ�����µ��ܶ�ֵ
                        newTemperature(i, j, k) = mGrid.mT.interpolate(lastPos); // ��ֵ�����µ��¶�ֵ

                        if (i < mGrid.dim[0]) {
                            glm::vec3 pt = mGrid.getBack(i, j, k); // ��ȡ��߽��
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // ��ȡǰһ��λ��
                            newU(i, j, k) = mGrid.getVelocityX(lastPos); // ��ֵ�����µ�x�����ٶ�
                        }

                        if (j < mGrid.dim[1]) {
                            glm::vec3 pt = mGrid.getLeft(i, j, k); // ��ȡ��߽��
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // ��ȡǰһ��λ��
                            newV(i, j, k) = mGrid.getVelocityY(lastPos); // ��ֵ�����µ�y�����ٶ�
                        }

                        if (k < mGrid.dim[2]) {
                            glm::vec3 pt = mGrid.getBottom(i, j, k); // ��ȡ�±߽��
                            glm::vec3 lastPos = mGrid.semiLagrangian(pt, Lagrangian3dPara::dt); // ��ȡǰһ��λ��
                            newW(i, j, k) = mGrid.getVelocityZ(lastPos); // ��ֵ�����µ�z�����ٶ�
                        }
                    }
                }
            }

            // �����ٶȳ�
            mGrid.mU = newU;
            mGrid.mV = newV;
            mGrid.mW = newW;

            // ������������
            mGrid.mD = newDensity;
            mGrid.mT = newTemperature;
        }

        //ʩ��������Ӱ��
        void external_forces(MACGrid3d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_FACE{
                glm::vec3 center = mGrid.getCenter(i, j,k);
                //glm::vec2 center(i*mGrid.cellSize, j* mGrid.cellSize);
                mGrid.mW(i, j,k) += mGrid.getBoussinesqForce(center) * Lagrangian3dPara::dt;
            }
        }

        void projection(MACGrid3d& mGrid) {
            int iterations = 50; // ����������
            double dt = Lagrangian3dPara::dt; // ʱ�䲽��
            double dx = mGrid.cellSize; // �ռ䲽��
            double p = 1.3; // �����ܶ�
            double alpha = -p * dx * dx * dx / dt; // ����ϵ��
            double tolerance = 1e-6; // ������ֵ

            std::vector<std::vector<std::vector<double>>> pressure(Eulerian3dPara::theDim3d[MACGrid3d::X],
                std::vector<std::vector<double>>(Eulerian3dPara::theDim3d[MACGrid3d::Y],
                    std::vector<double>(Eulerian3dPara::theDim3d[MACGrid3d::Z], 0.0)));

            for (int k = 0; k < iterations; k++) {
                double maxResidual = 0.0;
                std::vector<std::vector<std::vector<double>>> newPressure = pressure;

                FOR_EACH_CELL{
                    if (mGrid.isSolidCell(i, j, k)) continue; // �������嵥Ԫ
                    double div = mGrid.checkDivergence(i, j, k); // ��ȡ��ǰλ�õ�ɢ��
                    double p_neighbors = 0.0;
                    int num_neighbors = 0;

                    // ������������Ԫ��ѹ��ֵ��ѹ��ϵ��
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

                    // ����в�
                    double residual = fabs(newPressure[i][j][k] - pressure[i][j][k]);
                    if (residual > maxResidual) {
                        maxResidual = residual;
                    }
                }

                pressure = newPressure;

                // �����������
                if (maxResidual < tolerance) {
                    break;
                }
            }

#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{
                if (!mGrid.isSolidCell(i, j, k)) {
                    // �����ٶ�
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

                // ���ѹ����
                //outputPressureField(pressure);
        }


        Solver::Solver(MACGrid3d &grid) : mGrid(grid)//��ʼ��
        {
            mGrid.reset();
           
        }

        void Solver::solve()//ƽ������������ͶӰ�����ٶ��ڱ߽磬�¶ȡ��ܶ����ڲ�
        {
            //ƽ������

            //��ȡ���ǵ�ǰλ�õġ����ӡ���һ��ʱ�䲽��λ��
            
            advection(mGrid);

            //test(mGrid);

            external_forces(mGrid);

            //test(mGrid);

            //�˴��Ƿ���Ҫ��һ����ɢ����

            projection(mGrid);

            //test(mGrid);
        }
    }
}
