#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
    public:
        glm::vec3 Position;
        glm::quat Orientation; //orientation in quaternions

        float FOV;
        float Aspect;
        float NearPlane;
        float FarPlane;

        float Yaw = 0.0f;
        float Pitch = 0.0f;

        // default constructor
        Camera();

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix() const;

        void Rotate(float yawOffset, float pitchOffset);
        void MoveLocal(const glm::vec3 & dir);
<<<<<<< HEAD
        void SetView(const glm::vec3& position, float yaw, float pitch);
        void LookAt(const glm::vec3& eye, const glm::vec3& target);
=======
>>>>>>> c03a168ac1ee55ce4304085a785f225cd8fd36c5

};