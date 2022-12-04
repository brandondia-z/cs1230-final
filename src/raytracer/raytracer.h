#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include <tuple>
#include <unordered_map>

using namespace std;

// A forward declaration for the RaytraceScene class

class RayTraceScene;

// A class representing a ray-tracer

class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
    };

    struct Ray {
        glm::vec4 origin;
        glm::vec4 direction;
    };

public:
    RayTracer(Config config);

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    struct Texture {
        const vector<RGBA> textureRGBA;
        int width;
        int height;
    };

    vector<glm::mat4> m_inverseCTMCache;
    bool m_cached = false;
    unordered_map<string, Texture> m_cachedTextures;

    void render(RGBA *imageData, const RayTraceScene &scene);
    RGBA traceRay(Ray ray, const RayTraceScene &scene);
    tuple<vector<float>, glm::vec3, bool> cylinderIntersect(Ray ray);
    tuple<vector<float>, glm::vec3, bool> coneIntersect(Ray ray);
    tuple<vector<float>, glm::vec3, bool> sphereIntersect(Ray ray);
    tuple<vector<float>, glm::vec3, bool> cubeIntersect(Ray ray);
    vector<float> quadraticEquation(float a, float b, float c);
    RGBA toRGBA(const glm::vec4 &illumination);
    RGBA phong(glm::vec3 position, glm::vec3 normal, glm::vec3 directionToCamera, SceneMaterial &material,
               const vector<SceneLightData> &lights, const SceneGlobalData &globalData, const vector<RenderShapeData> &shapes, const RayTraceScene &scene, bool recursive, RGBA textureColor);
    bool checkShadow(const glm::vec3 origin, const glm::vec3 direction, const vector<RenderShapeData> &shapes, bool isDirectional, float tCheck);
    glm::vec3 recursiveReflection(const glm::vec3 origin, const glm::vec3 direction, int depth, const RayTraceScene &scene);
    RGBA mapTexture(const glm::vec3 intersection, const glm::vec3 normal, const RenderShapeData shape);

private:
    const Config m_config;
};

