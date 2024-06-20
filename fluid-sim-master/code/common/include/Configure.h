#pragma once
#ifndef __CONFIGURE_H__
#define __CONFIGURE_H__

#include <iostream>
#include <string>
#include "Component.h"
#include <vector>
#include "glm/glm.hpp"

#define LERP(a, b, t) (1 - t) * a + t *b

#ifndef __MINMAX_DEFINED
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

extern int imageWidth;
extern int imageHeight;

extern int windowWidth;
extern int windowHeight;

extern float fontSize;

extern bool simulating;

namespace Eulerian2dPara
{
    struct SourceSmoke {
        glm::ivec2 position = glm::ivec2(0);
        glm::vec2 velocity = glm::vec2(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim2d[];
    extern std::vector<SourceSmoke> source;
    extern float theCellSize2d;
    extern bool addSolid;

    extern float dt;

    extern float contrast;
    extern int drawModel;
    extern int gridNum;


    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;
}

namespace Eulerian3dPara
{
    struct SourceSmoke {
        glm::ivec3 position = glm::ivec3(0);
        glm::vec3 velocity = glm::vec3(0.0f);
        float density = 0.0f;
        float temp = 0.0f;
    };

    extern int theDim3d[];
    extern float theCellSize3d;
    extern std::vector<SourceSmoke> source;
    extern bool addSolid;

    extern float contrast;
    extern bool oneSheet;
    extern float distanceX;
    extern float distanceY;
    extern float distanceZ;
    extern bool xySheetsON;
    extern bool yzSheetsON;
    extern bool xzSheetsON;
    extern int xySheetsNum;
    extern int yzSheetsNum;
    extern int xzSheetsNum;
    extern int drawModel;
    extern int gridNumX;
    extern int gridNumY;
    extern int gridNumZ;

    extern float dt;

    extern float airDensity;
    extern float ambientTemp;
    extern float boussinesqAlpha;
    extern float boussinesqBeta;
    extern float vorticityConst;

}

// 命名空间 Lagrangian2dPara 包含了一些用于二维拉格朗日流体模拟的参数和流体块信息。

namespace Lagrangian2dPara
{
    // 结构体 FluidBlock 用于表示流体块的信息。
    struct FluidBlock {
        glm::vec2 lowerCorner = glm::vec2(0.0f, 0.0f); // 流体块的下角落位置
        glm::vec2 upperCorner = glm::vec2(0.0f, 0.0f); // 流体块的上角落位置
        glm::vec2 initVel = glm::vec2(0.0f, 0.0f); // 流体块的初始速度
        float particleSpace = 0.02f; // 粒子间距
    };

    // 外部声明的一些全局变量
    extern float scale; // 缩放因子
    extern std::vector<FluidBlock> fluidBlocks; // 流体块数组

    extern float dt; // 时间步长
    extern int substep; // 子步数
    extern float maxVelocity; // 最大速度
    extern float velocityAttenuation; // 速度衰减
    extern float eps; // 极小值

    extern float supportRadius; // 会受影响的范围
    extern float particleRadius; // 粒子半径
    extern float particleDiameter; // 粒子直径
    extern float gravityX; // X 方向的重力加速度
    extern float gravityY; // Y 方向的重力加速度
    extern float density; // 流体密度
    extern float stiffness; // 弹性系数
    extern float exponent; // 幂指数
    extern float viscosity; // 粘度
}


namespace Lagrangian3dPara
{

    struct FluidBlock {
        glm::vec3 lowerCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upperCorner = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 initVel = glm::vec3(0.0f, 0.0f, 0.0f);
        float particleSpace = 0.02f;
    };

    extern float scale;
    extern std::vector<FluidBlock> fluidBlocks;


    extern float dt;
    extern int substep;
    extern float maxVelocity;
    extern float velocityAttenuation;
    extern float eps;

    extern float supportRadius;
    extern float particleRadius;
    extern float particleDiameter;

    extern float gravityX;
    extern float gravityY;
    extern float gravityZ;

    extern float density;
    extern float stiffness;
    extern float exponent;
    extern float viscosity;
}

extern std::string shaderPath;
extern std::string picturePath;

extern std::vector<Glb::Component *> methodComponents;

#endif // !__CONFIGURE_H__
