#include "server/Server.hpp"
#include "component/RenderComponent.hpp"

#include "RayCastRenderer.hpp"

using namespace std;
using namespace NRenderer;

namespace RayCast
{
    class Adapter : public RenderComponent
    {
    public:
        void render(SharedScene spScene) {
            // 创建WhittedRayTracingRenderer对象
            RayCastRenderer rayCast{spScene};
            // 调用render接口
            auto result = rayCast.render();
            // 解包
            auto [ pixels, width, height ] = result;
            // 设置屏幕
            getServer().screen.set(pixels, width, height);
            // 释放资源
            rayCast.release(result);
        }
    };
}

const static string description = 
    "Ray Cast Renderer.\n"
    "Supported:\n"
    " - Lambertian and Phong\n"
    " - One Point Light\n"
    " - Triangle, Sphere, Plane\n"
    " - Simple Pinhole Camera\n\n"
    "Please use ray_cast.scn"
    ;

REGISTER_RENDERER(RayCast, description, RayCast::Adapter);
