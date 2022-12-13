#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1 < 1 ? 1 : param1;
    m_param2 = param2 < 3 ? 3 : param2;
    setVertexData();
}

void Cylinder::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    bool isBottomFace,
                    bool isTopFace) {
    glm::vec3 TLNorm = glm::normalize(glm::vec3{topLeft.x, 0, topLeft.z});
    glm::vec3 TRNorm = glm::normalize(glm::vec3{topRight.x, 0, topRight.z});
    glm::vec3 BLNorm = glm::normalize(glm::vec3{bottomLeft.x, 0, bottomLeft.z});
    glm::vec3 BRNorm = glm::normalize(glm::vec3{bottomRight.x, 0, bottomRight.z});
    if (isBottomFace) {
        TLNorm = {0, -1, 0};
        TRNorm = {0, -1, 0};
        BLNorm = {0, -1, 0};
        BRNorm = {0, -1, 0};
    } else if (isTopFace) {
        TLNorm = {0, 1, 0};
        TRNorm = {0, 1, 0};
        BLNorm = {0, 1, 0};
        BRNorm = {0, 1, 0};
    }
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, TLNorm);
    insertVec2(m_vertexData, getUV(topLeft));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, BLNorm);
    insertVec2(m_vertexData, getUV(bottomLeft));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, TRNorm);
    insertVec2(m_vertexData, getUV(topRight));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, TRNorm);
    insertVec2(m_vertexData, getUV(topRight));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, BLNorm);
    insertVec2(m_vertexData, getUV(bottomLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, BRNorm);
    insertVec2(m_vertexData, getUV(bottomRight));
}

void Cylinder::makeWedge(float currentTheta, float nextTheta) {
    glm::vec3 NW;
    glm::vec3 NE;
    glm::vec3 SW;
    glm::vec3 SE;

    float phiIncrement = float(1.f / m_param1);
    float topPhi = -0.5f;
    float bottomPhi = -0.5f;

    for (int i = 0; i < m_param1; i++) {
        bottomPhi += phiIncrement;

        // Make Body Tile
        NW = {0.5f * glm::cos(currentTheta), topPhi, 0.5f * glm::sin(currentTheta)};
        NE = {0.5f * glm::cos(nextTheta), topPhi, 0.5f * glm::sin(nextTheta)};
        SW = {0.5f * glm::cos(currentTheta), bottomPhi, 0.5f * glm::sin(currentTheta)};
        SE = {0.5f * glm::cos(nextTheta), bottomPhi, 0.5f * glm::sin(nextTheta)};
        makeTile(NW, NE, SW, SE, false, false);

        // Make Faces
        makeFaces(currentTheta, nextTheta, topPhi, bottomPhi);

        topPhi += phiIncrement;
    }
}

void Cylinder::makeFaces(float currentTheta, float nextTheta, float topPhi, float bottomPhi) {

    glm::vec3 NW;
    glm::vec3 NE;
    glm::vec3 SW;
    glm::vec3 SE;

    // Top Face
    NW = {0.5f * (0.5f - topPhi) * glm::cos(currentTheta), 0.5f, 0.5f * (0.5f - topPhi) * glm::sin(currentTheta)};
    NE = {0.5f * (0.5f - topPhi) * glm::cos(nextTheta), 0.5f, 0.5f * (0.5f - topPhi) * glm::sin(nextTheta)};
    SW = {0.5f * (0.5f - bottomPhi) * glm::cos(currentTheta), 0.5f, 0.5f * (0.5f - bottomPhi) * glm::sin(currentTheta)};
    SE = {0.5f * (0.5f - bottomPhi) * glm::cos(nextTheta), 0.5f, 0.5f * (0.5f - bottomPhi) * glm::sin(nextTheta)};
    makeTile(NW, NE, SW, SE, false, true);

    // Bottom Face
    NW = {0.5f * (0.5f - topPhi) * glm::cos(currentTheta), -0.5f, 0.5f * (0.5f - topPhi) * glm::sin(currentTheta)};
    NE = {0.5f * (0.5f - topPhi) * glm::cos(nextTheta), -0.5f, 0.5f * (0.5f - topPhi) * glm::sin(nextTheta)};
    SW = {0.5f * (0.5f - bottomPhi) * glm::cos(currentTheta), -0.5f, 0.5f * (0.5f - bottomPhi) * glm::sin(currentTheta)};
    SE = {0.5f * (0.5f - bottomPhi) * glm::cos(nextTheta), -0.5f, 0.5f * (0.5f - bottomPhi) * glm::sin(nextTheta)};
    makeTile(NE, NW, SE, SW, true, false); // Flip W & E to make counter clockwise

}

void Cylinder::makeCylinder() {
    //make body
    float thetaIncrement = glm::radians(360.f / m_param2);
    float currentTheta = 0;
    float nextTheta = 0;

    for (int i = 0; i < m_param2; i++) {
        nextTheta += thetaIncrement;
        makeWedge(currentTheta, nextTheta);
        currentTheta += thetaIncrement;
    }

}

void Cylinder::setVertexData() {
    // TODO for Project 5: Lights, Camera
     makeCylinder();

}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}

glm::vec2 Cylinder::getUV(glm::vec3 objPoint) {
    float u;
    float v;
    float theta;
    if (objPoint.y == 0.5f) {
        v = -(objPoint.z + 0.5f);
        u = (objPoint.x  + 0.5f);
    } else if (objPoint.y == -0.5f) {
        u = objPoint.x + 0.5f;
        v = objPoint.z + 0.5f;
    } else {
        theta = atan2(objPoint.z, objPoint.x);
        u = (theta  < 0) ? (-theta/(2*M_PI)) : (1 - (theta/(2*M_PI)));
        v = objPoint.y + 0.5f;
    }

    return glm::vec2(u, v);
}
