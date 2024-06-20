#pragma once
#ifndef GLOBAL_H
#define GLOBAL_H
#include <chrono>
#include <random>
#include <unordered_map>
#include <glm/glm.hpp>
#include <string>

namespace Glb {

    class Timer {
    public:
        static Timer& getInstance() {
            static Timer instance;
            return instance;
        }

    private:
        Timer() {

        };

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        std::chrono::system_clock::time_point fpsLastTime;
        std::chrono::system_clock::time_point fpsNow;

        std::chrono::system_clock::time_point lastTime;
        std::chrono::system_clock::time_point now;

        std::unordered_map<std::string, unsigned long long int> record;

    public:

        bool empty() {
            return record.empty();
        }

        void clear() {
            record.clear();
        }

        void start() {
            lastTime = std::chrono::system_clock::now();
        } 

        // 记录当前时间作为上一个帧的时间点。
        void timeFPS() {
            fpsLastTime = fpsNow;
            fpsNow = std::chrono::system_clock::now();
        }

        // 获取帧率。
        std::string getFPS() {
            auto dur = fpsNow - fpsLastTime;
            float dt = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            if (dt >= 1000) {
                return "0.00"; // 如果时间间隔超过1秒，则返回0帧/秒。
            }
            else if (dt <= 1) {
                return "1000.00"; // 如果时间间隔小于等于1毫秒，则返回1000帧/秒。
            }
            return std::to_string(1000 / dt).substr(0, 5); // 否则，返回计算得到的帧率。
        }

        // 记录特定事件的时间。
        void recordTime(std::string str) {
            now = std::chrono::system_clock::now();
            auto dur = now - lastTime;
            lastTime = now;
            auto it = record.find(str);
            if (it != record.end()) {
                it->second = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            }
            else {
                record[str] = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
            }
        }

        // 获取当前状态，包括各个事件所占用的时间百分比。
        std::string currentStatus() {
            std::string str;
            int total_time = 0;
            for (const auto& timing : record) {
                total_time += timing.second; // 计算所有事件的总时间。
            }

            for (const auto& timing : record) {
                float percentage = static_cast<float>(timing.second) / total_time * 100; // 计算每个事件的时间百分比。
                str += timing.first + ": " + std::to_string(percentage).substr(0, 5) + "%% \n"; // 将事件名称和时间百分比拼接成字符串。
            }

            return str; // 返回拼接好的字符串。
        }

    };


    // 随机数生成器类
    class RandomGenerator {
    private:
        std::random_device dev; // 随机设备对象，用于获取随机种子
    public:
        // 获取指定范围内的均匀分布的随机浮点数
        // 参数：
        // - min: 随机数的最小值，默认为0.0f
        // - max: 随机数的最大值，默认为1.0f
        // 返回：
        // 指定范围内的随机浮点数
        float GetUniformRandom(float min = 0.0f, float max = 1.0f) {
            std::mt19937 rng(dev()); // 以随机设备对象的随机种子为种子创建 Mersenne Twister 伪随机数生成器
            std::uniform_real_distribution<float> dist(min, max); // 创建均匀实数分布对象，范围为[min, max]
            return dist(rng); // 返回在指定范围内的随机浮点数
        }
    };


}


#endif // !GLOBAL_H




