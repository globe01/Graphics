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

        // ��ʼ����Ⱦ����
        // ���أ�
        // ��ʼ�������0 ��ʾ�ɹ����� 0 ��ʾʧ�ܡ�
        int32_t Renderer::init()
        {
            // ��ȡ��ɫ���ļ�·����
            extern std::string shaderPath;

            // �������Ӷ�����ɫ����Ƭ����ɫ����·����
            std::string particleVertShaderPath = shaderPath + "/DrawParticles2d.vert";
            std::string particleFragShaderPath = shaderPath + "/DrawParticles2d.frag";

            // ������ɫ�����󲢴��ļ�������ɫ������
            shader = new Glb::Shader();
            shader->buildFromFile(particleVertShaderPath, particleFragShaderPath);

            // ���ɶ����������
            glGenVertexArrays(1, &VAO);
            // ���ɶ��㻺���������λ�ã���
            glGenBuffers(1, &positionVBO);
            // ���ɶ��㻺����������ܶȣ���
            glGenBuffers(1, &densityVBO);

            // ����֡�������
            glGenFramebuffers(1, &FBO);
            // ����֡�������
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // ��������
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            // �������������
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glBindTexture(GL_TEXTURE_2D, 0);

            // �������ӵ�֡����������ɫ�����ϡ�
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            // ������Ⱦ�������
            glGenRenderbuffers(1, &RBO);
            glBindRenderbuffer(GL_RENDERBUFFER, RBO);
            // ������Ⱦ�������Ĵ洢��ʽ��
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            // ����Ⱦ������󸽼ӵ�֡�����������ģ�帽���ϡ�
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

            // ���֡��������Ƿ�������
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                Glb::Logger::getInstance().addLog("Error: Framebuffer is not complete!");
            }

            // ���֡�������
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // �����ӿڴ�С��
            glViewport(0, 0, imageWidth, imageHeight);

            // ���س�ʼ�������
            return 0;
        }


        // ��������ϵͳ�е����ӡ�
        // ������
        // - ps: Ҫ���Ƶ�����ϵͳ��������á�
        void Renderer::draw(ParticleSystem2d& ps)
        {
            // �󶨶����������VAO����
            glBindVertexArray(VAO);

            // �󶨶��㻺�����VBO���� GL_ARRAY_BUFFER��
            glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
            // �����ݸ��Ƶ���ǰ�󶨵Ļ�������VBO���С�
            glBufferData(GL_ARRAY_BUFFER, ps.particles.size() * sizeof(ParticleInfo2d), ps.particles.data(), GL_STATIC_DRAW);

            // ���ö�������ָ�룬����ָ����ν��ͻ������е����ݡ�
            // λ������
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, position));
            // ����λ������
            glEnableVertexAttribArray(0);

            // �ܶ�����
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo2d), (void*)offsetof(ParticleInfo2d, density));
            // �����ܶ�����
            glEnableVertexAttribArray(1);

            // ��󶥵��������
            glBindVertexArray(0);

            // ��¼����������
            particleNum = ps.particles.size();

            // ��֡�������
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);

            // ���������ɫΪ��ɫ��������Ȳ��ԣ������ɫ�������Ȼ��塣
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // �󶨶����������VAO����
            glBindVertexArray(VAO);
            // ʹ��������ɫ������
            shader->use();
            // ������ɫ���е��������ӡ�
            shader->setFloat("scale", ps.scale);

            // ���ó�����С��
            glEnable(GL_PROGRAM_POINT_SIZE);

            // �������ӡ�
            glDrawArrays(GL_POINTS, 0, particleNum);

            // ���֡�������
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }


        GLuint Renderer::getRenderedTexture()
        {
            return textureID;
        }
    }

}