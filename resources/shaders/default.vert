#version 330 core

// Object-space position
layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec3 objectNormal;

// World-space position and normal,
out vec3 worldPosition;
out vec3 worldNormal;

// Model matrix (CTM)
uniform mat4 modelMat;

// View and projection matrix
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 itModelMat;
uniform bool isMesh;

void main() {

    if (isMesh) {
        worldPosition = objectPosition;
        worldNormal = normalize(objectNormal);
    } else {
        worldPosition = vec3(modelMat * vec4(objectPosition, 1));
        worldNormal = itModelMat * vec3(normalize(objectNormal)); // also transposed
    }

    // Position transformed to clip space
    gl_Position = (projMat * (viewMat * vec4(worldPosition, 1)));
}
