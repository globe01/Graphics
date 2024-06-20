#include "server/Server.hpp"
#include "scene/Scene.hpp"
#include "component/RenderComponent.hpp"
#include "Camera.hpp"

#include "SimplePathTracer.hpp"

using namespace std;
using namespace NRenderer;

namespace SimplePathTracer
{
    class Adapter : public RenderComponent
    {
        void render(SharedScene spScene) {
            SimplePathTracerRenderer renderer{spScene};// 创建渲染器
            auto renderResult = renderer.render();// 渲染
            auto [ pixels, width, height ]  = renderResult;// 获取渲染结果
            getServer().screen.set(pixels, width, height);// 设置渲染结果
            renderer.release(renderResult);// 释放渲染结果资源
        }
    };
}

const static string description = 
    "A Simple Path Tracer. "
    "Only some simple primitives and materials(Lambertian) are supported."
    "\nPlease use scene file : cornel_area_light.scn";

REGISTER_RENDERER(SimplePathTracer, description, SimplePathTracer::Adapter);