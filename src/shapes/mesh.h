#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "utils/OBJ_Loader.h"

class Mesh
{
public:
    void init();
    std::vector<float> generateShape(objl::Mesh meshData);


private:
    void unpackMesh(objl::Vertex vertex);
    std::vector<float> m_vertexData;
};
