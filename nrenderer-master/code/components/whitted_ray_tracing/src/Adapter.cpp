#include "component/RenderComponent.hpp"
#include "server/Server.hpp"

// WhittedRayTracingRenderer递归光线追踪的头文件
#include "WhittedRayTracingRenderer.hpp"

using namespace std;
using namespace NRenderer;


namespace WhittedRayTracing
{

    // 继承RenderComponent, 复写render接口
    class Adapter : public RenderComponent {
        virtual void render(SharedScene spScene) {
            // 创建WhittedRayTracingRenderer对象
            WhittedRayTracingRenderer rayCast{spScene};
            // 调用render接口来渲染
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
    "Whitted Ray Tracing Renderer.\n"
    "Supported:\n"
    " - Lambertian, Phong and Dielectric\n"
    " - One Point Light\n"
    " - Triangle, Sphere, Plane\n"
    " - Simple Pinhole Camera\n\n"
    "Please use whitted_ray_tracing.scn"
    ;


// REGISTER_RENDERER(Name, Description, Class)
REGISTER_RENDERER(WhittedRayTracing, description, WhittedRayTracing::Adapter);
