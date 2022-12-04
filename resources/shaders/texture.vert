#version 330 core

// Add a second layout variable representing a UV coordinate
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 uvLayout;

// Create an "out" variable representing a UV coordinate
out vec3 uvCoords;

void main() {
    // Assign the UV layout variable to the UV "out" variable
    uvCoords = uvLayout;
    gl_Position = vec4(position, 1.0);
}
