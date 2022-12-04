#include "mesh.h"
#include <iostream>
#include <ostream>

void Mesh::init() {
    m_vertexData = std::vector<float>();
}

void Mesh::unpackMesh(objl::Vertex vertex) {
    m_vertexData.push_back(vertex.Position.X);
    m_vertexData.push_back(vertex.Position.Y);
    m_vertexData.push_back(vertex.Position.Z);
    m_vertexData.push_back(vertex.Normal.X);
    m_vertexData.push_back(vertex.Normal.Y);
    m_vertexData.push_back(vertex.Normal.Z);
}

std::vector<float> Mesh::generateShape(objl::Mesh meshData) {
    int size = meshData.Vertices.size();
    for (int i = 0; size; i++) {
        if (i >= size) {
            break;
        }
        unpackMesh(meshData.Vertices[i]);
    }
    return m_vertexData;
}
