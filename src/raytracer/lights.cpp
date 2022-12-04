#include "raytracer.h"
#include "raytracescene.h"
#include <iostream>

RGBA RayTracer::toRGBA(const glm::vec4 &illumination) {
    float r = 255 * min(max(illumination.x, 0.f), 1.f);
    float g = 255 * min(max(illumination.y, 0.f), 1.f);
    float b = 255 * min(max(illumination.z, 0.f), 1.f);
    return RGBA{uint8_t(r), uint8_t(g), uint8_t(b)};
}

RGBA RayTracer::phong(glm::vec3 position, glm::vec3 normal, glm::vec3 directionToCamera, SceneMaterial &material,
                      const vector<SceneLightData> &lights, const SceneGlobalData &globalData, const vector<RenderShapeData> &shapes, const RayTraceScene &scene, bool recursive, RGBA textureColor) {
    normal = glm::normalize(normal);
    directionToCamera = glm::normalize(directionToCamera);

    glm::vec4 illumination(0, 0, 0, 1);

    float phong_r = globalData.ka * material.cAmbient.x;
    float phong_g = globalData.ka * material.cAmbient.y;
    float phong_b = globalData.ka * material.cAmbient.z;

    for (const SceneLightData &light : lights) {
        float f_att, lambert, reflection, distance, redColor, greenColor, blueColor, innerTheta, outerTheta, currAngle, falloff, interpolationR, interpolationG, interpolationB;
        glm::vec3 source, specVec, textureFloats;

        switch (light.type) {
            case LightType::LIGHT_POINT:

                distance = sqrt(pow(light.pos.x - position.x, 2) + pow(light.pos.y - position.y, 2) + pow(light.pos.z - position.z, 2));

                if (RayTracer::checkShadow(position, glm::normalize(glm::vec3{light.pos} - position), shapes, false, distance)) {
                    break;
                }

                f_att = min(1.f, float(1.f / (light.function.x + (distance * light.function.y) + (pow(distance, 2) * light.function.z))));

                lambert = max(0.f, glm::dot(normal, glm::normalize(glm::vec3{light.pos} - position)));

                textureFloats = {(textureColor.r/255.f), (textureColor.g/255.f), (textureColor.b/255.f)};

                interpolationR = (material.blend * textureFloats.r) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.x));
                interpolationG = (material.blend * textureFloats.g) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.y));
                interpolationB = (material.blend * textureFloats.b) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.z));

                phong_r += (f_att * light.color.x) * (interpolationR * (lambert));
                phong_g += (f_att * light.color.y) * (interpolationG * (lambert));
                phong_b += (f_att * light.color.z) * (interpolationB * (lambert));

                source = glm::normalize(glm::vec3{position - glm::vec3{light.pos}});
                specVec = source - (2.f * glm::dot(source, normal) * normal);
                reflection = max(0.f, glm::dot(-glm::normalize(specVec), -directionToCamera));

                phong_r += (f_att * light.color.x) * ((globalData.ks * material.cSpecular.x) * (pow(reflection, material.shininess)));
                phong_g += (f_att * light.color.y) * ((globalData.ks * material.cSpecular.y) * (pow(reflection, material.shininess)));
                phong_b += (f_att * light.color.z) * ((globalData.ks * material.cSpecular.z) * (pow(reflection, material.shininess)));

                break;

            case LightType::LIGHT_DIRECTIONAL:

                if (RayTracer::checkShadow(position, glm::normalize(glm::vec3{-light.dir}), shapes, true, 0.00)) {
                    break;
                }

                f_att = 1.f;

                lambert = max(0.f, glm::dot(normal, glm::normalize(glm::vec3{-light.dir})));

                textureFloats = {(textureColor.r/255.f), (textureColor.g/255.f), (textureColor.b/255.f)};

                interpolationR = (material.blend * textureFloats.r) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.x));
                interpolationG = (material.blend * textureFloats.g) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.y));
                interpolationB = (material.blend * textureFloats.b) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.z));

                phong_r += (f_att * light.color.x) * (interpolationR * (lambert));
                phong_g += (f_att * light.color.y) * (interpolationG * (lambert));
                phong_b += (f_att * light.color.z) * (interpolationB * (lambert));

                source = glm::normalize(glm::vec3{light.dir});
                specVec = source - (2.f * glm::dot(source, normal) * normal);
                reflection = max(0.f, glm::dot(-glm::normalize(specVec), -directionToCamera));

                phong_r += (f_att * light.color.x) * ((globalData.ks * material.cSpecular.x) * (pow(reflection, material.shininess)));
                phong_g += (f_att * light.color.y) * ((globalData.ks * material.cSpecular.y) * (pow(reflection, material.shininess)));
                phong_b += (f_att * light.color.z) * ((globalData.ks * material.cSpecular.z) * (pow(reflection, material.shininess)));

                break;

            case LightType::LIGHT_SPOT:

                distance = sqrt(pow(light.pos.x - position.x, 2) + pow(light.pos.y - position.y, 2) + pow(light.pos.z - position.z, 2));

                if (RayTracer::checkShadow(position, glm::normalize(glm::vec3{light.pos} - position), shapes, false, distance)) {
                    break;
                }

                f_att = min(1.f, float(1.f / (light.function.x + (distance * light.function.y) + (pow(distance, 2) * light.function.z))));

                lambert = min(1.f, max(0.f, glm::dot(normal, glm::normalize(glm::vec3{light.pos} - position))));
                outerTheta = light.angle;
                innerTheta = light.angle - light.penumbra;

                currAngle = glm::acos(glm::dot(glm::normalize(glm::vec3{light.dir}), glm::normalize(position - glm::vec3{light.pos})));

                if (currAngle <= innerTheta) {
                    redColor = light.color.x;
                    greenColor = light.color.y;
                    blueColor = light.color.z;
                } else if (innerTheta < currAngle && currAngle <= outerTheta) {
                    falloff = -2.f * pow( (currAngle - innerTheta) / (outerTheta - innerTheta), 3.f) +
                            3.f * pow( (currAngle - innerTheta) / (outerTheta - innerTheta), 2.f);
                    redColor = light.color.x * (1.f - falloff);
                    greenColor = light.color.y * (1.f - falloff);
                    blueColor = light.color.z * (1.f - falloff);
                } else {
//                    redColor = 0;
//                    greenColor = 0;
//                    blueColor = 0;
                    break;
                }

                textureFloats = {(textureColor.r/255.f), (textureColor.g/255.f), (textureColor.b/255.f)};

                interpolationR = (material.blend * textureFloats.r) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.x));
                interpolationG = (material.blend * textureFloats.g) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.y));
                interpolationB = (material.blend * textureFloats.b) + ((1.f - material.blend) * (globalData.kd * material.cDiffuse.z));

                phong_r += (f_att * redColor) * (interpolationR * (lambert));
                phong_g += (f_att * greenColor) * (interpolationG * (lambert));
                phong_b += (f_att * blueColor) * (interpolationB * (lambert));

                source = glm::normalize(glm::vec3{position - glm::vec3{light.pos}});
                specVec = source - (2.f * glm::dot(source, normal) * normal);
                reflection = min(1.f, max(0.f, glm::dot(-glm::normalize(specVec), -directionToCamera)));

                phong_r += (f_att * redColor) * ((globalData.ks * material.cSpecular.x) * (pow(reflection, material.shininess)));
                phong_g += (f_att * greenColor) * ((globalData.ks * material.cSpecular.y) * (pow(reflection, material.shininess)));
                phong_b += (f_att * blueColor) * ((globalData.ks * material.cSpecular.z) * (pow(reflection, material.shininess)));

                break;

            case LightType::LIGHT_AREA:
                // Not supported, but put here to suppress QT Warnings
                break;
        }
    }

    if (!recursive && (material.cReflective.x != 0 || material.cReflective.y != 0 || material.cReflective.z != 0)) {

        glm::vec3 reflection = glm::reflect(-directionToCamera, normal);
        glm::vec3 reflectedColor = RayTracer::recursiveReflection(position, reflection, 0, scene);

        if (reflectedColor.x < 0) {
            reflectedColor.x = 0.f;
        } else if (reflectedColor.x > 1) {
            reflectedColor.x = 1.f;
        }
        if (reflectedColor.y < 0) {
            reflectedColor.y = 0.f;
        } else if (reflectedColor.y > 1) {
            reflectedColor.y = 1.f;
        }
        if (reflectedColor.z < 0) {
            reflectedColor.z = 0.f;
        } else if (reflectedColor.z > 1) {
            reflectedColor.z = 1.f;
        }

        phong_r += (scene.m_globalData.ks * material.cReflective.x) * reflectedColor.r;
        phong_g += (scene.m_globalData.ks * material.cReflective.y) * reflectedColor.g;
        phong_b += (scene.m_globalData.ks * material.cReflective.z) * reflectedColor.b;

    }

    illumination = {phong_r, phong_g, phong_b, 1};

    RGBA returnValue = RayTracer::toRGBA(illumination);
    return returnValue;
}

bool RayTracer::checkShadow(glm::vec3 origin, glm::vec3 direction, const vector<RenderShapeData> &shapes, bool isDirectional, float tCheck) {
    tuple<vector<float>, glm::vec3, bool> result;
    bool test = false;
    Ray objectRay;
    float epsilon = 0.01f;
    glm::vec3 normal;
    RenderShapeData closest;
    Ray closestRay;
    glm::vec4 offsetDirection = {direction.x, direction.y, direction.z, 0};
    glm::vec4 offsetOrigin = {origin.x, origin.y, origin.z, 1};
    offsetOrigin += (epsilon * offsetDirection);
    int shapeIndex = 0;
    for (const RenderShapeData &shape : shapes) {
        objectRay = {m_inverseCTMCache[shapeIndex] * offsetOrigin, m_inverseCTMCache[shapeIndex] * offsetDirection}; // to Object space
        shapeIndex++;
        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                result = RayTracer::cubeIntersect(objectRay);
                test = get<2>(result);
                if (test && (isDirectional || get<0>(result)[0] < tCheck + epsilon)) {
                    return true;
                }
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                result = RayTracer::coneIntersect(objectRay);
                test = get<2>(result);
                if (test && (isDirectional || get<0>(result)[0] < tCheck + epsilon)) {
                    return true;
                }
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                result = RayTracer::cylinderIntersect(objectRay);
                test = get<2>(result);
                if (test && (isDirectional || get<0>(result)[0] < tCheck + epsilon)) {
                    return true;
                }
                break;
            case PrimitiveType::PRIMITIVE_SPHERE:
                result = RayTracer::sphereIntersect(objectRay);
                test = get<2>(result);
                if (test && (isDirectional || get<0>(result)[0] < tCheck + epsilon)) {
                    return true;
                }
                break;
            case PrimitiveType::PRIMITIVE_MESH:
                // not implemented, put here to suppress QT warnings
                break;
            case PrimitiveType::PRIMITIVE_TORUS:
                // not implemented, put here to suppress QT warnings
                break;
        }
    }
    return false;
}

glm::vec3 RayTracer::recursiveReflection(glm::vec3 origin, glm::vec3 direction, int depth, const RayTraceScene &scene) {
    float t = 0.f, min = INFINITY, epsilon = 0.01f;
    RenderShapeData closest;
    vector<RenderShapeData> shapes = scene.m_shapes;
    glm::vec3 normal;
    tuple<vector<float>, glm::vec3, bool> result;
    Ray objectRay, closestRay;
    glm::vec4 offsetDirection = {direction.x, direction.y, direction.z, 0};
    glm::vec4 offsetOrigin = {origin.x, origin.y, origin.z, 1};
    offsetOrigin += (epsilon * offsetDirection);
    int shapeIndex = 0;
    for (const RenderShapeData &shape : shapes) {
        objectRay = {m_inverseCTMCache[shapeIndex] * offsetOrigin, m_inverseCTMCache[shapeIndex] * offsetDirection}; // to Object space
        shapeIndex++;
        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                result = RayTracer::cubeIntersect(objectRay);
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                result = RayTracer::coneIntersect(objectRay);
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                result = RayTracer::cylinderIntersect(objectRay);
                break;
            case PrimitiveType::PRIMITIVE_SPHERE:
                result = RayTracer::sphereIntersect(objectRay);
                break;
            case PrimitiveType::PRIMITIVE_MESH:
                // not implemented, put here to suppress QT warnings
                break;
            case PrimitiveType::PRIMITIVE_TORUS:
                // not implemented, put here to suppress QT warnings
                break;
        }
        if (get<0>(result).empty()) {
            continue;
        } else {
            t = get<0>(result)[0];
        }
        if (t < min) {
            min = t;
            normal = get<1>(result);
            closest = shape;
            closestRay = {objectRay.origin, objectRay.direction};
        }
    }
    if (min == INFINITY) {
        return {0.f, 0.f, 0.f};
    }
    // Compute object space normal at object space intersection
    glm::vec4 worldIntersectPosition = closest.ctm * closestRay.origin;
    glm::vec4 worldIntersectDirection = closest.ctm * closestRay.direction;
    glm::vec3 intersect = glm::vec3{worldIntersectPosition + (min * worldIntersectDirection)};
    // Transform object space normal to world space
    glm::vec3 worldNormal = glm::inverse(glm::transpose(glm::mat3(closest.ctm))) * normal;
    // Use normal and intersection point (both in world space) for lighting computation

    RGBA textureColor;
    if (closest.primitive.material.textureMap.isUsed) {
        textureColor = RayTracer::mapTexture(glm::vec3{closestRay.origin + (min * closestRay.direction)}, normal, closest);
    } else {
        textureColor = RGBA{0, 0, 0};
    }

    RGBA newColor = RayTracer::phong(intersect, worldNormal, glm::vec3{offsetOrigin} - intersect, closest.primitive.material, scene.m_lights, scene.m_globalData , scene.m_shapes, scene, true, textureColor);
    glm::vec3 reflected = {newColor.r/255.f, newColor.g/255.f, newColor.b/255.f};

    if (reflected.x < 0) {
        reflected.x = 0.f;
    } else if (reflected.x > 1) {
        reflected.x = 1.f;
    }
    if (reflected.y < 0) {
        reflected.y = 0.f;
    } else if (reflected.y > 1) {
        reflected.y = 1.f;
    }
    if (reflected.z < 0) {
        reflected.z = 0.f;
    } else if (reflected.z > 1) {
        reflected.z = 1.f;
    }


    if (depth < 3 && (closest.primitive.material.cReflective.x != 0 || closest.primitive.material.cReflective.y != 0 || closest.primitive.material.cReflective.z != 0)) {
        glm::vec3 normalDirection = glm::vec3{worldIntersectDirection};
        glm::vec3 normalWorld = glm::normalize(worldNormal);
        glm::vec3 reflection = glm::reflect(normalDirection, normalWorld);
        glm::vec3 next = RayTracer::recursiveReflection(intersect, reflection, depth + 1, scene);
        reflected.x += next.x * (scene.m_globalData.ks * closest.primitive.material.cReflective.x);
        reflected.y += next.y * (scene.m_globalData.ks * closest.primitive.material.cReflective.y);
        reflected.z += next.z * (scene.m_globalData.ks * closest.primitive.material.cReflective.z);
        return reflected;
    } else {
        return reflected;
    }
}
