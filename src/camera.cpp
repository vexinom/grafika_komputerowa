# include "camera.h"

Camera::Camera()
{
    Position = glm::vec3(0.0f, 0.0f, 3.0f);
    Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    FOV = 45.0f;
    Aspect = 800.0f / 600.0f;

    NearPlane = 0.1f;
    FarPlane = 100.0f;
}

glm::mat4 Camera::GetViewMatrix() const
{
    glm::mat4 rotation = glm::mat4_cast(Orientation);
    glm::mat4 transformation = glm::translate(glm::mat4(1.0f), -Position);

    return rotation * transformation;
}

glm::mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(FOV), Aspect, NearPlane, FarPlane);
}

void Camera::Rotate(float vertical, float horizontal)
{
    glm::quat Qvertical = glm::angleAxis(vertical, glm::vec3(0, 1, 0));
    glm::quat Qhorizontal = glm::angleAxis(horizontal, glm::vec3(1, 0, 0));

    Orientation = Qvertical * Orientation;
    Orientation = Orientation * Qhorizontal;

    Orientation = glm::normalize(Orientation);
}

void Camera::MoveLocal(const glm::vec3 & dir)
{
    Position += Orientation * dir;
}