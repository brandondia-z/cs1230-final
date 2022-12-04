#pragma once

#include "utils/scenedata.h"
#include <glm/glm.hpp>

// A class representing a virtual camera.

// Feel free to make your own design choices for Camera class, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class Camera {
public:
    // Returns look vector
    glm::vec3 getLookVector() const;

    // Resets the Camera's look vector to a new vector
    void setPosVector(glm::vec4 translated);

    glm::vec4 getPosition() const;

    SceneCameraData getData() const;

    // Returns vector perpendicular to look and up vectors
    glm::vec3 getPerpLookVector() const;

    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;

    // Returns the projection matrix for the current camera settings.
    glm::mat4 getProjectionMatrix() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the width angle of the camera in RADIANS.
    float getWidthAngle() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    // This is for the depth of field extra-credit feature only;
    // You can ignore if you are not attempting to implement depth of field.
    float getAperture() const;

    void init(const SceneCameraData &cameraData, int width, int height);
    SceneCameraData m_cameraData;
    float m_aspectRatio;
    float m_heightAngle;
    float m_widthAngle;
};
