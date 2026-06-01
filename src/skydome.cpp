#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "skydome.h"

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

    glBindVertexArray(0);
}

void Skydome::Draw(glm::mat4 &viewProjection, glm::vec3 &cameraPosition, unsigned int skydomeShaderID, float time)
{
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glUseProgram(skydomeShaderID);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cameraPosition);

    model = glm::scale(model, glm::vec3(100.0f));

    unsigned int vpLoc = glGetUniformLocation(skydomeShaderID, "viewProjection");
    glUniformMatrix4fv(vpLoc, 1, GL_FALSE, &viewProjection[0][0]);

    unsigned int modelLoc = glGetUniformLocation(skydomeShaderID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

}