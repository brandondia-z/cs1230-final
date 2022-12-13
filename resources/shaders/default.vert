#version 330 core

// Object-space position
layout(location = 0) in vec3 objectPosition;
layout(location = 1) in vec3 objectNormal;
layout(location = 2) in vec2 layoutUvCoords;

// World-space position and normal,
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 uvCoords;

uniform sampler2D height_sampler;
uniform int height_time;

// Model matrix (CTM)
uniform mat4 modelMat;
uniform mat4 rotationMat;

// View and projection matrix
uniform mat4 viewMat;
uniform mat4 projMat;
uniform mat3 itModelMat;
uniform bool isMesh;
uniform int shapeType;

void main() {
    uvCoords = layoutUvCoords;
    if (shapeType == 6) { // Plane
        float disp;
        disp = texture(height_sampler, layoutUvCoords + (height_time/15000.f)).r;
        vec4 displace = vec4(objectPosition, 1);
        float displaceFactor = 0.1;
        float displaceBias = 0.0;
        displace.xyz += (displaceFactor * disp - displaceBias) * objectNormal;
        displace = modelMat * displace;
        gl_Position = (projMat * (viewMat * displace));

        vec4 wp = modelMat * vec4(objectPosition, 1.0);
        worldPosition = vec3(wp);
        worldNormal = itModelMat * normalize(objectNormal);
    } else if (shapeType == 1) { // Cone
        vec4 rotatedPosition = rotationMat * vec4(objectPosition, 1);
        vec4 rotatedNormal = rotationMat * vec4(normalize(objectNormal), 1);
        worldPosition = vec3(modelMat * rotatedPosition);
        worldNormal = itModelMat * vec3(normalize(rotatedNormal)); // also transposed
        gl_Position = (projMat * (viewMat * vec4(worldPosition, 1)));
    }
    else {
        worldPosition = vec3(modelMat * vec4(objectPosition, 1));
        worldNormal = itModelMat * vec3(normalize(objectNormal)); // also transposed

        // Position transformed to clip space
        gl_Position = (projMat * (viewMat * vec4(worldPosition, 1)));
    }
}
