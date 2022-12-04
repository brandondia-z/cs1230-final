#include "raytracer.h"
#include <tuple>

tuple<vector<float>, glm::vec3, bool> RayTracer::cylinderIntersect(Ray ray) {
    vector<tuple<float, glm::vec3>> solutions;
    glm::vec3 P = glm::vec3(ray.origin);
    glm::vec3 d = glm::vec3(ray.direction);
    float a = powf(d.x, 2.f) + powf(d.z, 2.f);
    float b = (2.f * P.x * d.x) + (2.f * P.z * d.z);
    float c = powf(P.x, 2.f) + powf(P.z, 2.f) - 0.25f;
    vector<float> infinite = RayTracer::quadraticEquation(a, b, c);
    for (int i = 0; i < infinite.size(); i++) {
        glm::vec3 tRay = P + infinite[i] * d;
        if (tRay.y > 0.5 || tRay.y < -0.5 || infinite[i] <= 0) {
            continue;
        }
        glm::vec3 reflect = P + (infinite[i] * d);
        solutions.push_back({{infinite[i]}, {2 * reflect.x, 0, 2 * reflect.z}});
    }
    float t3 = (0.5f - P.y) / d.y;
    float t4 = (-0.5f - P.y) / d.y;
    glm::vec3 t3Ray = P + t3 * d;
    glm::vec3 t4Ray = P + t4 * d;
    float t3Check = pow(t3Ray.x, 2.f) + pow(t3Ray.z, 2.f);
    float t4Check = pow(t4Ray.x, 2.f) + pow(t4Ray.z, 2.f);
    if (t3Check <= .25 && t3 > 0) {
        solutions.push_back({{t3}, {0, 1, 0}});
    }
    if (t4Check <= .25 && t4 > 0) {
        solutions.push_back({{t4}, {0, -1, 0}});
    }
    if (solutions.empty()) {
        return {{}, {0, 0, 0}, false};
    } else {
        float min = get<0>(solutions[0]);
        glm::vec3 normal = get<1>(solutions[0]);
        for (int j = 1; j < solutions.size(); j++) {
            if (get<0>(solutions[j]) < min) {
                min = get<0>(solutions[j]);
                normal = get<1>(solutions[j]);
            }
        }
        return {{min}, normal, true};
    }
}

tuple<vector<float>, glm::vec3, bool> RayTracer::coneIntersect(Ray ray) {
    vector<tuple<float, glm::vec3>> solutions;
    glm::vec3 P = glm::vec3(ray.origin);
    glm::vec3 d = glm::vec3(ray.direction);
    float a = powf(d.x, 2.f) + powf(d.z, 2.f) - (0.25f * powf(d.y, 2.f));
    float b = (2.f * P.x * d.x) + (2.f * P.z * d.z) - (0.5f * P.y * d.y) + (0.25f * d.y);
    float c = powf(P.x, 2.f) + powf(P.z, 2.f) - (0.25f * powf(P.y, 2.f)) + (0.25f * P.y) - 0.0625f;
    vector<float> infinite = RayTracer::quadraticEquation(a, b, c);
    for (int i = 0; i < infinite.size(); i++) {
        glm::vec3 tRay = P + infinite[i] * d;
        if (tRay.y > 0.5 || tRay.y < -0.5 || infinite[i] <= 0) {
            continue;
        }
        glm::vec3 reflect = P + (infinite[i] * d);
        solutions.push_back({infinite[i], {2.f * reflect.x, -0.5f * reflect.y + 0.25f, 2.f * reflect.z}});
    }
    float t3 = (-0.5f - P.y) / d.y;
    float t3Check = pow(P.x + (t3 * d.x), 2.f) + pow(P.z + (t3 * d.z), 2.f);
    if (t3Check <= 0.25f && t3 > 0) {
        solutions.push_back({t3, {0, -1, 0}});
    }
    if (solutions.empty()) {
        return {{}, {0, 0, 0}, false};
    } else {
        float min = get<0>(solutions[0]);
        glm::vec3 normal = get<1>(solutions[0]);
        for (int j = 1; j < solutions.size(); j++) {
            if (get<0>(solutions[j]) < min) {
                min = get<0>(solutions[j]);
                normal = get<1>(solutions[j]);
            }
        }
        return {{min}, normal, true};
    }
}

tuple<vector<float>, glm::vec3, bool> RayTracer::sphereIntersect(Ray ray) {
    glm::vec3 P = glm::vec3(ray.origin);
    glm::vec3 d = glm::vec3(ray.direction);
    float a = powf(d.x, 2.f) + powf(d.y, 2.f) + powf(d.z, 2.f);
    float b = (2.f * P.x * d.x) + (2.f * P.y * d.y) + (2.f * P.z * d.z);
    float c = powf(P.x, 2.f) + powf(P.y, 2.f) + powf(P.z, 2.f) - 0.25f;
    vector<float> solutions = RayTracer::quadraticEquation(a, b, c);
    if (solutions.empty()) {
        return {{}, {0, 0, 0}, false};
    } else {
        float min = solutions[0];
        for (int j = 1; j < solutions.size(); j++) {
            if (solutions[j] < min) {
                min = solutions[j];
            }
        }
        glm::vec3 reflect = P + (min * d);
        return {{min}, {2 * reflect.x, 2 * reflect.y, 2 * reflect.z}, true};
    }
}

tuple<vector<float>, glm::vec3, bool> RayTracer::cubeIntersect(Ray ray) {
    std::vector<tuple<float, glm::vec3>> solutions;
    glm::vec4 newRay;

    float t1 = (0.5f - ray.origin.x)/ray.direction.x;
    newRay = ray.origin + t1*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t1 > 0) {
        solutions.push_back({{t1}, glm::vec3{1, 0, 0}});
    }
    float t2 = (-0.5f - ray.origin.x)/ray.direction.x;
    newRay = ray.origin + t2*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t2 > 0) {
        solutions.push_back({{t2}, glm::vec3{-1, 0, 0}});
    }
    float t3 = (0.5f - ray.origin.y)/ray.direction.y;
    newRay = ray.origin + t3*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t3 > 0) {
        solutions.push_back({{t3}, glm::vec3{0, 1, 0}});
    }
    float t4 = (-0.5f - ray.origin.y)/ray.direction.y;
    newRay = ray.origin + t4*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t4 > 0) {
        solutions.push_back({{t4}, glm::vec3{0, -1, 0}});
    }
    float t5 = (0.5f - ray.origin.z)/ray.direction.z;
    newRay = ray.origin + t5*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t5 > 0) {
        solutions.push_back({{t5}, glm::vec3{0, 0, 1}});
    }
    float t6 = (-0.5f - ray.origin.z)/ray.direction.z;
    newRay = ray.origin + t6*ray.direction;
    if (newRay.x >= -0.5001 && newRay.x <= 0.5001 && newRay.y >= -0.5001 && newRay.y <= 0.5001 && newRay.z >= -0.5001 && newRay.z <= 0.5001 && t6 > 0) {
        solutions.push_back({{t6}, glm::vec3{0, 0, -1}});
    }
    if (solutions.empty()) {
        return {{}, {0, 0, 0}, false};
    } else {
        float min = get<0>(solutions[0]);
        glm::vec3 normal = get<1>(solutions[0]);
        for (int i = 1; i < solutions.size(); i++) {
            if (get<0>(solutions[i]) < min) {
                min = get<0>(solutions[i]);
                normal = get<1>(solutions[i]);
            }
        }
        return {{min}, normal, true};
    }
}

vector<float> RayTracer::quadraticEquation(float a, float b, float c) {
    float discriminant = powf(b, 2.f) - (4.f * a * c);
    vector<float> solutions;
    if (discriminant > 0) {
        float t1 = (-b + sqrt(powf(b, 2.f) - (4.f * a * c))) / (2.f * a);
        float t2 = (-b - sqrt(powf(b, 2.f) - (4.f * a * c))) / (2.f * a);
        if (t1 > 0) {
            solutions.push_back(t1);
        }
        if (t2 > 0) {
            solutions.push_back(t2);
        }
        return solutions;
    } else if (discriminant == 0) {
        float t = -b / (2.f * a);
        if (t > 0) {
            solutions.push_back(t);
        }
        return solutions;
    } else {
        return solutions;
    }
}
