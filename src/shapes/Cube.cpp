#include "Cube.h"
#include <iostream>
#include <ostream>

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1 < 1 ? 1 : param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    // upper triangle
    glm::vec3 upperNormal = glm::normalize(glm::cross(topLeft - bottomLeft, bottomLeft - topRight));
    Cube::insertVec3(m_vertexData, topLeft);
    Cube::insertVec3(m_vertexData, upperNormal);
    Cube::insertVec3(m_vertexData, bottomLeft);
    Cube::insertVec3(m_vertexData, upperNormal);
    Cube::insertVec3(m_vertexData, topRight);
    Cube::insertVec3(m_vertexData, upperNormal);

    // lower triangle
    glm::vec3 lowerNormal = glm::normalize(glm::cross(topLeft - bottomLeft, bottomLeft - topRight));
    Cube::insertVec3(m_vertexData, topRight);
    Cube::insertVec3(m_vertexData, lowerNormal);
    Cube::insertVec3(m_vertexData, bottomLeft);
    Cube::insertVec3(m_vertexData, lowerNormal);
    Cube::insertVec3(m_vertexData, bottomRight);
    Cube::insertVec3(m_vertexData, lowerNormal);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

    float xDiff = bottomRight.x - topLeft.x;
    float yDiff = bottomRight.y - topLeft.y;
    float zDiff = bottomRight.z - topLeft.z;

    float xIncrement = float(xDiff)/float(m_param1);
    float yIncrement = float(yDiff)/float(m_param1);
    float zIncrement = float(zDiff)/float(m_param1);

    glm::vec3 NW;
    glm::vec3 NE;
    glm::vec3 SW;
    glm::vec3 SE;

    if (xDiff == 0) {
        for (int col = 0; col < m_param1; col++) {
            for (int row = 0; row < m_param1; row++) {
                NW = {topLeft.x,
                      topLeft.y + (row * yIncrement),
                      topLeft.z + ((col + 1) * zIncrement)};
                NE = {topLeft.x,
                      topLeft.y + (row * yIncrement),
                      topLeft.z + (col * zIncrement)};
                SW = {topLeft.x,
                      topLeft.y + ((row + 1) * yIncrement),
                      topLeft.z + ((col + 1) * zIncrement)};
                SE = {topLeft.x,
                      topLeft.y + ((row + 1) * yIncrement),
                      topLeft.z + (col * zIncrement)};
                Cube::makeTile(NW, NE, SW, SE);
            }
        }
    } else if (yDiff == 0) {
        for (int col = 0; col < m_param1; col++) {
            for (int row = 0; row < m_param1; row++) {
                NW = {topLeft.x + (col * xIncrement),
                      topLeft.y,
                      topLeft.z + (row * zIncrement)};
                NE = {topLeft.x + ((col + 1) * xIncrement),
                      topLeft.y,
                      topLeft.z + (row * zIncrement)};
                SW = {topLeft.x + (col * xIncrement),
                      topLeft.y,
                      topLeft.z + ((row + 1) * zIncrement)};
                SE = {topLeft.x + ((col + 1) * xIncrement),
                      topLeft.y,
                      topLeft.z + ((row + 1) * zIncrement)};
                Cube::makeTile(NW, NE, SW, SE);
            }
        }
    } else if (zDiff == 0) {
        for (int col = 0; col < m_param1; col++) {
            for (int row = 0; row < m_param1; row++) {
                NW = {topLeft.x + (col * xIncrement),
                      topLeft.y + (row * yIncrement),
                      topLeft.z};
                NE = {topLeft.x + ((col + 1) * xIncrement),
                      topLeft.y + (row * yIncrement),
                      topLeft.z};
                SW = {topLeft.x + (col * xIncrement),
                      topLeft.y + ((row + 1) * yIncrement),
                      topLeft.z};
                SE = {topLeft.x + ((col + 1) * xIncrement),
                      topLeft.y + ((row + 1) * yIncrement),
                      topLeft.z};
                Cube::makeTile(NW, NE, SW, SE);
            }
        }
    }
}

void Cube::setVertexData() {

     Cube::makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
              glm::vec3( 0.5f,  0.5f, 0.5f),
              glm::vec3(-0.5f, -0.5f, 0.5f),
              glm::vec3( 0.5f, -0.5f, 0.5f));

     Cube::makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
              glm::vec3( 0.5f, -0.5f,  0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f));

     Cube::makeFace(glm::vec3(0.5f, -0.5f,  0.5f),
              glm::vec3(0.5f,  0.5f,  0.5f),
              glm::vec3(0.5f, -0.5f, -0.5f),
              glm::vec3(0.5f,  0.5f, -0.5f));

     Cube::makeFace(glm::vec3( 0.5f, 0.5f,  0.5f),
              glm::vec3(-0.5f, 0.5f,  0.5f),
              glm::vec3( 0.5f, 0.5f, -0.5f),
              glm::vec3(-0.5f, 0.5f, -0.5f));

     Cube::makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
              glm::vec3(-0.5f, -0.5f,  0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));

     Cube::makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
              glm::vec3(-0.5f,  0.5f, -0.5f),
              glm::vec3( 0.5f, -0.5f, -0.5f),
              glm::vec3(-0.5f, -0.5f, -0.5f));
}

void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
