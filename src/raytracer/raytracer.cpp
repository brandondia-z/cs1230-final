#include "raytracer.h"
#include "raytracescene.h"
#include <iostream>
#include <ostream>
#include <QString>
#include <QImage>

using namespace std;

RayTracer::RayTracer(Config config) :
    m_config(config)
{}

void RayTracer::render(RGBA *imageData, const RayTraceScene &scene) {
    // Note that we're passing `data` as a pointer (to its first element)
    // Recall from Lab 1 that you can access its elements like this: `data[i]`
    Camera camera = scene.getCamera();
    int width = scene.width(), height = scene.height(), k = 1;
    glm::mat4 inverse = glm::inverse(camera.getViewMatrix());

    // puts textures in a map, filename -> vector<RGBA>
    for (const RenderShapeData &shape : scene.m_shapes) {
        string filename = shape.primitive.material.textureMap.filename;
        if (filename.empty() || (m_cachedTextures.find(filename) != m_cachedTextures.end())) {
            continue;
        }
        QImage textureImage;
        QString file = QString::fromStdString(filename);
        if (!textureImage.load(file)) {
            cout << "Error loading texture" << endl;
            continue;
        }
        vector<RGBA> textureRGBA;
        textureImage = textureImage.convertToFormat(QImage::Format_RGBX8888);
        int w = textureImage.width();
        int h = textureImage.height();
        QByteArray arr = QByteArray::fromRawData((const char*) textureImage.bits(), textureImage.sizeInBytes());

        textureRGBA.clear();
        textureRGBA.reserve(w * h);
        for (int i = 0; i < arr.size() / 4.f; i++){
            textureRGBA.push_back(RGBA{(uint8_t) arr[4*i], (uint8_t) arr[4*i+1], (uint8_t) arr[4*i+2], (uint8_t) arr[4*i+3]});
        }
        Texture texture {textureRGBA, w, h};
        m_cachedTextures.insert({filename, texture});
    }

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            float x = ((i + 0.5f) / float(width)) - 0.5f;
            float y = ((height - 1 - j + 0.5f) / float(height)) - 0.5f;
            float theta_h = camera.getHeightAngle();
            float theta_w = camera.getHeightAngle() * float(camera.getAspectRatio());
            float u = 2 * k * std::tan(theta_w / 2.0f);
            float v = 2 * k * std::tan(theta_h / 2.0f);
            glm::vec3 uvk = glm::vec3{u * x, v * y, -k};
            glm::vec3 pEye = glm::vec3{0, 0, 0};
            glm::vec3 dir = uvk - pEye;
            glm::vec4 origin = inverse * glm::vec4{pEye.x, pEye.y, pEye.z, 1};
            glm::vec4 direction = inverse * glm::vec4{dir.x, dir.y, dir.z, 0};
            Ray ray{origin, direction}; // World Space
            RGBA color = traceRay(ray, scene);
            int index = (j * width) + i;
            imageData[index] = color;
        }
    }
}

RGBA RayTracer::traceRay(Ray ray, const RayTraceScene &scene) {
    float t = 0.f, min = INFINITY;
    RenderShapeData closest;
    vector<RenderShapeData> shapes = scene.m_shapes;
    glm::vec3 normal;
    tuple<vector<float>, glm::vec3, bool> result;
    Ray objectRay;
    Ray closestRay;
    int shapeIndex = 0;
    for (const RenderShapeData &shape : shapes) {
        if (!m_cached) {
            m_inverseCTMCache.push_back(glm::inverse(shape.ctm));
        }
        objectRay = {m_inverseCTMCache[shapeIndex] * ray.origin, m_inverseCTMCache[shapeIndex] * ray.direction}; // to Object space
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
        return RGBA{0, 0, 0};
    }
    glm::vec4 worldIntersectPosition = closest.ctm * closestRay.origin;
    glm::vec4 worldIntersectDirection = closest.ctm * closestRay.direction;
    glm::vec3 intersect = glm::vec3{worldIntersectPosition + (min * worldIntersectDirection)};
    // Transform object space normal to world space
    glm::vec3 worldNormal = glm::inverse(glm::transpose(glm::mat3(closest.ctm))) * normal;
    // Use normal and intersection point (both in world space) for lighting computation
    RGBA textureColor;
    if (closest.primitive.material.textureMap.isUsed) {
        textureColor = RayTracer::mapTexture(glm::vec3{closestRay.origin + (min * closestRay.direction)}, glm::normalize(normal), closest);
    } else {
        textureColor = RGBA{0, 0, 0};
    }
    return RayTracer::phong(intersect, worldNormal, glm::vec3{ray.origin} - intersect, closest.primitive.material, scene.m_lights, scene.m_globalData , scene.m_shapes, scene, false, textureColor);
}

RGBA RayTracer::mapTexture(glm::vec3 intersection, glm::vec3 normal, RenderShapeData shape) {
    const Texture texture = m_cachedTextures[shape.primitive.material.textureMap.filename];
    float u=0, v=0, theta, phi;
    int c, r;
    switch (shape.primitive.type) {
        case PrimitiveType::PRIMITIVE_CUBE:
            //cube
            if (normal.x == 1) {
                u = 0.5f - intersection.z;
                v = intersection.y + 0.5f;
            } else if (normal.x == -1) {
                u = intersection.z + 0.5f;
                v = intersection.y - 0.5f;
            } else if (normal.y == 1) {
                u = intersection.x - 0.5f;
                while (u < 0) {
                    u++;
                }
                v = 1.5f - intersection.z;
                while (v > 0) {
                    v--;
                }
            } else if (normal.y == -1) {
                u = intersection.x + 0.5f;
                v = intersection.z + 0.5f;
            } else if (normal.z == 1) {
                u = intersection.x + 0.5f;
                v = intersection.y + 0.5f;
            } else if (normal.z == -1) {
                u = 0.5f - intersection.x;
                v = intersection.y + 0.5f;
            }
            break;
        case PrimitiveType::PRIMITIVE_CONE:
            if (normal.y == -1) {
                u = intersection.x - 0.5f;
                v = intersection.z + 0.5f;
            } else {
                //rounded
                theta = glm::atan(intersection.z, intersection.x);
                if (theta < 0) {
                    u = -theta/(2.f*M_PI);
                } else if (theta >= 0) {
                    u = 1.f - (theta/(2.f*M_PI));
                }
                v = intersection.y  + 0.5f;
            }
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            if (normal.y == 1) {
                u = intersection.x - 0.5f;
                while (u < 0) {
                    u++;
                }
                v = 1.5f - intersection.z;
                while (v > 0) {
                    v--;
                }
            } else if (normal.y == -1) {
                u = intersection.x - 0.5f;
                v = intersection.z + 0.5f;
            } else {
                theta = glm::atan(intersection.z, intersection.x);
                if (theta < 0) {
                    u = -theta/(2*M_PI);
                } else if (theta >= 0) {
                    u = 1.f - (theta/(2.f*M_PI));
                }
                v = intersection.y  + 0.5f;
            }
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:

            phi = glm::asin(intersection.y/0.5f);
            v = 0.5f + (phi/M_PI);
            if (v == 0 || v == 1) {
                u = 0.5f;
                break;
            }
            theta = glm::atan(intersection.z, intersection.x);
            if (theta < 0) {
                u = -theta/(2.f*M_PI);
            } else if (theta >= 0) {
                u = 1.f - (theta/(2.f*M_PI));
            }

            break;
        case PrimitiveType::PRIMITIVE_MESH:
            // not implemented, put here to suppress QT warnings
            break;
        case PrimitiveType::PRIMITIVE_TORUS:
            // not implemented, put here to suppress QT warnings
            break;
    }
    int width = texture.width;
    int height = texture.height;
    if (u == 1.f) {
        width--;
    }
    if (v == 0.f) {
        height--;
    }
    float repeatU = shape.primitive.material.textureMap.repeatU;
    float repeatV = shape.primitive.material.textureMap.repeatV;

    if (repeatU > 1) {
        c = int(floor(u * repeatU * width)) % width;
    } else {
        c = int(floor(u * width));
    }
    if (repeatV > 1) {
        r = int(floor((1 - v) * repeatV * height)) % height;
    } else {
        r = int(floor((1 - v) * height));
    }

    int index = ((width * r) + c);
    return texture.textureRGBA[index];
}
