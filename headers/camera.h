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

};