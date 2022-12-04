#version 330 core

// UV coordinate in variable
in vec3 uvCoords;

// Sampler2D uniform
uniform sampler2D sampler;
uniform sampler2D water_sampler;
uniform sampler2D displacement_sampler;

// Bool on whether or not to filter the texture
uniform bool perPixel;
uniform bool kernelBased;
uniform bool extraCredit1;
uniform bool extraCredit2;

uniform int width;
uniform int height;
uniform int water_time;
uniform int displacement_time;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1.f);
    // Set fragColor using the sampler2D at the UV coordinate
    vec4 displacement = texture(displacement_sampler,
                                 vec2(uvCoords.x+(displacement_time/1200.f), uvCoords.y-(displacement_time/1200.f)));
    fragColor = texture(water_sampler, vec2(uvCoords.x+displacement.g-(water_time/640.f), uvCoords.y+displacement.g+(water_time/640.f)));
//    fragColor = displacement;

    // Invert fragColor's r, g, and b color channels
    if (perPixel) {
        // INVERT
        fragColor = vec4(1.f - fragColor.x, 1.f - fragColor.y, 1.f - fragColor.z, 1);
    } else if (kernelBased) {
        // SHARPEN
        mat3 sharpen = mat3(-1.f, -1.f, -1.f,
                            -1.f, 17.f, -1.f,
                            -1.f, -1.f, -1.f);
        float redAcc = 0.f, greenAcc = 0.f, blueAcc = 0.f;
        for (int i = -1; i < 2; i++) {
            for (int j = -1; j < 2; j++) {
                vec4 adjColor = float(1.f/9.f) * sharpen[i+1][j+1] * texture(sampler, vec2(uvCoords.x + (i/float(width)), uvCoords.y + (j/float(height))));
                redAcc += adjColor.x;
                greenAcc += adjColor.y;
                blueAcc += adjColor.z;
            }
        }
        fragColor = vec4(redAcc, greenAcc, blueAcc, 1);
    } else if (extraCredit1) {
        // GRAYSCALE
        float intensity = (0.299 * fragColor.x) + (0.587 * fragColor.y) + (0.114 * fragColor.z);
        fragColor = vec4(intensity, intensity, intensity, 1);
    } else if (extraCredit2) {
        // GAUSSIAN BLUR
        float blur[25] = float[25]
                (1.f, 4.f,  6.f,  4.f,  1.f,
                 4.f, 16.f, 24.f, 16.f, 4.f,
                 6.f, 24.f, 36.f, 24.f, 6.f,
                 4.f, 16.f, 24.f, 16.f, 4.f,
                 1.f, 4.f,  6.f,  4.f,  1.f);
        float redAcc = 0.f, greenAcc = 0.f, blueAcc = 0.f;
        for (int i = -2; i < 3; i++) {
            for (int j = -2; j < 3; j++) {
                vec4 adjColor = float(1.f/256.f) * blur[(5*(i+2)) + (j+2)] * texture(sampler, vec2(uvCoords.x + (i/float(width)), uvCoords.y + (j/float(height))));
                redAcc += adjColor.x;
                greenAcc += adjColor.y;
                blueAcc += adjColor.z;
            }
        }
        fragColor = vec4(redAcc, greenAcc, blueAcc, 1);
    }
}
