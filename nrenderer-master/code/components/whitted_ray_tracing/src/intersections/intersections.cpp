#include "intersections/intersections.hpp"


// intersection就是判断射线与物体是否相交，如果相交则返回交点的信息
namespace WhittedRayTracing::Intersection
{
    // 计算射线与三角形的交点
    HitRecord xTriangle(const Ray& ray, const Triangle& t, float tMin, float tMax) {
        // 三角形的三个顶点
        const auto& v1 = t.v1;
        const auto& v2 = t.v2;
        const auto& v3 = t.v3;
        // 三角形的法向量
        const auto& normal = glm::normalize(t.normal);
        // 计算射线与三角形的交点
        auto e1 = v2 - v1;// 三角形的第1条边
        auto e2 = v3 - v1;// 三角形的第2条边
        auto P = glm::cross(ray.direction, e2);// 射线方向与三角形的第2条边的叉乘
        float det = glm::dot(e1, P);// 三角形的第1条边与射线方向与三角形的第2条边的叉乘的点积
        Vec3 T;// 射线与三角形的交点
        if (det > 0) T = ray.origin - v1;// det>0说明射线与三角形在同一方向
        else { T = v1 - ray.origin; det = -det; }// det<0说明射线与三角形在相反方向
        // 如果det非常小，说明射线与三角形平行，不相交
        if (det < 0.000001f) return getMissRecord();

        // 然后就是计算交点的uv坐标
        float u, v, w;
        u = glm::dot(T, P);// 计算交点的u坐标
        if (u > det || u < 0.f) return getMissRecord();// 如果u不在[0, det]范围内，说明交点不在三角形内
        Vec3 Q = glm::cross(T, e1);// 计算交点的Q坐标，Q是交点的法向量
        v = glm::dot(ray.direction, Q);// 计算交点的v坐标
        if (v < 0.f || v + u > det) return getMissRecord();// 如果v不在[0, det]范围内，说明交点不在三角形内
        w = glm::dot(e2, Q);// 计算交点的w坐标
        float invDet = 1.f / det;// 计算det的倒数
        w *= invDet;// 计算交点的w坐标，w是交点的深度
        if (w >= tMax || w <= tMin) return getMissRecord();// 如果w不在[tMin, tMax]范围内，说明交点不在三角形内
        
        // 算出来交点的uv坐标后，就可以计算交点的位置了
        return getHitRecord(w, ray.at(w), normal, t.material);

    }

    // 计算射线与球体的交点
    HitRecord xSphere(const Ray& ray, const Sphere& s, float tMin, float tMax) {
        const auto& position = s.position;// 球体的位置
        const auto& r = s.radius;// 球体的半径
        Vec3 oc = ray.origin - position;// 射线的起点到球体的位置的向量
        // a是射线方向的点积，b是射线方向与oc的点积，c是oc的点积
        float a = glm::dot(ray.direction, ray.direction);
        float b = glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - r*r;
        // 计算判别式，用判别式来看是否有交点
        float discriminant = b*b - a*c;// 判别式
        float sqrtDiscriminant = sqrt(discriminant);// 判别式的平方根
        // 根据数学知识，如果判别式小于0，说明没有交点
        if (discriminant > 0) {
            float temp = (-b - sqrtDiscriminant) / a;// 计算交点的t值，二元一次方程的一个解
            // 如果交点的t值在框定的范围内，说明有交点
            if (temp < tMax && temp > tMin) {
                auto hitPoint = ray.at(temp);// 计算交点的位置，射线直接o+temp*d就找到点了
                auto normal = (hitPoint - position)/r;// 计算交点的法向量，交点-球心就是法向量
                return getHitRecord(temp, hitPoint, normal, s.material);// 返回交点的信息
            }
            // 如果交点的t值不在框定的范围内，说明没有交点
            temp = (-b + sqrtDiscriminant) / a;// 二元一次方程的另一个解
            // 也是同样的方式来判断这个解在不在框定的范围内
            if (temp < tMax && temp > tMin) {
                auto hitPoint = ray.at(temp);
                auto normal = (hitPoint - position)/r;
                return getHitRecord(temp, hitPoint, normal, s.material);
            }
        }
        // 如果判别式小于0，说明没有交点，=0说明射线与球体相切，也认为是没有交点
        return getMissRecord();
    }

    // 计算射线与平面的交点
    HitRecord xPlane(const Ray& ray, const Plane& p, float tMin, float tMax) {
        Vec3 normal = glm::normalize(p.normal);// 获取平面的法向量
        auto Np_dot_d = glm::dot(ray.direction, normal);// 计算射线方向与平面法向量的点积
        // 如果点积为0，说明射线与平面平行，没有交点
        if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f) return getMissRecord();
        
        // 算出来平面上一点与法向量的点积，然后就可以计算交点了
        float dp = -glm::dot(p.position, normal);// 平面上一点与法向量的点积
        float t = (-dp - glm::dot(normal, ray.origin))/Np_dot_d;// 计算交点的t值
        if (t >= tMax || t <= tMin) return getMissRecord();// 如果t值不在框定的范围内，说明没有交点
        // cross test就是判断交点是否在平面内
        Vec3 hitPoint = ray.at(t);// 先找到交点
        Mat3x3 d{p.u, p.v, glm::cross(p.u, p.v)};// 交点的uv坐标
        d = glm::inverse(d);// 交点的uv坐标的逆矩阵
        auto res  = d * (hitPoint - p.position);// 计算交点-平面上一点的向量，然后乘以逆矩阵，得到交点的uv坐标
        auto u = res.x, v = res.y;// 获取交点的uv坐标
        // 如果uv坐标在[0, 1]范围内，说明交点在平面内
        if ((u<=1 && u>=0) && (v<=1 && v>=0)) {
            return getHitRecord(t, hitPoint, normal, p.material);
        }
        return getMissRecord();// 否则与平面没有交点
    }

    // 计算射线与区域光源的交点
    HitRecord xAreaLight(const Ray& ray, const AreaLight& a, float tMin, float tMax) {
        Vec3 normal = glm::cross(a.u, a.v);// 获取区域光源的法向量
        Vec3 position = a.position;// 区域光源的位置
        auto Np_dot_d = glm::dot(ray.direction, normal);// 计算射线方向与法向量的点积
        // 如果点积为0，说明射线与法向量平行，没有交点
        if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f) return getMissRecord();
        float dp = -glm::dot(position, normal);// 计算区域光源上一点与法向量的点积
        float t = (-dp - glm::dot(normal, ray.origin))/Np_dot_d;// 计算交点的t值
        // 如果t值不在框定的范围内，说明没有交点
        if (t >= tMax || t <= tMin) return getMissRecord();
        // cross test 就是判断交点是否在区域光源内
        Vec3 hitPoint = ray.at(t);// 射线有了t值，就可以计算交点了
        Mat3x3 d{a.u, a.v, glm::cross(a.u, a.v)};// 交点的uv坐标
        d = glm::inverse(d);// 交点的uv坐标的逆矩阵
        auto res  = d * (hitPoint - position);// 计算交点-区域光源上一点的向量，然后乘以逆矩阵，得到交点的uv坐标
        auto u = res.x, v = res.y;// 获取交点的uv坐标
        // 如果uv坐标在[0, 1]范围内，说明交点在区域光源内
        if ((u<=1 && u>=0) && (v<=1 && v>=0)) {
            return getHitRecord(t, hitPoint, normal, {});
        }
        return getMissRecord();// 否则与区域光源没有交点
    }
}