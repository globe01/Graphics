#include "WhittedRayTracingRenderer.hpp"

#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"

#include "shaders/Dielectric.hpp"// 在shaderCreator里面也有，不写也行
#include <random>// 俄罗斯轮盘赌要用
#include <cstdlib>
#include <ctime>

// Whitted-style递归光线追踪算法，打到一个点后会反射、折射继续传播，每个照亮的弹射点都会加到着色的值


// 定义一下递归的最大深度，暂定个50
#define RecursionDepth 50


// 辅助函数，判断材质的类型后续来做反射折射的不同处理
int typeofMaterial(auto& material) {
    if (material.hasProperty("specularColor")) {
        return 1; // 1是包含镜面反射的Phong材质
    } else if (material.hasProperty("refractiveIndex")) {
        return 2; // 2是包含折射的透明材质
    } else {
        return 0; // 0就是原本的Lambertian材质
    }
}


// 实现Whitted-style递归光线追踪算法的主要文件
namespace WhittedRayTracing
{
    // realease函数用于释放渲染结果
    void WhittedRayTracingRenderer::release(const RenderResult& r) {
        auto [p, w, h] = r;
        delete[] p;// 释放像素数组的内存
    }

    // gamma函数对颜色进行伽马校正
    RGB WhittedRayTracingRenderer::gamma(const RGB& rgb) {
        return glm::sqrt(rgb);// 伽马校正，通常用于调整图像的亮度
    }

    

    // render执行渲染过程并返回渲染结果
    auto WhittedRayTracingRenderer::render() -> RenderResult {
        auto width = scene.renderOption.width;// 获取渲染宽度
        auto height = scene.renderOption.height;// 获取渲染高度
        auto pixels = new RGBA[width*height];// 创建像素数组

        VertexTransformer vertexTransformer{};// 创建顶点变换器
        vertexTransformer.exec(spScene);// 执行顶点变换，到这个场景

        ShaderCreator shaderCreator{};// 创建着色器创建器
        // 遍历场景中的所有材质，来创建着色器程序
        for (auto& mtl : scene.materials) {
            shaderPrograms.push_back(shaderCreator.create(mtl, scene.textures));
        }

        // 遍历渲染的每个像素，每个高，每个宽
        for (int i=0; i<height; i++) {
            for (int j=0; j < width; j++) {
                // 从相机发射光线
                auto ray = camera.shoot(float(j)/float(width), float(i)/float(height));
                // 递归光线追踪
                auto color = trace(ray, 0, 1.0f);// 加上递归深度和衰减参数
                // 对颜色进行clamp和gamma校正
                color = clamp(color);// clamp函数用于对颜色进行截断，限制颜色范围在0-1之间
                color = gamma(color);
                pixels[(height-i-1)*width+j] = {color, 1};// 将颜色赋值给像素数组
            }
        }

        return {pixels, width, height};// 返回像素数组，宽度，高度
    }
    

    // trace函数--追踪光线并计算颜色--递归光线追踪，加个递归深度参数来限制递归次数，再加个衰减参数，反射折射会衰减
    RGB WhittedRayTracingRenderer::trace(const Ray& r, int recursionDepth, float attenuation) {
        // 先限制一下递归次数
        if(recursionDepth > RecursionDepth){
            return {0, 0, 0};// 递归深度超过最大深度就直接返回rgb000--黑色
        }

        // 优化：加个俄罗斯轮盘赌，随机选择反射或折射
        // 大致想法就是随机数低于我设定的值时就停止递归
        float stopP = 0.0001f;// ->直接终止递归的概率
        // 随机数如果用rand总是生成0.96或者0.97，查资料说是除以MAX的时候都趋于0.96了，不用rand了
        // 找了个别的随机数库
        static uniform_real_distribution<float> distribution(0.0f, 1.0f);
        static default_random_engine generator;
        float randomFloat = distribution(generator);
        if (randomFloat < stopP) {
            return {0, 0, 0};// 根据概率终止递归
        }


        // 再限制一下衰减，如果衰减系数太小也直接返回黑色
        if(attenuation < 0.00001f){
            return {0, 0, 0};
        }



        // 如果没有光源，返回rgb000--黑色
        if (scene.pointLightBuffer.size() < 1) return {0, 0, 0};


        auto& l = scene.pointLightBuffer[0];// 获取第一个点光源
        auto closestHitObj = closestHit(r);// 找到光线击中的最近的物体
        // 如果击中了物体
        if (closestHitObj) {
            // 获取击中的物体的信息
            auto& hitRec = *closestHitObj;// 找到击中物体
            auto& hitMaterial = hitRec.material;// 获取击中物体的材质
            auto& hitNormal = hitRec.normal;// 获取击中物体的法向量
            auto& hitPoint = hitRec.hitPoint;// 获取击中物体的交点

            auto out = glm::normalize(l.position - hitPoint);// 计算出射光线的方向
            // 如果出射光线与法向量的点积小于0，说明光线与法向量的夹角大于90度，光源在物体的背面，所以返回rgb000--黑色
            if (glm::dot(out, hitNormal) < 0) {
                return {0, 0, 0};
            }

            auto distance = glm::length(l.position - hitPoint);// 计算光源到交点的距离
            auto shadowRay = Ray{hitPoint, out};// 创建阴影光线
            auto shadowHit = closestHit(shadowRay);// 找到阴影光线击中的最近的物体

            

            // 计算着色，c是光照强度
            auto c = shaderPrograms[hitMaterial.index()]->shade(-r.direction, out, hitNormal);


            // 根据不同材质分成不同的情况
            auto& MaterialForSwitch = scene.materials[hitMaterial.index()];
            int TypeForSwitch = typeofMaterial(MaterialForSwitch);

            if(randomFloat > stopP){// 大于才递归，小于stopP则直接终止递归
                switch(TypeForSwitch){
                    case 0:{// Lambertian材质，只有漫反射，保持原来的就行，不用递归
                        // 如果没有阴影，或者有阴影但是阴影的距离大于光源到交点的距离，返回光照强度
                        if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                            return c * l.intensity;// 没有遮挡，返回光照强度
                        }
                        else {
                            return Vec3{0};// 有遮挡，返回rgb000--黑色
                        }
                        break;
                    }
                    case 1:{// Phong材质
                        RGB plusRefLect = Vec3{0};// 先默认为黑色
                        // 新增反射光线，反射方向=入射方向−2*(入射方向*法向量)*法向量
                        // 计算反射光线方向
                        // 必须是入射光线相对于表面法线的反射方向！不是光源方向！
                        Vec3 reflectDir = reflect(glm::normalize(r.direction), hitNormal);// 用reflect函数计算反射光线
                        Ray reflectRay{hitPoint, reflectDir}; // 反射光线
                        //Ray reflectRay{hitPoint, reflect(hitNormal, out)};// 反射光线 好像不太对
                        

                        // 新增反射衰减
                        float reflectDecay = attenuation * 0.8f;
                        // 递归计算反射光线的颜色
                        auto reflectColor = trace(reflectRay, recursionDepth + 1, reflectDecay);
                        // 如果没有阴影，或者有阴影但是阴影的距离大于光源到交点的距离，返回光照强度
                        if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                            plusRefLect = c * l.intensity 
                                        + reflectColor;
                            return plusRefLect;// 没有遮挡，返回光照强度
                        }
                        else {
                            RGB plusRefLect = Vec3{0} 
                                            + reflectColor;
                            return plusRefLect;// 有遮挡，返回rgb000--黑色
                        }
                        break;
                    }
                    case 2:{// Dielectric材质如玻璃，这个有折射

                        RGB plusRefLectRefrect = Vec3{0};// 先默认为黑色

                        float refract_index = 1.5f;// 先用1.5代替吧
                        
                        // 反射 和上面一样
                        Vec3 reflectDir = reflect(glm::normalize(r.direction), hitNormal);// 用reflect函数计算反射光线
                        Ray reflectRay{hitPoint, reflectDir}; // 反射光线
                        float reflectDecay = attenuation * 0.8f; // 新增反射衰减
                        // 递归计算反射光线
                        auto reflectColor = trace(reflectRay, recursionDepth + 1, reflectDecay);


                        // 折射
                        Vec3 refractDir = refract(hitNormal, glm::normalize(r.direction), refract_index);// 先用1.5代替吧
                        Ray refractRay{hitPoint, refractDir}; // 折射光线
                        float refractDecay = attenuation * 0.7f; // 新增折射衰减
                        // 递归计算折射光线
                        auto refractColor = trace(refractRay, recursionDepth + 1, refractDecay);


                        // 如果没有阴影，或者有阴影但是阴影的距离大于光源到交点的距离，返回光照强度
                        if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                            plusRefLectRefrect = c * l.intensity 
                                                + reflectColor 
                                                + refractColor;// 再加折射
                            return plusRefLectRefrect;// 再加折射
                        }
                        else {
                            plusRefLectRefrect = Vec3{0} 
                                                + reflectColor 
                                                + refractColor;// 再加折射
                            return plusRefLectRefrect;// 再加折射
                        }
                        break;

                    }
                    
                    default:{
                        // 如果没有阴影，或者有阴影但是阴影的距离大于光源到交点的距离，返回光照强度
                        if ((!shadowHit) || (shadowHit && shadowHit->t > distance)) {
                            return c * l.intensity;// 没有遮挡，返回光照强度
                        }
                        else {
                            return Vec3{0};// 有遮挡，返回rgb000--黑色
                        }
                    }


                }
            }
            else if(randomFloat < stopP){// 小于stop就直接终止递归
                return Vec3{0};// 根据概率终止递归
            }
            


        }
        
        
        // 如果没有击中物体，也是返回rgb000--黑色
        else {
            return {0, 0, 0};
        }
    }


    // closestHit函数--找到光线击中的最近的物体
    HitRecord WhittedRayTracingRenderer::closestHit(const Ray& r) {
        HitRecord closestHit = nullopt;// 初始化最近的物体为空
        float closest = FLOAT_INF;// 初始化最近的距离为无穷大
        // 遍历场景中的所有物体，找到光线击中的最近的物体
        // 遍历所有球体
        for (auto& s : scene.sphereBuffer) {
            auto hitRecord = Intersection::xSphere(r, s, 0.01, closest);// 计算光线与球体的交点
            // 如果有交点，并且交点的距离小于最近的距离，更新成最近的距离和最近的物体
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        // 遍历所有三角形
        for (auto& t : scene.triangleBuffer) {
            auto hitRecord = Intersection::xTriangle(r, t, 0.01, closest);// 计算光线与三角形的交点
            // 如果有交点，并且交点的距离小于最近的距离，更新成最近的距离和最近的物体
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        // 遍历所有平面
        for (auto& p : scene.planeBuffer) {
            auto hitRecord = Intersection::xPlane(r, p, 0.01, closest);// 计算光线与平面的交点
            // 如果有交点，并且交点的距离小于最近的距离，更新成最近的距离和最近的物体
            if (hitRecord && hitRecord->t < closest) {
                closest = hitRecord->t;
                closestHit = hitRecord;
            }
        }
        return closestHit;// 返回最近的物体 
    }
}