#include "Configure.h"

// TODO(optional)
// add yor parameters

// system 
int imageWidth = 600;       // 渲染分辨率
int imageHeight = 600;

int windowWidth = 1080;     // 窗口默认分辨率
int windowHeight = 960;

float fontSize = 16.0f;     // 字体大小

bool simulating = false;    // 模拟状态，求解中或暂停


// 2d 欧拉描述所用到的参数
namespace Eulerian2dPara
{
    // MAC grid
    int theDim2d[2] = {100, 100};   // 网格维度
    float theCellSize2d = 0.5;
    
    // 用于释放烟雾的网格
    std::vector<SourceSmoke> source = {
        {   // 网格坐标                      // 网格初始速度       //密度 // 温度
            glm::ivec2(theDim2d[0] / 3, 0), glm::vec2(0.0f, 1.0f), 1.0f, 1.0f
        }
    };

    bool addSolid = true;

    // renderer settings
    float contrast = 1;     // 调整对比度
    int drawModel = 0;
    int gridNum = theDim2d[0];

    // solver settings
    float dt = 0.01;
    float airDensity = 1.3;         // 空气密度
    float ambientTemp = 0.0;        // 环境温度
    float boussinesqAlpha = 500.0;  // Boussinesq Force 公式中的 alpha 参数
    float boussinesqBeta = 2500.0;  // Boussinesq Force 公式中的 beta 参数
}

namespace Eulerian3dPara
{
    // MAC grid
    int theDim3d[3] = {12, 36, 36}; // 请确保 x <= y = z——单元网格数
    float theCellSize3d = 0.5;//网格单元的尺寸
    std::vector<SourceSmoke> source = {
        {glm::ivec3(theDim3d[0] / 2, theDim3d[1] / 2, 0), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f}
    };//用于指定烟雾源位置、速度（包含发现），后面两个不重要（一个温度一个密度，忘了哪个是哪个了）
    bool addSolid = true;//指示是否添加固体边界

    // renderer settings
    float contrast = 1;//渲染器的对比度
    bool oneSheet = true;//是否只绘制一个切片
    float distanceX = 0.51;
    float distanceY = 0.51;
    float distanceZ = 0.985;//切片之间的距离
    bool xySheetsON = true;
    bool yzSheetsON = true;
    bool xzSheetsON = true;//是否启用 XY、YZ 和 XZ 平面的切片
    int drawModel = 0;// 绘制模式
    int gridNumX = (int)((float)theDim3d[0] / theDim3d[2] * 100);
    int gridNumY = (int)((float)theDim3d[1] / theDim3d[2] * 100);
    int gridNumZ = 100;// 网格划分的数量
    int xySheetsNum = 3;//XY、YZ 和 XZ 平面的切片数量
    int yzSheetsNum = 3;
    int xzSheetsNum = 3;

    // solver settings
    float dt = 0.01;//时间步长
    float airDensity = 1.3;//空气密度
    float ambientTemp = 0.0;//环境温度
    float boussinesqAlpha = 500.0;//参数用于描述密度随温度变化的速率,单位温度变化引起的密度变化率
    float boussinesqBeta = 2500.0;//这个参数用于描述重力对密度变化的影响。
    //它表示单位高度变化引起的密度变化率。
    //较大的 boussinesqBeta 表示密度随高度变化的速率更快，
    //即密度随深度变化的影响更为显著
}

// 2d 拉格朗日描述所用到的参数
namespace Lagrangian2dPara
{
    // scale: 各维度扩大scale倍
    // 通过调整scale的大小，放大容器与流体块大小，从而增加粒子数——事实证明只能增加粒子数
    float scale = 2;

    // 初始化用到的流体块
    std::vector<FluidBlock> fluidBlocks = {
        {   // 左下角坐标             // 右上角坐标           // 初速度              // 粒子之间的间隔
            glm::vec2(-0.4f, -0.4f), glm::vec2(0.4f, 0.4f), glm::vec2(0.0f, 0.0f), 0.02f
        }
    };

    // solver settings
    float dt = 0.0016;                  //时间步长
    int substep = 1;
    float maxVelocity = 10;             // 粒子最大速度
    float velocityAttenuation = 0.7;    // 碰到到边界后的速度衰减系数
    float eps = 1e-5;                   // 一个极小的距离，用于边界检测，防止粒子脱离容器

    // particle system
    float supportRadius = 0.04;     // 在此半径内的其他粒子会对当前粒子产生影响
    float particleRadius = 0.01;    // 粒子的半径
    float particleDiameter = particleRadius * 2.0;//例子的直径
    float gravityX = 0.0f;          // x轴上的加速度
    float gravityY = 9.8f;          // y轴上的加速度——m/s^2
    float density = 1000.0f;        // 粒子密度——1000kg/m^3
    float stiffness = 70.0f;        // 刚度_牛顿每平方米（N/m^2）
    float exponent = 7.0f;          // 压力计算公式的指数
    float viscosity = 0.03f;        // 粘度
}

namespace Lagrangian3dPara
{
    // scale
    float scale = 1.2;
    std::vector<FluidBlock> fluidBlocks = {
        {
            glm::vec3(0.05, 0.05, 0.3), glm::vec3(0.45, 0.45, 0.7), glm::vec3(0.0, 0.0, -1.0), 0.02f
        },
        {
            glm::vec3(0.45, 0.45, 0.3), glm::vec3(0.85, 0.85, 0.7), glm::vec3(0.0, 0.0, -1.0), 0.02f
        }   
    };
    
    // solver settings
    float dt = 0.002;
    int substep = 1;
    float maxVelocity = 10;
    float velocityAttenuation = 0.7;
    float eps = 1e-5;

    // particle system
    float supportRadius = 0.04;
    float particleRadius = 0.01;
    float particleDiameter = particleRadius * 2.0;
    float gravityX = 0.0f;
    float gravityY = 0.0f;
    float gravityZ = 9.8f;
    float density = 1000.0f;
    float stiffness = 20.0f;
    float exponent = 7.0f;
    float viscosity = 8e-5f;
}

// stores system's all components
std::vector<Glb::Component *> methodComponents;

std::string shaderPath = "../../../../code/resources/shaders";
std::string picturePath = "../../../../code/resources/pictures";