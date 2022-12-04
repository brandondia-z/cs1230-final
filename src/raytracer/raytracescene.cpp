#include <stdexcept>
#include "raytracescene.h"
#include "utils/sceneparser.h"

using namespace std;

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData) {
    m_width = width;
    m_height = height;
    m_globalData = metaData.globalData;
    m_cameraData = metaData.cameraData;
    m_camera.init(metaData.cameraData, width, height);
    m_shapes = metaData.shapes;
    m_lights = metaData.lights;
}


const int& RayTraceScene::width() const {
    return m_width;
}

const int& RayTraceScene::height() const {
    return m_height;
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    return m_globalData;
}

const Camera& RayTraceScene::getCamera() const {
    return m_camera;
}

const vector<RenderShapeData>& RayTraceScene::getShapes() const {
    return m_shapes;
}
