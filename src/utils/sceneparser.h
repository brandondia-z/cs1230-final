#pragma once

#include "scenedata.h"
#include "OBJ_Loader.h"
#include <vector>
#include <string>

// Struct which contains data for a single primitive, to be used for rendering
struct RenderShapeData {
    ScenePrimitive primitive;
    glm::mat4 ctm; // the cumulative transformation matrix
    objl::Mesh meshData;
};

// Struct which contains all the data needed to render a scene
struct RenderData {
    SceneGlobalData globalData;
    SceneCameraData cameraData;

    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;
};

class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static int parse(std::string filepath, RenderData &renderData);
    static int parseMesh(std::string filepath, RenderData &renderData);
    static void dfs(SceneNode &node, RenderData &renderData, glm::mat4 ctm);
    static glm::mat4 visit(SceneNode &node, RenderData &renderData, glm::mat4 ctm);
    static glm::mat4 doCalc(TransformationType type, SceneTransformation transformation);
};

