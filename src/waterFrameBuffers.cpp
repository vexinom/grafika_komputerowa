#include "waterFrameBuffers.h"
#include "config.h"

WaterFrameBuffers::WaterFrameBuffers()
{
    initialiseReflectionFrameBuffer();
	initialiseRefractionFrameBuffer();
}

GLuint WaterFrameBuffers::createFrameBuffer()
{
    GLuint frameBuffer;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    return frameBuffer;
}

GLuint WaterFrameBuffers::createTextureAttachment( int width, int height)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
    return texture;

}

GLuint WaterFrameBuffers::createDepthTextureAttachment( int width, int height)
{   
    GLuint texture;
    glGenTextures(1, &texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
    return texture;

}

GLuint WaterFrameBuffers::createDepthBufferAttachment(int width, int height)
{
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    return depthBuffer;
}

void WaterFrameBuffers::unbindCurrentFrameBuffer()
{
    glBindBuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, config::WINDOW_WIDTH, config::WINDOW_HEIGHT);

}
void WaterFrameBuffers::initialiseReflectionFrameBuffer()
{
    reflectionFrameBuffer = createFrameBuffer();
    reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    reflectionDepthBuffer = createDepthTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    unbindCurrentFrameBuffer();

}

void WaterFrameBuffers::initialiseRefractionFrameBuffer()
{
    refractionFrameBuffer = createFrameBuffer();
    refractionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    refractionDepthTexture = createDepthTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
    unbindCurrentFrameBuffer();
}

GLuint WaterFrameBuffers::getReflectionTexture()
{
    return reflectionTexture;
}

GLuint WaterFrameBuffers::getRefractionTexture()
{
    return refractionTexture;
}

GLuint WaterFrameBuffers::getRefractionDepthTexture()
{
    return refractionDepthTexture;
}

void WaterFrameBuffers::bindReflectionFrameBuffer()
{
    bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
}

void WaterFrameBuffers::bindRefractionFrameBuffer()
{
    bindFrameBuffer(refractionFrameBuffer,REFRACTION_WIDTH,REFRACTION_HEIGHT);
}

 void WaterFrameBuffers::bindFrameBuffer(GLuint frameBuffer, int width, int height)
 {
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glViewport(0, 0, width, height);
 }

 void WaterFrameBuffers::cleanUp()
 {
    glDeleteFramebuffers(1, &reflectionFrameBuffer);
    glDeleteFramebuffers(1, &refractionFrameBuffer);

    glDeleteTextures(1, &reflectionTexture);
    glDeleteTextures(1, &refractionTexture);
    glDeleteTextures(1, &refractionDepthTexture);

    glDeleteRenderbuffers(1, &reflectionDepthBuffer);
 }