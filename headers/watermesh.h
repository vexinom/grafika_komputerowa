#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

class WaterMesh
{
public:
    void Init(int width, int height, float waterLevel);
    void Draw(glm::mat4 &viewProjection);

    private:
        GLuint VAO, VBO, EBO;
        int indexCount;
        float waterLevel;

};