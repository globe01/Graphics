#include "Eulerian/include/Solver.h"
#include "Configure.h"
#include<omp.h>
#define NUM_THREADS 0
using namespace std;
namespace FluidSimulation
{
    namespace Eulerian2d
    {

        //��Ԫ����
        void test(MACGrid2d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{
                if (mGrid.mV(i,j) != 0 || mGrid.mU(i,j) != 0)cout << i << "\t" << j << "\t" << mGrid.mV(i,j) << "\t" << mGrid.mU(i,j) << endl;
            }
        }

        void advection(MACGrid2d& mGrid) {
            // �������ܶȺ��¶���������
            Glb::CubicGridData2d newDensity = mGrid.mD;
            Glb::CubicGridData2d newTempeture = mGrid.mT;

            //��������ж���

            Glb::GridData2dX newU = mGrid.mU;
            Glb::GridData2dY newV = mGrid.mV;

            // ���д����ٶȳ��Ķ�������
#pragma omp parallel for num_threads(NUM_THREADS)
            for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y] + 1; j++)
                for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X] + 1; i++) {
                    glm::vec2 pt = mGrid.getCenter(i, j); // ��ȡ��ǰ��Ԫ���ĵ�
                    glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ͨ�����������շ�����ȡǰһ��λ��
                    newDensity(i, j) = mGrid.mD.interpolate(lastPos); // ��ֵ�����µ��ܶ�ֵ
                    newTempeture(i, j) = mGrid.mT.interpolate(lastPos); // ��ֵ�����µ��¶�ֵ

                    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
                        glm::vec2 pt = mGrid.getLeft(i, j); // ��ȡ��߽��
                        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ��ȡǰһ��λ��
                        newU(i, j) = mGrid.getVelocityX(lastPos); // ��ֵ�����µ�x�����ٶ�
                    }

                    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
                        glm::vec2 pt = mGrid.getBottom(i, j); // ��ȡ�±߽��
                        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ��ȡǰһ��λ��
                        newV(i, j) = mGrid.getVelocityY(lastPos); // ��ֵ�����µ�y�����ٶ�
                    }
                }

            //FOR_EACH_LINE{//i��j�����Ӧ�ñ�֤�˶�Ӧλ�÷��ʵ���ȷ��

            //    glm::vec2 pt = mGrid.getCenter(i, j); // ��ȡ��ǰ��Ԫ���ĵ�
            //    glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ͨ�����������շ�����ȡǰһ��λ��
            //    newDensity(i, j) = mGrid.mD.interpolate(lastPos); // ��ֵ�����µ��ܶ�ֵ
            //    newTempeture(i, j) = mGrid.mT.interpolate(lastPos); // ��ֵ�����µ��¶�ֵ

            //    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
            //        glm::vec2 pt = mGrid.getLeft(i, j); // ��ȡ��߽��
            //        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ��ȡǰһ��λ��
            //        newU(i, j) = mGrid.getVelocityX(lastPos); // ��ֵ�����µ�x�����ٶ�
            //    }

            //    if (i < mGrid.dim[0] && j < mGrid.dim[1]) {
            //        glm::vec2 pt = mGrid.getBottom(i, j); // ��ȡ�±߽��
            //        glm::vec2 lastPos = mGrid.semiLagrangian(pt, Lagrangian2dPara::dt); // ��ȡǰһ��λ��
            //        newV(i, j) = mGrid.getVelocityY(lastPos); // ��ֵ�����µ�y�����ٶ�
            //    }
            //}

            mGrid.mU = newU;
            mGrid.mV = newV;

            // ������������
            mGrid.mD = newDensity;
            mGrid.mT = newTempeture;

        }

        //ʩ��������Ӱ��
        void external_forces(MACGrid2d& mGrid) {
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_LINE{
                glm::vec2 center=mGrid.getCenter(i, j);
                //glm::vec2 center(i*mGrid.cellSize, j* mGrid.cellSize);
                mGrid.mV(i, j) += mGrid.getBoussinesqForce(center) * Lagrangian2dPara::dt;
            }
        }

        //ʩ��������Ӱ��
        void projection(MACGrid2d& mGrid) {
            int iterations = 50; // ����������
            double dt = Lagrangian2dPara::dt; // ʱ�䲽��
            double dx = mGrid.cellSize; // �ռ䲽��
            double p = 1.3;//�����ܶ�
            double alpha = -p * dx * dx / dt; // ����ϵ��
            double tolerance = 1e-6; // ������ֵ

            std::vector<std::vector<double>> pressure(Eulerian2dPara::theDim2d[MACGrid2d::X],
                std::vector<double>(Eulerian2dPara::theDim2d[MACGrid2d::Y], 0.0));

            for (int k = 0; k < iterations; k++) {
                double maxResidual = 0.0;
                std::vector<std::vector<double>> newPressure = pressure;
#pragma omp parallel for num_threads(NUM_THREADS)
                for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y] + 1; j++)
                    for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X] + 1; i++) {
                    if (mGrid.isSolidCell(i, j)) continue; // �������嵥Ԫ
                    double div = mGrid.checkDivergence(i, j); // ��ȡ��ǰλ�õ�ɢ��
                    double p_neighbors = 0.0;
                    int num_neighbors = 0;

                    // ������������Ԫ��ѹ��ֵ��ѹ��ϵ��
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

                    // ����в�
                    double residual = fabs(newPressure[i][j] - pressure[i][j]);
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
            for (int j = 0; j < Eulerian2dPara::theDim2d[MACGrid2d::Y]; j++) 
                for (int i = 0; i < Eulerian2dPara::theDim2d[MACGrid2d::X]; i++) {
                    //        //�����ٶ�
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
            //        //�����ٶ�
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

            //// �����ٶȳ�
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
                // ���ѹ����
            //outputPressureField(pressure);
        }

        void diffusion(MACGrid2d& mGrid, float alpha = 0.01) {
            // �����µ��ܶ������������ڸ���
            Glb::CubicGridData2d newDensity = mGrid.mD;

            // ���������ÿһ����
#pragma omp parallel for num_threads(NUM_THREADS)
            FOR_EACH_CELL{ // i��j�����Ӧ�ñ�֤�˶�Ӧλ�÷��ʵ���ȷ��
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

                // ���ֵ�ǰ������ܶȣ���ȥ��ɢ��ȥ�Ĳ���
                newDensity(i, j) -= 4 * alpha * mGrid.mD(i, j);
            }

                // ������������
            mGrid.mD = newDensity;
        }




        //���������
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

            //�˴��Ƿ���Ҫ��һ����ɢ����

            projection(mGrid);

            //test(mGrid);

            //diffusion(mGrid);
            
        }
    }
}
