#pragma once

#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"

using namespace std;

// A class representing a scene to be ray-traced

// Feel free to make your own design choices for RayTraceScene, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class RayTraceScene
{
public:
    RayTraceScene(int width, int height, const RenderData &metaData);

    // The getter of the width of the scene
    const int& width() const;

    // The getter of the height of the scene
    const int& height() const;

    // The getter of the global data of the scene
    const SceneGlobalData& getGlobalData() const;

    // The getter of the shared pointer to the camera instance of the scene
    const Camera& getCamera() const;

    // The getter of the shapes in the scene
    const vector<RenderShapeData>& getShapes() const;

    int m_width;
    int m_height;
    SceneGlobalData m_globalData;
    SceneCameraData m_cameraData;
    Camera m_camera;
    vector<RenderShapeData> m_shapes;
    vector<SceneLightData> m_lights;
};
