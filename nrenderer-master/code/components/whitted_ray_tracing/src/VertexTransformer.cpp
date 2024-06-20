#include "VertexTransformer.hpp"
#include "glm/gtc/matrix_transform.hpp"

// 直接沿用框架Raycast的代码结构即可
namespace WhittedRayTracing
{
    // exec函数是对场景中的所有节点进行变换
    void VertexTransformer::exec(SharedScene spScene) {
        auto& scene = *spScene;// 获取传入的场景
        // 遍历场景中的所有节点
        for (auto& node : scene.nodes) {
            Mat4x4 t{1};// 初始化变换矩阵，矩阵大小是4x4
            auto& model = spScene->models[node.model];// 获取节点对应的模型
            t = glm::translate(t, model.translation);// 根据模型的平移向量对矩阵进行平移变换

            // 如果节点类型是三角形
            if (node.type == Node::Type::TRIANGLE) {
                // 遍历三角形的三个顶点
                for (int i=0; i<3; i++) {
                    auto& v = scene.triangleBuffer[node.entity].v[i];// 获取三角形的顶点
                    v = t*Vec4{v, 1};// 对三角形的顶点进行变换
                }
            }
            // 如果节点类型是球体
            else if (node.type == Node::Type::SPHERE) {
                // 获取球体的位置
                auto& v = scene.sphereBuffer[node.entity].position;
                // 对球体的位置进行变换，怎么变换呢？就是矩阵乘以球体的位置，Vec4{v, 1}是将球体的位置转换为4维向量
                v = t*Vec4{v, 1};
            }
            // 如果节点类型是平面
            else if (node.type == Node::Type::PLANE) {
                // 获取平面的位置
                auto& v = scene.planeBuffer[node.entity].position;
                // 对平面的位置进行变换
                v = t*Vec4{v, 1};
            }
        }
    }
}