#include "MACGrid3d.h"
#include "Configure.h"
#include <math.h>
#include <map>
#include <stdio.h>

namespace FluidSimulation
{
    namespace Eulerian3d
    {

        MACGrid3d::MACGrid3d()//初始化
        {
            cellSize = Eulerian3dPara::theCellSize3d;
            dim[0] = Eulerian3dPara::theDim3d[0];
            dim[1] = Eulerian3dPara::theDim3d[1];
            dim[2] = Eulerian3dPara::theDim3d[2];
            initialize();
        }

        MACGrid3d::MACGrid3d(const MACGrid3d &orig)
        {
            mU = orig.mU;
            mV = orig.mV;
            mW = orig.mW;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;
        }

        MACGrid3d &MACGrid3d::operator=(const MACGrid3d &orig)
        {
            if (&orig == this)
            {
                return *this;
            }
            mU = orig.mU;
            mV = orig.mV;
            mW = orig.mW;
            mD = orig.mD;
            mT = orig.mT;
            mSolid = orig.mSolid;

            return *this;
        }

        MACGrid3d::~MACGrid3d()
        {
        }

        void MACGrid3d::reset()
        {
            mU.initialize(0.0);
            mV.initialize(0.0);
            mW.initialize(0.0);
            mD.initialize(0.0);
            mT.initialize(Eulerian3dPara::ambientTemp);
        }

        void MACGrid3d::updateSources()
        {
            //只初始化了一个点——烟雾源点保持不变——源源不断的烟雾
            for (int i = 0; i < Eulerian3dPara::source.size(); i++) {
                int x = Eulerian3dPara::source[i].position.x;
                int y = Eulerian3dPara::source[i].position.y;
                int z = Eulerian3dPara::source[i].position.z;
                mT(x, y, z) = Eulerian3dPara::source[i].temp;
                mD(x, y, z) = Eulerian3dPara::source[i].density;
                mU(x, y, z) = Eulerian3dPara::source[i].velocity.x;
                mV(x, y, z) = Eulerian3dPara::source[i].velocity.y;
                mW(x, y, z) = Eulerian3dPara::source[i].velocity.z;
            }
        }

        //创造一个固体
        void MACGrid3d::createSolids()
        {
            mSolid.initialize();

            if (Eulerian3dPara::addSolid) {
                for (int k = dim[2] / 2 - 2; k <= dim[2] / 2 + 2; k++) {
                    for (int j = dim[1] / 2 - 2; j <= dim[1] / 2 + 2; j++) {
                        for (int i = dim[0] / 2 - 2; i <= dim[0] / 2 + 2; i++) {
                            mSolid(i, j, k) = 1;
                        }
                    }
                }
            }

        }

        void MACGrid3d::initialize()
        {
            reset();
            createSolids();
        }

        /**
         * @brief 获取 Boussinesq 力——密度驱动的力
         *
         * @param pos 位置坐标
         * @return double Boussinesq 力
         */
        //虽然不知道这个力是啥，但是有现成的就很nice
        double MACGrid3d::getBoussinesqForce(const glm::vec3& pos)
        {
            // 获取当前位置的温度
            double temperature = getTemperature(pos);

            // 获取当前位置的烟雾密度
            double smokeDensity = getDensity(pos);

            // 计算 Boussinesq 力在 z 方向上的分量
            double zforce = -Eulerian3dPara::boussinesqAlpha * smokeDensity +
                Eulerian3dPara::boussinesqBeta * (temperature - Eulerian3dPara::ambientTemp);

            // 返回 Boussinesq 力
            return zforce;
        }

        //计算散度
        //什么散度？干啥的？不管，反正有现成的就直接用
        double MACGrid3d::checkDivergence(int i, int j, int k)
        {
            double x1 = mU(i + 1, j, k);
            double x0 = mU(i, j, k);

            double y1 = mV(i, j + 1, k);
            double y0 = mV(i, j, k);

            double z1 = mW(i, j, k + 1);
            double z0 = mW(i, j, k);

            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double zdiv = z1 - z0;
            double div = (xdiv + ydiv + zdiv) / cellSize;
            return div;
        }

        //和先前一致，高级的是判断了是否是固体！感觉是因为加不加固体前后会有差异
        double MACGrid3d::getDivergence(int i, int j, int k)
        {

            double x1 = isSolidCell(i + 1, j, k) ? 0.0 : mU(i + 1, j, k);
            double x0 = isSolidCell(i - 1, j, k) ? 0.0 : mU(i, j, k);

            double y1 = isSolidCell(i, j + 1, k) ? 0.0 : mV(i, j + 1, k);
            double y0 = isSolidCell(i, j - 1, k) ? 0.0 : mV(i, j, k);

            double z1 = isSolidCell(i, j, k + 1) ? 0.0 : mW(i, j, k + 1);
            double z0 = isSolidCell(i, j, k - 1) ? 0.0 : mW(i, j, k);

            double xdiv = x1 - x0;
            double ydiv = y1 - y0;
            double zdiv = z1 - z0;
            double div = (xdiv + ydiv + zdiv) / cellSize;

            return div;
        }

        /**
          * @brief 检查网格的散度是否满足要求
          *
          * @return true 散度满足要求
          * @return false 散度不满足要求
          */
        bool MACGrid3d::checkDivergence()
        {
            // 遍历网格的所有单元格
            FOR_EACH_CELL
            {
                // 获取当前单元格的散度值
                double div = checkDivergence(i, j, k);

                // 如果散度值超过了阈值
                if (fabs(div) > 0.01)
                {
                    // 输出散度值并返回 false
                    printf("Divergence(%d,%d,%d) = %.2f\n", i, j, k, div);
                    return false;
                }
            }

                // 散度值在阈值范围内，返回 true
            return true;
        }

        /**
         * @brief 根据给定的位置和时间步长使用半拉格朗日法进行流体粒子追踪
         *
         * @param pt 流体粒子的位置
         * @param dt 时间步长
         * @return glm::vec3 追踪后的流体粒子位置
         */
        //牛逼啊，这个是给出了实际的函数？
        glm::vec3 MACGrid3d::semiLagrangian(const glm::vec3& pt, double dt)
        {
            // 获取流体粒子在给定位置的速度
            glm::vec3 vel = getVelocity(pt);

            // 根据速度更新流体粒子的位置
            glm::vec3 pos = pt - vel * (float)dt;

            // 将更新后的位置限制在网格边界内
            pos[0] = max(0.0, min((dim[0] - 1) * cellSize, pos[0]));
            pos[1] = max(0.0, min((dim[1] - 1) * cellSize, pos[1]));
            pos[2] = max(0.0, min((dim[2] - 1) * cellSize, pos[2]));

            // 检查流体粒子是否进入固体物体内部
            int i, j, k;
            if (inSolid(pt, i, j, k))
            {
                double t = 0;
                // 如果流体粒子与固体物体发生碰撞，则根据碰撞时间更新流体粒子的位置
                if (intersects(pt, vel, i, j, k, t))
                {
                    pos = pt - vel * (float)t;
                }
                else
                {
                    // 发生碰撞时出错，输出错误信息
                    Glb::Logger::getInstance().addLog("Error: something goes wrong during advection");
                }
            }
            return pos;
        }

        /**
          * @brief 判断射线与单元格边界的相交情况，并计算相交时间
          *
          * @param wPos 射线的起点位置（世界坐标系）
          * @param wDir 射线的方向（世界坐标系）
          * @param i 单元格的索引 i
          * @param j 单元格的索引 j
          * @param k 单元格的索引 k
          * @param time 如果相交，返回相交的时间
          * @return true 如果射线与单元格相交
          * @return false 如果射线不与单元格相交
          */

        //看不懂思密达
        bool MACGrid3d::intersects(const glm::vec3& wPos, const glm::vec3& wDir, int i, int j, int k, double& time)
        {
            // 计算单元格的中心位置
            glm::vec3 pos = getCenter(i, j, k);

            // 将射线起点从世界坐标系转换到单元格局部坐标系
            glm::vec3 rayStart = wPos - pos;
            glm::vec3 rayDir = wDir;

            // 初始化相交时间范围
            double tmin = -9999999999.0;
            double tmax = 9999999999.0;

            // 计算单元格的边界范围
            double min = -0.5 * cellSize;
            double max = 0.5 * cellSize;

            // 遍历射线的三个坐标轴
            for (int i = 0; i < 3; i++)
            {
                double e = rayStart[i];
                double f = rayDir[i];

                // 如果射线方向分量不为零，则计算射线与边界的相交时间
                if (fabs(f) > 0.000000001)
                {
                    double t1 = (min - e) / f;
                    double t2 = (max - e) / f;
                    if (t1 > t2)
                        std::swap(t1, t2);
                    if (t1 > tmin)
                        tmin = t1;
                    if (t2 < tmax)
                        tmax = t2;
                    if (tmin > tmax)
                        return false;
                    if (tmax < 0)
                        return false;
                }
                // 如果射线方向分量为零，且起点不在边界范围内，则不相交
                else if (e < min || e > max)
                    return false;
            }

            // 判断相交情况并计算相交时间
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

            // 如果代码能够执行到此处，说明发生了错误，返回 false
            return false;
        }

        int MACGrid3d::getIndex(int i, int j, int k)
        {
            if (i < 0 || i > dim[0] - 1)
                return -1;
            if (j < 0 || j > dim[1] - 1)
                return -1;
            if (k < 0 || k > dim[2] - 1)
                return -1;

            int col = i;
            int row = k * dim[0];
            int stack = j * dim[0] * dim[2];
            return col + row + stack;
        }

        //高级，不知道有什么用，应该是获取index附近的i、j、k
        void MACGrid3d::getCell(int index, int &i, int &j, int &k)
        {
            j = (int)index / (dim[0] * dim[2]);
            k = (int)(index - j * dim[0] * dim[2]) / dim[0];
            i = index - j * dim[0] * dim[2] - k * dim[0];
        }

        glm::vec3 MACGrid3d::getCenter(int i, int j, int k)
        {
            double xstart = cellSize / 2.0;
            double ystart = cellSize / 2.0;
            double zstart = cellSize / 2.0;

            double x = xstart + i * cellSize;
            double y = ystart + j * cellSize;
            double z = zstart + k * cellSize;
            return glm::vec3(x, y, z);
        }

        glm::vec3 MACGrid3d::getLeft(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(0.0, cellSize * 0.5, 0.0);
        }

        glm::vec3 MACGrid3d::getRight(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(0.0, cellSize * 0.5, 0.0);
        }

        glm::vec3 MACGrid3d::getTop(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(0.0, 0.0, cellSize * 0.5);
        }

        glm::vec3 MACGrid3d::getBottom(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(0.0, 0.0, cellSize * 0.5);
        }

        glm::vec3 MACGrid3d::getFront(int i, int j, int k)
        {
            return getCenter(i, j, k) + glm::vec3(cellSize * 0.5, 0.0, 0.0);
        }

        glm::vec3 MACGrid3d::getBack(int i, int j, int k)
        {
            return getCenter(i, j, k) - glm::vec3(cellSize * 0.5, 0.0, 0.0);
        }

        glm::vec3 MACGrid3d::getVelocity(const glm::vec3 &pt)
        {
            if (inSolid(pt))
            {
                return glm::vec3(0);
            }

            glm::vec3 vel;
            vel[0] = getVelocityX(pt);
            vel[1] = getVelocityY(pt);
            vel[2] = getVelocityZ(pt);
            return vel;
        }

        double MACGrid3d::getVelocityX(const glm::vec3 &pt)
        {
            return mU.interpolate(pt);
        }

        double MACGrid3d::getVelocityY(const glm::vec3 &pt)
        {
            return mV.interpolate(pt);
        }

        double MACGrid3d::getVelocityZ(const glm::vec3 &pt)
        {
            return mW.interpolate(pt);
        }

        double MACGrid3d::getTemperature(const glm::vec3 &pt)
        {
            return mT.interpolate(pt);
        }

        double MACGrid3d::getDensity(const glm::vec3 &pt)
        {
            return mD.interpolate(pt);
        }

        int MACGrid3d::numSolidCells()
        {
            int numSolid = 0;
            FOR_EACH_CELL { numSolid += mSolid(i, j, k); }
            return numSolid;
        }

        bool MACGrid3d::inSolid(const glm::vec3 &pt)
        {
            int i, j, k;
            mSolid.getCell(pt, i, j, k);
            return isSolidCell(i, j, k) == 1;
        }

        bool MACGrid3d::inSolid(const glm::vec3 &pt, int &i, int &j, int &k)
        {
            mSolid.getCell(pt, i, j, k);
            return isSolidCell(i, j, k) == 1;
        }

        int MACGrid3d::isSolidCell(int i, int j, int k)
        {
            bool containerBoundary = (i < 0 || i > dim[0] - 1) ||
                                     (j < 0 || j > dim[1] - 1) ||
                                     (k < 0 || k > dim[2] - 1);

            bool objectBoundary = (mSolid(i, j, k) == 1);

            return containerBoundary || objectBoundary ? 1 : 0;
        }

        int MACGrid3d::isSolidFace(int i, int j, int k, MACGrid3d::Direction d)
        {
            if (d == X && (i == 0 || i == dim[0]))
                return 1;
            else if (d == Y && (j == 0 || j == dim[1]))
                return 1;
            else if (d == Z && (k == 0 || k == dim[2]))
                return 1;

            if (d == X && (mSolid(i, j, k) || mSolid(i - 1, j, k)))
                return 1;
            if (d == Y && (mSolid(i, j, k) || mSolid(i, j - 1, k)))
                return 1;
            if (d == Z && (mSolid(i, j, k) || mSolid(i, j, k - 1)))
                return 1;

            return 0;
        }

        bool MACGrid3d::isNeighbor(int i0, int j0, int k0, int i1, int j1, int k1)
        {
            if (abs(i0 - i1) == 1 && j0 == j1 && k0 == k1)
                return true;
            if (abs(j0 - j1) == 1 && i0 == i1 && k0 == k1)
                return true;
            if (abs(k0 - k1) == 1 && j0 == j1 && i0 == i1)
                return true;
            return false;
        }

        /**
         * @brief 计算两个单元格之间的压力系数
         *
         * @param i 当前单元格的 x 索引
         * @param j 当前单元格的 y 索引
         * @param k 当前单元格的 z 索引
         * @param pi 目标单元格的 x 索引
         * @param pj 目标单元格的 y 索引
         * @param pk 目标单元格的 z 索引
         * @return double 压力系数
         */
        double MACGrid3d::getPressureCoeffBetweenCells(int i, int j, int k, int pi, int pj, int pk)
        {
            // 如果当前单元格索引与目标单元格索引相同，即计算当前单元格自身的压力系数
            if (i == pi && j == pj && k == pk)
            {
                // 计算当前单元格周围的固体单元格数
                int numSolidNeighbors = (isSolidCell(i + 1, j, k) +
                    isSolidCell(i - 1, j, k) +
                    isSolidCell(i, j + 1, k) +
                    isSolidCell(i, j - 1, k) +
                    isSolidCell(i, j, k + 1) +
                    isSolidCell(i, j, k - 1));

                // 返回当前单元格的压力系数，该值等于6减去周围固体单元格的数量
                return 6.0 - numSolidNeighbors;
            }
            // 如果当前单元格索引与目标单元格索引相邻且目标单元格不是固体单元格
            else if (isNeighbor(i, j, k, pi, pj, pk) && !isSolidCell(pi, pj, pk))
            {
                // 返回压力系数为-1.0，表示相邻的两个单元格之间存在压力差
                return -1.0;
            }
            // 其他情况下，返回0.0，表示给定的两个单元格之间没有压力差
            else
            {
                return 0.0;
            }
        }

        glm::vec4 MACGrid3d::getRenderColor(int i, int j, int k)
        {
            double value = mD(i, j, k);
            return glm::vec4(1.0, 1.0, 1.0, value);
        }

        glm::vec4 MACGrid3d::getRenderColor(const glm::vec3 &pt)
        {
            double value = getDensity(pt);
            return glm::vec4(value, value, value, value);
        }

        bool MACGrid3d::isValid(int i, int j, int k, MACGrid3d::Direction d)
        {
            switch (d)
            {
            case X:
                return (i >= 0 && i < dim[X] + 1 &&
                        j >= 0 && j < dim[Y] &&
                        k >= 0 && k < dim[Z]);
            case Y:
                return (i >= 0 && i < dim[X] &&
                        j >= 0 && j < dim[Y] + 1 &&
                        k >= 0 && k < dim[Z]);
            case Z:
                return (i >= 0 && i < dim[X] &&
                        j >= 0 && j < dim[Y] &&
                        k >= 0 && k < dim[Z] + 1);
            }
            Glb::Logger::getInstance().addLog("Error: bad direction");
            return false;
        }
    }
}