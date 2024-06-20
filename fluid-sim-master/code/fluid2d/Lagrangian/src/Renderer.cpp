#include "Lagrangian/include/Renderer.h"

#include <iostream>
#include <fstream>
#include "Configure.h"

// This is the main definitions of renderer, which involve the OpenGL functions.
// Before browsing this file, make sure you have elementary knowledge of OpenGL.

namespace FluidSimulation
{

    namespace Lagrangian2d
    {

        Renderer::Renderer()
        {
        }

        // 初始化渲染器。
        // 返回：
        // 初始化结果，0 表示成功，非 0 表示失败。
        int32_t Renderer::init()
        {
            // 获取着色器文件路径。
            extern std::string shaderPath;

            // 设置粒子顶点着色器和片段着色器的路径。
            std::string particleVertShaderPath = shaderPath + "/DrawParticles2d.vert";
            std::string particleFragShaderPath = shaderPath + "/DrawParticles2d.frag";

            // 创建着色器对象并从文件构建着色器程序。
            shader = new Glb::Shader();
            shader->buildFromFile(particleVertShaderPath, particleFragShaderPath);

            // 生成顶点数组对象。
            glGenVertexArrays(1, &VAO);
            // 生成顶点缓冲对象（用于位置）。
            glGenBuffers(1, &positionVBO);
            // 生成顶点缓冲对象（用于密度）。
            glGenBuffers(1, &densityVBO);

            // 生成帧缓冲对象。
            glGenFramebuffers(1, &FBO);
            // 激活帧缓冲对象。
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 生成纹理。
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            // 设置纹理参数。
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

            // 将纹理附加到帧缓冲对象的颜色附件上。
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            // 生成渲染缓冲对象。
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            // 设置渲染缓冲对象的存储格式。
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // 将渲染缓冲对象附加到帧缓冲对象的深度模板附件上。
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

            // 检查帧缓冲对象是否完整。
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
            }

            // 解绑帧缓冲对象。
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 设置视口大小。
            glViewport(0, 0, imageWidth, imageHeight);

            // 返回初始化结果。
            return 0;
        }


        // 绘制粒子系统中的粒子。
        // 参数：
        // - ps: 要绘制的粒子系统对象的引用。
        void Renderer::draw(ParticleSystem2d& ps)
        {
            // 绑定顶点数组对象（VAO）。
            glBindVertexArray(VAO);

            // 绑定顶点缓冲对象（VBO）到 GL_ARRAY_BUFFER。
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            // 将数据复制到当前绑定的缓冲区（VBO）中。
            glBufferData(GL_ARRAY_BUFFER, ps.particles.size() * sizeof(ParticleInfo2d), ps.particles.data(), GL_STATIC_DRAW);

            // 设置顶点属性指针，用于指定如何解释缓冲区中的数据。
            // 位置属性
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, position));
            // 启用位置属性
            glEnableVertexAttribArray(0);

            // 密度属性
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, density));
            // 启用密度属性
            glEnableVertexAttribArray(1);

            // 解绑顶点数组对象。
            glBindVertexArray(0);

            // 记录粒子数量。
            particleNum = ps.particles.size();

            // 绑定帧缓冲对象。
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // 设置清除颜色为白色，启用深度测试，清除颜色缓冲和深度缓冲。
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 绑定顶点数组对象（VAO）。
            glBindVertexArray(VAO);
            // 使用粒子着色器程序。
            shader->use();
            // 设置着色器中的缩放因子。
            shader->setFloat("scale", ps.scale);

            // 启用程序点大小。
            glEnable(GL_PROGRAM_POINT_SIZE);

            // 绘制粒子。
            glDrawArrays(GL_POINTS, 0, particleNum);

            // 解绑帧缓冲对象。
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }


        GLuint Renderer::getRenderedTexture()
        {
            return textureID;
        }
    }

}