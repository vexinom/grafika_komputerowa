#include "worldmesh.h"
#include <cmath>
#include <glad/glad.h>

//must be here for #include "stb_image.h" to work, without it linker throw errors
#define STB_IMAGE_IMPLEMENTATION 

#include "stb_image.h"

void WorldMesh::Init()
{
    int nChannels;

    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load("assets/worldmap.png", &width, &height, &nChannels, 0);

    if (!data) {
        fprintf(stderr, "Failed to load terrain texture heightmap map!\n");
        return;
    }

    NUM_STRIPS = height - 1;
    NUM_VERTS_PER_STRIP = width * 2;


    float yScale = 64.0f / 256.0f, yShift = 16.0f;
    unsigned bytePerPixel = nChannels;

    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            unsigned char* pixelOffset = data + (j + width * i) * bytePerPixel;
            unsigned char y = pixelOffset[0];

            vertices.push_back((float)y * yScale - yShift);
        }
    }
    stbi_image_free(data);

    unsigned int restartIndex = 0xFFFFFFFF;

    for(unsigned int i = 0; i < (unsigned int)(height - 1); i++)       
    {
        for(unsigned int j = 0; j < (unsigned int)width; j++)      
        {
            for(unsigned int k = 0; k < 2; k++)      
            {
                indices.push_back(j + width * (i + k));
            }
        }
        //  The restart token goes HERE (outside the 'j' loop, inside the 'i' loop)
        // This cuts the strip only when a full horizontal row is done.
        indices.push_back(restartIndex); 
    }

    indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


}

void WorldMesh::Draw()
{
    glBindVertexArray(VAO);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glDrawElements(
        GL_TRIANGLE_STRIP,
        indexCount,
        GL_UNSIGNED_INT,
        (void*)0
    );

    glDisable(GL_PRIMITIVE_RESTART);
    glBindVertexArray(0);
}


