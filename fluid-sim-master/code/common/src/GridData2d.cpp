#include "GridData2d.h"
#include "Configure.h"
//GridData2d 类是一个用于表示二维网格数据的基类
namespace Glb
{

    // 二维网格数据基类的默认构造函数
    GridData2d::GridData2d() : mDfltValue(0.0), mMax(0.0, 0.0), cellSize(Eulerian2dPara::theCellSize2d)
    {
        // 设置网格数据的维度
        dim[0] = Eulerian2dPara::theDim2d[0];
        dim[1] = Eulerian2dPara::theDim2d[1];
    }

    // 二维网格数据基类的拷贝构造函数
    GridData2d::GridData2d(const GridData2d& orig) : mDfltValue(orig.mDfltValue)
    {
        // 复制网格数据
        mData = orig.mData;
        // 复制最大值
        mMax = orig.mMax;
        // 复制单元格大小
        cellSize = orig.cellSize;
        // 复制维度
        dim[0] = orig.dim[0];
        dim[1] = orig.dim[1];
    }

    // 二维网格数据基类的析构函数
    GridData2d::~GridData2d()
    {
        // 空实现
    }

    // 获取网格数据
    ublas::vector<double>& GridData2d::data()
    {
        return mData;
    }

    // 二维网格数据基类的赋值运算符重载函数
    GridData2d& GridData2d::operator=(const GridData2d& orig)
    {
        // 检查自赋值情况
        if (this == &orig)
        {
            return *this;
        }
        // 复制网格数据
        mData = orig.mData;
        // 复制最大值
        mMax = orig.mMax;
        // 复制单元格大小
        cellSize = orig.cellSize;
        // 复制维度
        dim[0] = orig.dim[0];
        dim[1] = orig.dim[1];
        return *this;
    }


    // 初始化网格数据
    void GridData2d::initialize(double dfltValue)
    {
        // 设置默认值
        mDfltValue = dfltValue;
        // 计算网格数据的最大值
        mMax[0] = cellSize * dim[0];
        mMax[1] = cellSize * dim[1];
        // 调整网格数据的大小
        mData.resize(dim[0] * dim[1], false);
        // 将网格数据初始化为默认值
        std::fill(mData.begin(), mData.end(), mDfltValue);
    }

    // 访问网格数据中指定位置的元素
    double& GridData2d::operator()(int i, int j)
    {
        static double dflt = 0; // 默认值
        dflt = mDfltValue; // HACK: Protect against setting the default value

        // 检查索引是否有效，若无效则返回默认值
        if (i < 0 || j < 0 ||
            i > dim[0] - 1 ||
            j > dim[1] - 1)
            return dflt;

        //获取网格内部的点，100-1=99

        int col = i;
        int row = j * dim[0];

        return mData(col + row); // 返回指定位置的网格数据元素
    }

    // 根据世界坐标获取对应网格的行列索引
    void GridData2d::getCell(const glm::vec2& pt, int& i, int& j)
    {
        glm::vec2 pos = worldToSelf(pt); // 将世界坐标转换为网格内部坐标
        // 计算网格行列索引
        i = (int)(pos[0] / cellSize);
        j = (int)(pos[1] / cellSize);
    }


    double GridData2d::interpolate(const glm::vec2 &pt)
    {
        glm::vec2 pos = worldToSelf(pt);

        int i = (int)(pos[0] / cellSize);
        int j = (int)(pos[1] / cellSize);

        double scale = 1.0 / cellSize;
        double fractx = scale * (pos[0] - i * cellSize);
        double fracty = scale * (pos[1] - j * cellSize);

        assert(fractx < 1.0 && fractx >= 0);
        assert(fracty < 1.0 && fracty >= 0);

        double tmp1 = (*this)(i, j);
        double tmp2 = (*this)(i, j + 1);
        double tmp3 = (*this)(i + 1, j);
        double tmp4 = (*this)(i + 1, j + 1);

        double tmp12 = LERP(tmp1, tmp2, fracty);
        double tmp34 = LERP(tmp3, tmp4, fracty);

        double tmp = LERP(tmp12, tmp34, fractx);
        return tmp;
    }

    glm::vec2 GridData2d::worldToSelf(const glm::vec2 &pt) const
    {
        glm::vec2 out;
        out[0] = min(max(0.0, pt[0] - cellSize * 0.5), mMax[0]);
        out[1] = min(max(0.0, pt[1] - cellSize * 0.5), mMax[1]);
        return out;
    }

    GridData2dX::GridData2dX() : GridData2d()
    {
    }

    GridData2dX::~GridData2dX()
    {
    }

    void GridData2dX::initialize(double dfltValue)
    {
        GridData2d::initialize(dfltValue);
        mMax[0] = cellSize * (dim[0] + 1); // plus one
        mMax[1] = cellSize * dim[1];
        mData.resize((dim[0] + 1) * dim[1], false);
        std::fill(mData.begin(), mData.end(), mDfltValue);
    }

    double &GridData2dX::operator()(int i, int j)
    {
        static double dflt = 0;
        dflt = mDfltValue;

        //获取边界的速度
        if (i < 0 || i > dim[0])
            return dflt;

        //i可以取到100，j所有大于dim[1]-1的都赋值为dim[1]-1
        if (j < 0)
            j = 0;
        if (j > dim[1] - 1)
            j = dim[1] - 1;

        int col = i;
        int row = j * (dim[0] + 1);

        return mData(row + col);
    }

    glm::vec2 GridData2dX::worldToSelf(const glm::vec2 &pt) const
    {
        glm::vec2 out;
        out[0] = min(max(0.0, pt[0]), mMax[0]);
        out[1] = min(max(0.0, pt[1] - cellSize * 0.5), mMax[1]);
        return out;
    }

    GridData2dY::GridData2dY() : GridData2d()
    {
    }

    GridData2dY::~GridData2dY()
    {
    }

    void GridData2dY::initialize(double dfltValue)
    {
        GridData2d::initialize(dfltValue);
        mMax[0] = cellSize * dim[0];
        mMax[1] = cellSize * (dim[1] + 1);
        mData.resize(dim[0] * (dim[1] + 1), false);
        std::fill(mData.begin(), mData.end(), mDfltValue);
    }

    double &GridData2dY::operator()(int i, int j)
    {
        static double dflt = 0;
        dflt = mDfltValue; // Protect against setting the default value

        if (j < 0 || j > dim[1])
            return dflt;

        if (i < 0)
            i = 0;
        if (i > dim[0] - 1)
            i = dim[0] - 1;

        int col = i;
        int row = j * dim[0];

        return mData(row + col);
    }

    glm::vec2 GridData2dY::worldToSelf(const glm::vec2 &pt) const
    {
        glm::vec2 out;
        out[0] = min(max(0.0, pt[0] - cellSize * 0.5), mMax[0]);
        out[1] = min(max(0.0, pt[1]), mMax[1]);
        return out;
    }

    CubicGridData2d::CubicGridData2d() : GridData2d()
    {
    }

    CubicGridData2d::CubicGridData2d(const CubicGridData2d &orig) : GridData2d(orig)
    {
    }

    CubicGridData2d::~CubicGridData2d()
    {
    }

    //三次插值？
    double CubicGridData2d::cubic(double q1, double q2, double q3, double q4, double t)
    {
        double deltaq = q3 - q2;
        double d1 = (q3 - q1) * 0.5;
        double d2 = (q4 - q2) * 0.5;

        // Force monotonic: if d1/d2 differ in sign to deltaq, make it zero
        if (deltaq > 0.0001)
        {
            d1 = d1 > 0 ? d1 : 0.0;
            d2 = d2 > 0 ? d2 : 0.0;
        }
        else if (deltaq < 0.0001)
        {
            d1 = d1 < 0 ? d1 : 0.0;
            d2 = d2 < 0 ? d2 : 0.0;
        }

        double tmp = q2 + d1 * t + (3 * deltaq - 2 * d1 - d2) * t * t + (-2 * deltaq + d1 + d2) * t * t * t;
        return tmp;
    }

    //下面都是插值？
    double CubicGridData2d::interpY(int i, int j, double fracty)
    {
        double tmp1 = (*this)(i, j - 1 < 0 ? j : j - 1);
        double tmp2 = (*this)(i, j);
        double tmp3 = (*this)(i, j + 1);
        double tmp4 = (*this)(i, j + 2);
        return cubic(tmp1, tmp2, tmp3, tmp4, fracty);
    }

    double CubicGridData2d::interpX(int i, int j, double fracty, double fractx)
    {
        double tmp1 = interpY(i - 1 < 0 ? i : i - 1, j, fracty); // hack
        double tmp2 = interpY(i, j, fracty);
        double tmp3 = interpY(i + 1, j, fracty);
        double tmp4 = interpY(i + 2, j, fracty);
        return cubic(tmp1, tmp2, tmp3, tmp4, fractx);
    }

    double CubicGridData2d::interpolate(const glm::vec2 &pt)
    {
        // Bicubic Interpolation
        glm::vec2 pos = worldToSelf(pt);

        int i = (int)(pos[0] / cellSize);
        int j = (int)(pos[1] / cellSize);

        double scale = 1.0 / cellSize;
        double fractx = scale * (pos[0] - i * cellSize);
        double fracty = scale * (pos[1] - j * cellSize);

        assert(fractx < 1.0 && fractx >= 0);
        assert(fracty < 1.0 && fracty >= 0);

        double tmp = interpX(i, j, fracty, fractx);
        return tmp;

        // Bilinear Interpolation
        /*
        float x = (pos[0] / theCellSize2d);
        float y = (pos[1] / theCellSize2d);

        i = (int)(x);
        j = (int)(y);

        float s1 = x - i; float t1 = y - j;
        float s0 = 1.0 - s1; float t0 = 1.0 - t1;

        double tmp = s0 * t0 * (*this)(i, j) + s0 * t1 * (*this)(i, j + 1) + s1 * t0 * (*this)(i + 1, j) + s1 * t1 * (*this)(i + 1, j + 1);

        return tmp;
        */
    }
}
