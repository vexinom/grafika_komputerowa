# pragma once

#include <glad/glad.h>

class WaterFrameBuffers
{

public:
 

    const int REFLECTION_WIDTH = 320;
    const int REFLECTION_HEIGHT = 180;

    const int REFRACTION_WIDTH = 1280;
    const int REFRACTION_HEIGHT = 720;

    const int OCEANDEPTH_WIDTH = 1280;
    const int OCEANDEPTH_HEIGHT = 720;

    GLuint reflectionFrameBuffer;
    GLuint reflectionTexture;
    GLuint reflectionDepthBuffer;

    GLuint refractionFrameBuffer;
    GLuint refractionTexture;
    GLuint refractionDepthTexture;

    GLuint oceandepthFrameBuffer;
    GLuint oceandepthColorBuffer;
    GLuint oceandepthDepthTexture;

    GLuint quadVAO, quadVBO;



    void init();

    GLuint createFrameBuffer();
    GLuint createTextureAttachment( int width, int height);
    GLuint createDepthTextureAttachment( int width, int height);
    GLuint createDepthBufferAttachment(int width, int height);

    GLuint getReflectionTexture();
    GLuint getRefractionTexture();
    GLuint getRefractionDepthTexture();

    void bindReflectionFrameBuffer();
    void bindRefractionFrameBuffer();
    void bindOceandepthFrameBuffer();
    void bindFrameBuffer(GLuint frameBuffer, int width, int height);

    void unbindCurrentFrameBuffer();

    void initialiseReflectionFrameBuffer();
    void initialiseRefractionFrameBuffer();
    void initialiseOceandepthFrameBuffer();

    void initialiseQuad();

    void cleanUp();

};