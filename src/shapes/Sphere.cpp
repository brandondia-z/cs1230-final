#include "Sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1 < 2 ? 2 : param1;
    m_param2 = param2 < 3 ? 3 : param2;
    Sphere::setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    glm::vec3 TLNorm = glm::normalize(topLeft);
    glm::vec3 BLNorm = glm::normalize(bottomLeft);
    glm::vec3 TRNorm = glm::normalize(topRight);
    glm::vec3 BRNorm = glm::normalize(bottomRight);

    Sphere::insertVec3(m_vertexData, topLeft);
    Sphere::insertVec3(m_vertexData, TLNorm);
    Sphere::insertVec3(m_vertexData, bottomLeft);
    Sphere::insertVec3(m_vertexData, BLNorm);
    Sphere::insertVec3(m_vertexData, topRight);
    Sphere::insertVec3(m_vertexData, TRNorm);

    Sphere::insertVec3(m_vertexData, topRight);
    Sphere::insertVec3(m_vertexData, TRNorm);
    Sphere::insertVec3(m_vertexData, bottomLeft);
    Sphere::insertVec3(m_vertexData, BLNorm);
    Sphere::insertVec3(m_vertexData, bottomRight);
    Sphere::insertVec3(m_vertexData, BRNorm);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {

    glm::vec3 NW;
    glm::vec3 NE;
    glm::vec3 SW;
    glm::vec3 SE;

    float phiIncrement = glm::radians(180.f / m_param1);
    float topPhi = 0.f;
    float bottomPhi = 0.f;

    for (int i = 0; i < m_param1; i++) {
        bottomPhi += phiIncrement;
        NW = {glm::sin(topPhi) * glm::sin(currentTheta),
              glm::cos(topPhi),
              glm::sin(topPhi) * glm::cos(currentTheta)};
        NW = 0.5f * NW;
        NE = {glm::sin(topPhi) * glm::sin(nextTheta),
              glm::cos(topPhi),
              glm::sin(topPhi) * glm::cos(nextTheta)};
        NE = 0.5f * NE;
        SW = {glm::sin(bottomPhi) * glm::sin(currentTheta),
              glm::cos(bottomPhi),
              glm::sin(bottomPhi) * glm::cos(currentTheta)};
        SW = 0.5f * SW;
        SE = {glm::sin(bottomPhi) * glm::sin(nextTheta),
              glm::cos(bottomPhi),
              glm::sin(bottomPhi) * glm::cos(nextTheta)};
        SE = 0.5f * SE;
        Sphere::makeTile(NW, NE, SW, SE);
        topPhi += phiIncrement;
    }

}

void Sphere::makeSphere() {

    float thetaIncrement = glm::radians(360.f / m_param2);
    float currTheta = 0;
    float nextTheta = 0;

    for (int j = 0; j < m_param2; j++) {
        nextTheta += thetaIncrement;
        Sphere::makeWedge(currTheta, nextTheta);
        currTheta += thetaIncrement;
    }
}

void Sphere::setVertexData() {
     Sphere::makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
