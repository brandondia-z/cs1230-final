#include <stdexcept>
#include "camera.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "utils/scenedata.h"
#include "settings.h"

void Camera::init(const SceneCameraData &cameraData, int width, int height) {
    m_cameraData = cameraData;
    m_aspectRatio = width/float(height);
    m_heightAngle = cameraData.heightAngle;
    float depth = height / float(2.f * glm::tan(m_heightAngle / 2.f));
    m_widthAngle = 2.f * glm::atan((width/2.f) / depth);
}

glm::vec3 Camera::getLookVector() const {
    return glm::normalize(m_cameraData.look);
}

glm::vec3 Camera::getPerpLookVector() const {
    glm::vec3 w = -glm::normalize(m_cameraData.look);
    glm::vec3 v = glm::normalize(glm::vec3{m_cameraData.up} - (glm::dot(glm::vec3{m_cameraData.up}, w)) * w);
    return glm::cross(v, w);
}

glm::mat4 Camera::getViewMatrix() const {
    glm::vec3 w = -glm::normalize(m_cameraData.look);
    glm::vec3 v = glm::normalize(glm::vec3{m_cameraData.up} - (glm::dot(glm::vec3{m_cameraData.up}, w)) * w);
    glm::vec3 u = glm::cross(v, w);
    glm::mat4 rotation {u.x, v.x, w.x, 0, u.y, v.y, w.y, 0, u.z, v.z, w.z, 0, 0, 0, 0, 1.f};
    glm::mat4 translation {1.f, 0, 0, 0, 0, 1.f, 0, 0, 0, 0, 1.f, 0, -m_cameraData.pos.x, -m_cameraData.pos.y, -m_cameraData.pos.z, 1.f};
    return rotation * translation;
}

glm::mat4 Camera::getProjectionMatrix() const {
    float c = -settings.nearPlane/settings.farPlane;
    glm::mat4 scalingMatrix = {1.f / (settings.farPlane * glm::tan(m_widthAngle/2.f)), 0, 0, 0,
                               0, 1.f / (settings.farPlane * glm::tan(m_heightAngle/2.f)), 0, 0,
                               0, 0, (1.f / settings.farPlane), 0,
                               0, 0, 0, 1.f};
    glm::mat4 unhingingMatrix = {1.f, 0, 0, 0,
                                 0, 1.f, 0, 0,
                                 0, 0, 1.f / (1.f + c), -1.f,
                                 0, 0, -c/(1.f + c), 0};
    glm::mat4 remappingMatrix = {1.f, 0, 0, 0,
                                 0, 1.f, 0, 0,
                                 0, 0, -2.f, 0,
                                 0, 0, -1.f, 1.f};
    return remappingMatrix* unhingingMatrix * scalingMatrix;
}

float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

float Camera::getHeightAngle() const {
    return m_heightAngle;
}

float Camera::getWidthAngle() const {
    return m_widthAngle;
}

float Camera::getFocalLength() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}

float Camera::getAperture() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}
