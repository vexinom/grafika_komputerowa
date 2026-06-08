# include "camera.h"

Camera::Camera()
{
    Position = glm::vec3(15.0f, 15.0f, 15.0f);
    Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    FOV = 45.0f;
    Aspect = 800.0f / 600.0f;

    NearPlane = 0.1f;
    FarPlane = 1500.0f;
}

glm::mat4 Camera::GetViewMatrix() const
{
    glm::mat4 rotation = glm::mat4_cast(glm::conjugate(Orientation));
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), -Position);

    return rotation * translation;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(FOV), Aspect, NearPlane, FarPlane);
}

void Camera::Rotate(float yawOffset, float pitchOffset)
{
    Yaw -= yawOffset; 
    Pitch -= pitchOffset;

    float pitchLimit = glm::radians(89.0f);
    if (Pitch > pitchLimit) 
    {
        Pitch = pitchLimit;
    }
    
    if (Pitch < -pitchLimit) 
    {
        Pitch = -pitchLimit;
    }

    glm::quat qYaw = glm::angleAxis(Yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat qPitch = glm::angleAxis(Pitch, glm::vec3(1.0f, 0.0f, 0.0f));

    Orientation = glm::normalize(qYaw * qPitch);
}

void Camera::MoveLocal(const glm::vec3 & dir)
{
    Position += Orientation * dir;
}