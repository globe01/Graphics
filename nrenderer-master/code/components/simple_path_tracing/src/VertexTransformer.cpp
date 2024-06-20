#include "VertexTransformer.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace SimplePathTracer
{
    // 由局部坐标转换为世界坐标
    void VertexTransformer::exec(SharedScene spScene) {
        auto& scene = *spScene;
        // 遍历场景中的每个节点
        for (auto& node : scene.nodes) {
            // 生成变换矩阵，单位矩阵
            Mat4x4 t{1};
            // 获取节点所属的模型
            auto& model = spScene->models[node.model];
            // 应用模型的平移变换
            t = glm::translate(t, model.translation);
            // 根据节点类型，应用变换到对应的顶点
            if (node.type == Node::Type::TRIANGLE) {
                for (int i=0; i<3; i++) {
                    auto& v = scene.triangleBuffer[node.entity].v[i];
                    v = t*Vec4{v, 1};// 变换三角形顶点
                }
            }
            else if (node.type == Node::Type::SPHERE) {
                auto& v = scene.sphereBuffer[node.entity].position;
                v = t*Vec4{v, 1};// 变换球体顶点
            }
            else if (node.type == Node::Type::PLANE) {
                auto& v = scene.planeBuffer[node.entity].position;
                v = t*Vec4{v, 1};// 变换平面顶点
            }
        }
    }
}