#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1 < 1 ? 1 : param1;
    m_param2 = param2 < 3 ? 3 : param2;
    setVertexData();
}

void Cone::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    bool isFace,
                    bool isTip) {
    glm::vec3 TLNorm = glm::normalize(glm::vec3{topLeft.x, 0.5f * euclideanDistance(topLeft.x, topLeft.z), topLeft.z});
    glm::vec3 TRNorm = glm::normalize(glm::vec3{topRight.x, 0.5f * euclideanDistance(topRight.x, topRight.z), topRight.z});
    glm::vec3 BLNorm = glm::normalize(glm::vec3{bottomLeft.x, 0.5f * euclideanDistance(bottomLeft.x, bottomLeft.z), bottomLeft.z});
    glm::vec3 BRNorm = glm::normalize(glm::vec3{bottomRight.x, 0.5f * euclideanDistance(bottomRight.x, bottomRight.z), bottomRight.z});


    if (isFace) {
        TLNorm = {0, -1, 0};
        TRNorm = {0, -1, 0};
        BLNorm = {0, -1, 0};
        BRNorm = {0, -1, 0};
    } else if (isTip) {
        float x = (bottomLeft.x + bottomRight.x)/2.f;
        float z = (bottomLeft.z + bottomRight.z)/2.f;
        TLNorm = glm::normalize(glm::vec3{x, 0.5f * euclideanDistance(x, z), z});
        TRNorm = glm::normalize(glm::vec3{x, 0.5f * euclideanDistance(x, z), z});
    }

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, TLNorm);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, BLNorm);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, TRNorm);

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, TRNorm);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, BLNorm);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, BRNorm);
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    glm::vec3 NW;
    glm::vec3 NE;
    glm::vec3 SW;
    glm::vec3 SE;

    float phiIncrement = float(1.f / m_param1);
    float topPhi = 1.f;
    float bottomPhi = 1.f;

    for (int i = 0; i < m_param1; i++) {
        bottomPhi -= phiIncrement;

        // Make Body Tile
        NW = {0.5f * (1.f - topPhi) * glm::cos(nextTheta),
              topPhi - 0.5f,
              0.5f * (1.f - topPhi) * glm::sin(nextTheta)};
        NE = {0.5f * (1.f - topPhi) * glm::cos(currentTheta),
              topPhi - 0.5f,
              0.5f * (1.f - topPhi) * glm::sin(currentTheta)};
        SW = {0.5f * (1.f - bottomPhi) * glm::cos(nextTheta),
              bottomPhi - 0.5f,
              0.5f * (1.f - bottomPhi) * glm::sin(nextTheta)};
        SE = {0.5f * (1.f - bottomPhi) * glm::cos(currentTheta),
              bottomPhi - 0.5f,
              0.5f * (1.f - bottomPhi) * glm::sin(currentTheta)};
        if (topPhi == 1.f) {
            makeTile(NW, NE, SW, SE, false, true); // is Tip
        }
        makeTile(NW, NE, SW, SE, false, false);

        // Make Face
        makeFace(currentTheta, nextTheta, topPhi, bottomPhi);

        topPhi -= phiIncrement;
    }
}

void Cone::makeFace(float currentTheta, float nextTheta, float topPhi, float bottomPhi) {
    glm::vec3 NW = {0.5f * topPhi * glm::cos(nextTheta),
                    -0.5f,
                    0.5f * topPhi * glm::sin(nextTheta)};
    glm::vec3 NE = {0.5f * topPhi * glm::cos(currentTheta),
                    -0.5f,
                    0.5f * topPhi * glm::sin(currentTheta)};
    glm::vec3 SW = {0.5f * bottomPhi * glm::cos(nextTheta),
                    -0.5f,
                    0.5f * bottomPhi * glm::sin(nextTheta)};
    glm::vec3 SE = {0.5f * bottomPhi * glm::cos(currentTheta),
                    -0.5f,
                    0.5f * bottomPhi * glm::sin(currentTheta)};
    makeTile(NW, NE, SW, SE, true, false); // is Face
}

void Cone::makeCone() {
    //make body
    float thetaIncrement = glm::radians(360.f / m_param2);
    float currentTheta = 0.f;
    float nextTheta = 0.f;

    for (int i = 0; i < m_param2; i++) {
        nextTheta += thetaIncrement;
        makeWedge(currentTheta, nextTheta);
        currentTheta += thetaIncrement;
    }

}

void Cone::setVertexData() {
    makeCone();
}


// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

float Cone::euclideanDistance(float x, float z) {
    return sqrt(pow(x, 2) + pow(z, 2));
}
