#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "skydome.h"
#include "stb_image.h"

void Skydome::Init()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;;

    const unsigned int rows = 64;
    const unsigned int columns = 64;

    for(int x = 0; x <= rows; x++)
    {
        for(int y = 0; y <= columns; y++)
        {
            float x_segment = (float) x / (float) rows;
            float y_segment = (float) y / (float) columns;  

            float phi = x_segment * 2.0f * M_PI;
            float theta = y_segment * M_PI;


            float xPos = cos(phi) * sin(theta);
            float yPos = cos(theta);
            float zPos = sin(phi) * sin(theta);

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            vertices.push_back(x_segment);
            vertices.push_back(y_segment);
        }
    }

    bool oddRow = false;

    for(int y = 0; y < columns; y++)
    {
        if(!oddRow)
        {
            for(int x = 0; x <= rows; x++) 
            {
                indices.push_back(y * (rows + 1) + x);
                indices.push_back((y + 1) * (rows + 1) + x);
            }
        }
        else 
        {
            for(int x = rows; x >= 0; x--) 
            {
                indices.push_back((y + 1) * (rows + 1) + x);
                indices.push_back(y * (rows + 1) + x);
            }
        }

        oddRow = !oddRow;
    }

    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load("assets/skydome.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);


    glGenTextures(1, &texture2ID);
    glBindTexture(GL_TEXTURE_2D, texture2ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *data2 = stbi_load("assets/skydome2.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data2);

    glBindVertexArray(0);
}

void Skydome::Draw(Shader& shader, const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const glm::vec3& sunDirection, float time)
{
    shader.Use();
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cameraPosition);
    model = glm::scale(model, glm::vec3(100.0f));


    shader.SetMat4("viewProjection", viewProjection);
    shader.SetMat4("model", model);
    shader.SetVec3("sunDirection", sunDirection);
    shader.SetInt("nightSkyTexture", 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    shader.SetInt("cloudTexture", 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2ID);

    shader.SetFloat("time", time);


    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

}