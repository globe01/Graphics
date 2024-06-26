#include "MACGrid2d.h"
#include "Configure.h"
#include <math.h>
#include <map>
#include <stdio.h>

namespace FluidSimulation
{
    namespace Eulerian2d
    {
        //------------------------赋值和重载运算符-----------------------------------
        MACGrid2d::MACGrid2d()
        {
            cellSize = Eulerian2dPara::theCellSize2d;
            dim[0] = Eulerian2dPara::theDim2d[0];
            dim[1] = Eulerian2dPara::theDim2d[1];
            initialize();
        }

        MACGrid2d::MACGrid2d(const MACGrid2d &orig)
        {
            mU = orig.mU;
            mV = orig.mV;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;
        }

        MACGrid2d &MACGrid2d::operator=(const MACGrid2d &orig)
        {
            if (&orig == this)
            {
                return *this;
            }
            mU = orig.mU;
            mV = orig.mV;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;

            return *this;
        }

        MACGrid2d::~MACGrid2d()
        {
        }

        void MACGrid2d::reset()
        {
            mU.initialize(0.0);
            mV.initialize(0.0);
            mD.initialize(0.0);
            mT.initialize(Eulerian2dPara::ambientTemp);
        }

        //设置固体场
        void MACGrid2d::createSolids()
        {
            mSolid.initialize();
            if (Eulerian2dPara::addSolid) {
                int j = dim[1] / 2;
                for (int i = dim[0] / 4; i < dim[0] * 3 / 4; i++) {
                    mSolid(i, j) = 1;
                }
            }
        }

        // 更新网格中的源点信息
        void MACGrid2d::updateSources()
        {
            for (int i = 0; i < Eulerian2dPara::source.size(); i++) {
                int x = Eulerian2dPara::source[i].position.x;
                int y = Eulerian2dPara::source[i].position.y;
                mT(x, y) = Eulerian2dPara::source[i].temp;
                mD(x, y) = Eulerian2dPara::source[i].density;
                mU(x, y) = Eulerian2dPara::source[i].velocity.x;
                mV(x, y) = Eulerian2dPara::source[i].velocity.y;
                //std::cout << x << "\t" << y << std::endl;
            }
        }

        void MACGrid2d::initialize()
        {
            reset();
            createSolids();
            assert(checkDivergence());
        }

        // 根据温度差异计算 Boussinesq Force
        double MACGrid2d::getBoussinesqForce(const glm::vec2 &pos)
        {
            // f = [0, -alpha*smokeDensity + beta*(T - T_amb), 0]
            double temperature = getTemperature(pos);
            double smokeDensity = getDensity(pos);

            double yforce = -Eulerian2dPara::boussinesqAlpha * smokeDensity +
                            Eulerian2dPara::boussinesqBeta * (temperature - Eulerian2dPara::ambientTemp);

            return yforce;
        }


        // 计算散度_速度的散度
        double MACGrid2d::getDivergence(int i, int j)
        {

            double x1 = isSolidCell(i + 1, j) ? 0.0 : mU(i + 1, j);
            double x0 = isSolidCell(i - 1, j) ? 0.0 : mU(i, j);

            double y1 = isSolidCell(i, j + 1) ? 0.0 : mV(i, j + 1);
            double y0 = isSolidCell(i, j - 1) ? 0.0 : mV(i, j);

            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double div = (xdiv + ydiv) / cellSize;

            return div;
        }

        double MACGrid2d::checkDivergence(int i, int j)
        {
            double x1 = mU(i + 1, j);
            double x0 = mU(i, j);
            double y1 = mV(i, j + 1);
            double y0 = mV(i, j);
            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double div = (xdiv + ydiv) / cellSize;
            return div;
        }

        bool MACGrid2d::checkDivergence()
        {
            FOR_EACH_CELL
            {
                double div = checkDivergence(i, j);
                if (fabs(div) > 0.01)
                {
                    return false;
                }
            }
            return true;
        }

        // 使用半拉格朗日方法对给定点进行运动更新
        glm::vec2 MACGrid2d::semiLagrangian(const glm::vec2& pt, double dt)
        {
            // 获取给定点处的速度
            glm::vec2 vel = getVelocity(pt);

            // 根据速度进行位置更新
            glm::vec2 pos = pt - vel * (float)dt;

            // 限制更新后的位置在网格边界内
            pos[0] = max(0.0, min((dim[0] - 1) * cellSize, pos[0])); // 将 x 坐标限制在 [0, (dim[0] - 1) * cellSize] 范围内
            pos[1] = max(0.0, min((dim[1] - 1) * cellSize, pos[1])); // 将 y 坐标限制在 [0, (dim[1] - 1) * cellSize] 范围内

            int i, j;
            // 如果更新后的位置位于固体内部，则进行修正
            if (inSolid(pt, i, j))
            {
                double t = 0;
                // 计算速度线段与固体边界的交点，并返回交点处的时间 t
                if (intersects(pt, vel, i, j, t))
                {
                    pos = pt - vel * (float)t; // 更新位置为交点处的位置
                }
                else
                {
                    // 如果计算交点出现问题，则输出错误信息
                    Glb::Logger::getInstance().addLog("Error: something goes wrong during advection");
                }
            }

            return pos; // 返回更新后的位置
        }


        // 获取相交的具体时间
       // 判断射线与固体边界是否相交，并计算交点处的时间
        bool MACGrid2d::intersects(const glm::vec2& wPos, const glm::vec2& wDir, int i, int j, double& time)
        {
            // 获取当前网格单元的中心位置
            glm::vec2 pos = getCenter(i, j);

            // 计算从射线起点到网格中心的向量
            glm::vec2 rayStart = wPos - pos;
            // 设置射线方向
            glm::vec2 rayDir = wDir;

            // 初始化射线的最小和最大时间
            double tmin = -9999999999.0;
            double tmax = 9999999999.0;

            // 设置固体单元的最小和最大范围
            double min = -0.5 * cellSize;
            double max = 0.5 * cellSize;

            // 遍历射线的各个维度
            for (int i = 0; i < 3; i++)
            {
                // 计算当前维度的起点坐标和方向值
                double e = rayStart[i];
                double f = rayDir[i];

                // 如果射线方向值不为零
                if (fabs(f) > 0.000000001)
                {
                    // 计算射线与当前维度范围的交点时间
                    double t1 = (min - e) / f;
                    double t2 = (max - e) / f;

                    // 保证 t1 小于 t2
                    if (t1 > t2)
                        std::swap(t1, t2);

                    // 更新射线的最小和最大时间
                    if (t1 > tmin)
                        tmin = t1;
                    if (t2 < tmax)
                        tmax = t2;

                    // 如果最小时间大于最大时间，则射线与固体边界没有相交
                    if (tmin > tmax)
                        return false;

                    // 如果最大时间小于零，则射线与固体边界没有相交
                    if (tmax < 0)
                        return false;
                }
                // 如果射线方向值为零，且射线起点不在当前维度的范围内，则射线与固体边界没有相交
                else if (e < min || e > max)
                    return false;
            }

            // 根据最小时间的值确定交点处的时间
            if (tmin >= 0)
            {
                time = tmin;
                return true;
            }
            else
            {
                time = tmax;
                return true;
            }

            return false;
        }



        // 根据网格索引(i, j)获取对应的单元格索引
        int MACGrid2d::getIndex(int i, int j)
        {
            // 检查索引是否超出边界
            if (i < 0 || i > dim[0] - 1)
                return -1;
            if (j < 0 || j > dim[1] - 1)
                return -1;

            // 计算单元格的列和行索引
            int col = i;
            int row = j * dim[0];
            // 返回单元格索引
            return col + row;
        }

        // 根据单元格索引获取对应的网格索引(i, j)
        void MACGrid2d::getCell(int index, int& i, int& j)
        {
            // 计算行索引
            j = (int)index / dim[0]; // row
            // 计算列索引
            i = index - j * dim[0];  // col
        }

        // 根据网格索引(i, j)获取对应单元格的中心位置
        glm::vec2 MACGrid2d::getCenter(int i, int j)
        {
            // 计算单元格左下角点的坐标
            double xstart = cellSize / 2.0;//0.5/2+i*0.5
            double ystart = cellSize / 2.0;

            // 计算单元格中心点的坐标
            double x = xstart + i * cellSize;
            double y = ystart + j * cellSize;

            // 返回单元格中心点的坐标向量
            return glm::vec2(x, y);
        }


        glm::vec2 MACGrid2d::getLeft(int i, int j)
        {
            return getCenter(i, j) - glm::vec2(cellSize * 0.5, 0.0);
        }

        glm::vec2 MACGrid2d::getRight(int i, int j)
        {
            return getCenter(i, j) + glm::vec2(cellSize * 0.5, 0.0);
        }

        glm::vec2 MACGrid2d::getTop(int i, int j)
        {
            return getCenter(i, j) + glm::vec2(0.0, cellSize * 0.5);
        }

        glm::vec2 MACGrid2d::getBottom(int i, int j)
        {
            return getCenter(i, j) - glm::vec2(0.0, cellSize * 0.5);
        }

        glm::vec2 MACGrid2d::getVelocity(const glm::vec2 &pt)
        {
            if (inSolid(pt))
            {
                return glm::vec2(0, 0);
            }

            glm::vec2 vel;
            vel[0] = getVelocityX(pt);
            vel[1] = getVelocityY(pt);
            return vel;
        }

        double MACGrid2d::getVelocityX(const glm::vec2 &pt)
        {
            return mU.interpolate(pt);
        }

        double MACGrid2d::getVelocityY(const glm::vec2 &pt)
        {
            return mV.interpolate(pt);
        }

        double MACGrid2d::getTemperature(const glm::vec2 &pt)
        {
            return mT.interpolate(pt);
        }

        double MACGrid2d::getDensity(const glm::vec2 &pt)
        {
            return mD.interpolate(pt);
        }

        int MACGrid2d::numSolidCells()
        {
            int numSolid = 0;
            FOR_EACH_CELL { numSolid += mSolid(i, j); }
            return numSolid;
        }

        bool MACGrid2d::inSolid(const glm::vec2 &pt)
        {
            int i, j, k;
            mSolid.getCell(pt, i, j);
            return isSolidCell(i, j) == 1;
        }

        bool MACGrid2d::inSolid(const glm::vec2 &pt, int &i, int &j)
        {
            mSolid.getCell(pt, i, j);
            return isSolidCell(i, j) == 1;
        }

        int MACGrid2d::isSolidCell(int i, int j)
        {
            bool containerBoundary = (i < 0 || i > dim[0] - 1) ||
                                     (j < 0 || j > dim[1] - 1);

            bool objectBoundary = (mSolid(i, j) == 1);

            return containerBoundary || objectBoundary ? 1 : 0;
        }

        // 检查网格单元的面是否与实心边界相交
        // 参数i、j为网格单元的索引，d为面的方向（X方向或Y方向）
        // 返回值为1表示面与实心边界相交，否则返回0
        int MACGrid2d::isSolidFace(int i, int j, MACGrid2d::Direction d)
        {
            // 如果面位于网格边界，则与实心边界相交
            if (d == X && (i == 0 || i == dim[0]))
                return 1;
            else if (d == Y && (j == 0 || j == dim[1]))
                return 1;

            // 如果面的邻近网格单元存在实心边界，则与实心边界相交
            if (d == X && (mSolid(i, j) || mSolid(i - 1, j)))
                return 1;
            if (d == Y && (mSolid(i, j) || mSolid(i, j - 1)))
                return 1;

            // 否则，不与实心边界相交
            return 0;
        }

        // 检查网格单元(i0, j0)是否为网格单元(i1, j1)的邻居
        // 返回true表示(i0, j0)与(i1, j1)是相邻的网格单元，否则返回false
        bool MACGrid2d::isNeighbor(int i0, int j0, int i1, int j1)
        {
            // 如果网格单元的行索引或列索引之差为1，且另一个索引相等，则表示两个网格单元相邻
            if (abs(i0 - i1) == 1 && j0 == j1)
                return true;
            if (abs(j0 - j1) == 1 && i0 == i1)
                return true;
            // 否则，不相邻
            return false;
        }


        // 获取两个网格单元之间的压力系数
        // 参数i、j表示第一个网格单元的索引，pi、pj表示第二个网格单元的索引
        // 返回值为两个网格单元之间的压力系数
        double MACGrid2d::getPressureCoeffBetweenCells(int i, int j, int pi, int pj)
        {
            // 如果两个网格单元是同一个单元
            if (i == pi && j == pj) // 自身
            {
                // 计算周围非固体网格单元的数量
                int numSolidNeighbors = (isSolidCell(i + 1, j) +
                    isSolidCell(i - 1, j) +
                    isSolidCell(i, j + 1) +
                    isSolidCell(i, j - 1));
                // 返回周围非固体网格单元的数量
                return 4.0 - numSolidNeighbors; // 在二维情况下，一个网格单元周围有4个邻居
            }
            // 如果两个网格单元是邻居但后者是固体，则返回错误值-1.0
            if (isNeighbor(i, j, pi, pj) && !isSolidCell(pi, pj))
                return -1.0;
            // 如果两个网格单元不是邻居，则返回0.0
            return 0.0;
        }


        glm::vec4 MACGrid2d::getRenderColor(int i, int j)
        {
            double value = mD(i, j);
            return glm::vec4(1.0, 1.0, 1.0, value);
        }


        glm::vec4 MACGrid2d::getRenderColor(const glm::vec2 &pt)
        {
            double value = getDensity(pt);
            return glm::vec4(value, value, value, value);
        }


        // 确保在界内
        bool MACGrid2d::isValid(int i, int j, MACGrid2d::Direction d)
        {
            switch (d)
            {
            case X:
                return (i >= 0 && i < dim[X] + 1 &&
                        j >= 0 && j < dim[Y]);
            case Y:
                return (i >= 0 && i < dim[X] &&
                        j >= 0 && j < dim[Y] + 1);
            }
            Glb::Logger::getInstance().addLog("Error: bad direction");
            return false;
        }
    }
}