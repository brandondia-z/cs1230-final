#include "Plane.h"
#include <iostream>
#include <ostream>

void Plane::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1 < 1 ? 1 : param1;
    setVertexData();
}

void Plane::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {

//    // upper triangle
//    glm::vec3 upperNormal = glm::normalize(glm::cross(topLeft - bottomLeft, bottomLeft - topRight));
//    Cube::insertVec3(m_vertexData, topLeft);
//    Cube::insertVec3(m_vertexData, upperNormal);
//    Cube::insertVec3(m_vertexData, bottomLeft);
//    Cube::insertVec3(m_vertexData, upperNormal);
//    Cube::insertVec3(m_vertexData, topRight);
//    Cube::insertVec3(m_vertexData, upperNormal);

//    // lower triangle
//    glm::vec3 lowerNormal = glm::normalize(glm::cross(topLeft - bottomLeft, bottomLeft - topRight));
//    Cube::insertVec3(m_vertexData, topRight);
//    Cube::insertVec3(m_vertexData, lowerNormal);
//    Cube::insertVec3(m_vertexData, bottomLeft);
//    Cube::insertVec3(m_vertexData, lowerNormal);
//    Cube::insertVec3(m_vertexData, bottomRight);
//    Cube::insertVec3(m_vertexData, lowerNormal);

    glm::vec3 edge1 = topLeft - bottomLeft;
    glm::vec3 edge2 = bottomLeft - topRight;
    glm::vec3 upperNormal = glm::normalize(glm::cross(edge1, edge2));
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, upperNormal);
    insertVec2(m_vertexData, getUV(topLeft));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, upperNormal);
    insertVec2(m_vertexData, getUV(bottomLeft));

    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, upperNormal);
    insertVec2(m_vertexData, getUV(topRight));

    glm::vec3 edge3 = topLeft - bottomLeft;
    glm::vec3 edge4 = bottomLeft - topRight;
    glm::vec3 lowerNormal = glm::normalize(glm::cross(edge3, edge4));
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, lowerNormal);
    insertVec2(m_vertexData, getUV(topRight));

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, lowerNormal);
    insertVec2(m_vertexData, getUV(bottomLeft));

    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, lowerNormal);
    insertVec2(m_vertexData, getUV(bottomRight));
}

void Plane::makeFace(glm::vec3 topLeft,
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
                Plane::makeTile(NW, NE, SW, SE);
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
                Plane::makeTile(NW, NE, SW, SE);
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
                Plane::makeTile(NW, NE, SW, SE);
            }
        }
    }
}

void Plane::setVertexData() {

//     Cube::makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
//              glm::vec3( 0.5f,  0.5f, 0.5f),
//              glm::vec3(-0.5f, -0.5f, 0.5f),
//              glm::vec3( 0.5f, -0.5f, 0.5f));

//     Cube::makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),
//              glm::vec3( 0.5f, -0.5f,  0.5f),
//              glm::vec3(-0.5f, -0.5f, -0.5f),
//              glm::vec3( 0.5f, -0.5f, -0.5f));

//     Cube::makeFace(glm::vec3(0.5f, -0.5f,  0.5f),
//              glm::vec3(0.5f,  0.5f,  0.5f),
//              glm::vec3(0.5f, -0.5f, -0.5f),
//              glm::vec3(0.5f,  0.5f, -0.5f));

     Plane::makeFace(glm::vec3( 0.5f, 0.5f,  0.5f),
              glm::vec3(-0.5f, 0.5f,  0.5f),
              glm::vec3( 0.5f, 0.5f, -0.5f),
              glm::vec3(-0.5f, 0.5f, -0.5f));

//     Cube::makeFace(glm::vec3(-0.5f,  0.5f,  0.5f),
//              glm::vec3(-0.5f, -0.5f,  0.5f),
//              glm::vec3(-0.5f,  0.5f, -0.5f),
//              glm::vec3(-0.5f, -0.5f, -0.5f));

//     Cube::makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),
//              glm::vec3(-0.5f,  0.5f, -0.5f),
//              glm::vec3( 0.5f, -0.5f, -0.5f),
//              glm::vec3(-0.5f, -0.5f, -0.5f));
}

void Plane::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

glm::vec2 Plane::getUV(glm::vec3 objPoint) {
    float u;
    float v;
    if (objPoint.x ==0.5f) {
        u = -(objPoint.z  + 0.5f);
        v = objPoint.y  + 0.5f;
        // bottom left
    } else if (objPoint.y== 0.5f) {
        v = -(objPoint.z + 0.5f);
        u = (objPoint.x  + 0.5f);
        // middle bottom
    }  else if (objPoint.z == 0.5f) {
        u = objPoint.x  + 0.5f;
        v = objPoint.y  + 0.5f;
        // top left
    }  else if (objPoint.x == -0.5f) {
        v = objPoint.y  + 0.5f;
        u = objPoint.z  + 0.5f;
        // middle top
    } else if (objPoint.y == -0.5f) {
        u = objPoint.x + 0.5f;
        v = objPoint.z + 0.5f;
        // bottom right
    } else if (objPoint.z == -0.5f) {
        v = (objPoint.y + 0.5f);
        u = -(objPoint.x + 0.5f);
        // top right
    }
    return glm::vec2(u, v);
}

void Plane::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
