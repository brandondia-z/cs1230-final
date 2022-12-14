#pragma once

#include <vector>
#include <glm/glm.hpp>

class Cube
{
public:
    void updateParams(int param1, bool invert);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void insertVec3(std::vector<float> &data, glm::vec3 v);
    void insertVec2(std::vector<float> &data, glm::vec2 v);
    glm::vec2 getUV(glm::vec3 objPoint);
    void setVertexData(bool invert);
    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight);
    void makeWedge(float currTheta, float nextTheta);


    std::vector<float> m_vertexData;
    int m_param1;
};
