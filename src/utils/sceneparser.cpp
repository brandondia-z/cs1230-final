#include "sceneparser.h"
#include "scenefilereader.h"
#include "glm/gtx/transform.hpp"

#include <chrono>
#include <memory>
#include <iostream>

int SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readXML();
    if (!success) {
        return -1;
    }
    renderData.globalData = fileReader.getGlobalData();
    renderData.lights = fileReader.getLights();
    renderData.cameraData = fileReader.getCameraData();

    std::vector<SceneTransformation> currTransformations;
    SceneNode* root = fileReader.getRootNode();
    renderData.shapes.clear();
    glm::mat4 identity = glm::mat4(1.f);
    SceneParser::dfs(*root, renderData, identity);

    return 0;
}

int SceneParser::parseMesh(std::string filepath, RenderData &renderData) {
    objl::Loader loader;
    bool success = loader.LoadFile(filepath);
    if (!success) {
        return -1;
    }

    renderData.globalData.ka = 0.5f;
    renderData.globalData.kd = 0.5f;
    renderData.globalData.ks = 0.5f;

    renderData.cameraData.pos = glm::vec4{-10, 5, 5, 1};
    renderData.cameraData.look = glm::vec4{0, 0, 0, 0};
    renderData.cameraData.up = glm::vec4{0, 1, 0, 0};
    renderData.cameraData.heightAngle = 30;

    SceneLightData light;
    light.id = 0;
    light.type = LightType::LIGHT_DIRECTIONAL;
    light.color = glm::vec4{1, 1, 1, 1};
    light.dir = glm::vec4{0, -1, 0, 0};
    renderData.lights = std::vector{light};

    RenderShapeData mesh;
    mesh.meshData = loader.LoadedMeshes[0];
    mesh.primitive.type = PrimitiveType::PRIMITIVE_MESH;
    mesh.primitive.material.cAmbient = glm::vec4{loader.LoadedMeshes[0].MeshMaterial.Ka.X,
            loader.LoadedMeshes[0].MeshMaterial.Ka.Y, loader.LoadedMeshes[0].MeshMaterial.Ka.Z, 1};
    mesh.primitive.material.cDiffuse = glm::vec4{loader.LoadedMeshes[0].MeshMaterial.Kd.X,
            loader.LoadedMeshes[0].MeshMaterial.Kd.Y, loader.LoadedMeshes[0].MeshMaterial.Kd.Z, 1};
    mesh.primitive.material.cSpecular = glm::vec4{loader.LoadedMeshes[0].MeshMaterial.Ks.X,
            loader.LoadedMeshes[0].MeshMaterial.Ks.Y, loader.LoadedMeshes[0].MeshMaterial.Ks.Z, 1};
    mesh.primitive.material.shininess = loader.LoadedMeshes[0].MeshMaterial.Ns;

    renderData.shapes = std::vector{mesh};

    return 1;
}

void SceneParser::dfs(SceneNode &node, RenderData &renderData, glm::mat4 ctm) {
    glm::mat4 updated = SceneParser::visit(node, renderData, ctm);
    for (SceneNode* child : node.children) {
        SceneParser::dfs(*child, renderData, updated);
    }
    for (ScenePrimitive* primitive : node.primitives) {
        renderData.shapes.push_back(RenderShapeData{*primitive, updated});
    }
}

glm::mat4 SceneParser::visit(SceneNode &node, RenderData &renderData, glm::mat4 ctm) {
    for (int i = 0; i < node.transformations.size(); i++) {
        ctm = ctm * doCalc(node.transformations[i]->type, *node.transformations[i]);
    }
    return ctm;
}

glm::mat4 SceneParser::doCalc(TransformationType type, SceneTransformation transformation) {
    switch (type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            return glm::translate(transformation.translate);
        case TransformationType::TRANSFORMATION_SCALE:
            return glm::scale(transformation.scale);
        case TransformationType::TRANSFORMATION_ROTATE:
            return glm::rotate(transformation.angle, transformation.rotate);
        default:
            return transformation.matrix;
    }
}
