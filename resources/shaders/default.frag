#version 330 core

// In variables for the world-space position and normal
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uvCoords;

// Output color
out vec4 fragColor;

// textures
uniform sampler2D water_sampler;
uniform sampler2D displacement_sampler;
uniform sampler2D stone_sampler;
uniform sampler2D marble_sampler;
uniform sampler2D diamond_sampler;

// scrolling
uniform int water_time;
uniform int displacement_time;

// Ambient lighting
uniform float ambient;
uniform vec4 materialAmbient;

uniform int shapeType;

// Diffuse lighting
struct LightingData
{
  int type;
  vec4 color;
  vec4 lightDir;
  vec4 lightPos;
  vec3 attenuation;
  float angle;
  float penumbra;
};

uniform LightingData lighting[8];
uniform int numLights;
uniform float diffuse;
uniform vec4 materialDiffuse;

// Specular lighting
uniform float specular;
uniform float shininess;
uniform float blend;
uniform vec4 materialSpecular;

uniform vec4 cameraPos;

void main() {

    if (shapeType == 6) { // Plane
        vec4 displacement = texture(displacement_sampler,vec2(uvCoords.x+(displacement_time/1200.f), uvCoords.y-(displacement_time/1200.f)));
        fragColor = texture(water_sampler,vec2(uvCoords.x+displacement.g-(water_time/640.f), uvCoords.y+displacement.g+(water_time/640.f)));
        fragColor.a = 0.5f;
    } else if (shapeType == 0) { // Cube
        fragColor = texture(stone_sampler, vec2(uvCoords.x, uvCoords.y));
    
    } else if (shapeType == 2) {
        fragColor = texture(marble_sampler, vec2(uvCoords.x, uvCoords.y));
    } else if (shapeType == 1) {
        fragColor = texture(diamond_sampler, vec2(uvCoords.x, uvCoords.y));
    }
    if (shapeType != 6) { // not Plane and not Cone
        // Add ambient component to output
        float red = ambient * materialAmbient.x;
        float green = ambient * materialAmbient.y;
        float blue = ambient * materialAmbient.z;

        // variable declarations
        float distance, atten, diffuseDotProd, specularDotProd, shininessCalc, interpolationR, interpolationG, interpolationB;
        vec3 directionToLight, reflectedLight, textureColor;

        for (int i = 0; i < numLights; i++) {
            vec3 directionToCamera = normalize(vec3(cameraPos) - worldPosition);

            switch(lighting[i].type) {
                case 0 : // Point Lighting
                    distance = sqrt(pow(lighting[i].lightPos.x - worldPosition.x, 2.f) +
                                          pow(lighting[i].lightPos.y - worldPosition.y, 2.f) +
                                          pow(lighting[i].lightPos.z - worldPosition.z, 2.f));
                    atten = min(1.f, float(1.f /(lighting[i].attenuation.x + (distance * lighting[i].attenuation.y) +
                                                       (pow(distance, 2.f) * lighting[i].attenuation.z))));

                    // diffuse componenet
                    directionToLight = normalize(vec3(lighting[i].lightPos) - worldPosition);
                    diffuseDotProd = min(1.f, max(0.f, dot(normalize(worldNormal), directionToLight)));

                    interpolationR = (blend * fragColor.r) + ((1.f - blend) * (diffuse * materialDiffuse.x));
                    interpolationG = (blend * fragColor.g) + ((1.f - blend) * (diffuse * materialDiffuse.y));
                    interpolationB = (blend * fragColor.b) + ((1.f - blend) * (diffuse * materialDiffuse.z));

                    red += (atten * lighting[i].color.x * interpolationR * diffuseDotProd);
                    green += (atten * lighting[i].color.y * interpolationG * diffuseDotProd);
                    blue += (atten * lighting[i].color.z * interpolationB * diffuseDotProd);

                    // specular component
                    reflectedLight = normalize(reflect(-directionToLight, normalize(worldNormal)));
                    specularDotProd = min(1.f, max(0.f, dot(reflectedLight, directionToCamera)));
                    shininessCalc = shininess <= 0.f ? 1.f : pow(specularDotProd, shininess);

                    red += (atten * lighting[i].color.x * specular * materialSpecular.x * shininessCalc);
                    green += (atten * lighting[i].color.y * specular * materialSpecular.y * shininessCalc);
                    blue += (atten * lighting[i].color.z * specular * materialSpecular.z * shininessCalc);
                    break;

                case 1 : // Directional Lighting
                    // diffuse component
                    diffuseDotProd = min(1.f, max(0.f, dot(normalize(worldNormal), vec3(normalize(-lighting[i].lightDir)))));

                    interpolationR = (blend * fragColor.r) + ((1.f - blend) * (diffuse * materialDiffuse.r));
                    interpolationG = (blend * fragColor.g) + ((1.f - blend) * (diffuse * materialDiffuse.g));
                    interpolationB = (blend * fragColor.b) + ((1.f - blend) * (diffuse * materialDiffuse.b));

                    red += (lighting[i].color.x * interpolationR * diffuseDotProd);
                    green += (lighting[i].color.y * interpolationG * diffuseDotProd);
                    blue += (lighting[i].color.z * interpolationB * diffuseDotProd);

                    // specular component
                    reflectedLight = normalize(reflect(normalize(vec3(lighting[i].lightDir)), normalize(worldNormal)));
                    specularDotProd = min(1.f, max(0.f, dot(reflectedLight, directionToCamera)));
                    shininessCalc = shininess <= 0.f ? 1.f : pow(specularDotProd, shininess);

                    red += (lighting[i].color.x * specular * materialSpecular.x * shininessCalc);
                    green += (lighting[i].color.y * specular * materialSpecular.y * shininessCalc);
                    blue += (lighting[i].color.z * specular * materialSpecular.z * shininessCalc);
                    break;

                case 2: // Spot Lighting
                    distance = sqrt(pow(lighting[i].lightPos.x - worldPosition.x, 2.f) +
                                          pow(lighting[i].lightPos.y - worldPosition.y, 2.f) +
                                          pow(lighting[i].lightPos.z - worldPosition.z, 2.f));
                    atten = min(1.f, float(1.f / (lighting[i].attenuation.x + (distance * lighting[i].attenuation.y) +
                                                        (pow(distance, 2.f) * lighting[i].attenuation.z))));

                    // diffuse component
                    directionToLight = normalize(vec3(lighting[i].lightPos) - worldPosition);
                    diffuseDotProd = min(1, max(0.f, dot(normalize(worldNormal), directionToLight)));

                    float outerTheta = lighting[i].angle;
                    float innerTheta = lighting[i].angle - lighting[i].penumbra;
                    float currAngle = acos(dot(vec3(normalize(lighting[i].lightDir)), vec3(normalize(vec4(worldPosition, 1.f) - lighting[i].lightPos))));

                    float redColor, greenColor, blueColor;
                    if (currAngle <= innerTheta) {
                        redColor = lighting[i].color.x;
                        greenColor = lighting[i].color.y;
                        blueColor = lighting[i].color.z;
                    } else if (innerTheta < currAngle && currAngle <= outerTheta) {
                        float falloff = -2.f * pow( (currAngle - innerTheta) / (outerTheta - innerTheta), 3.f) +
                                3.f * pow( (currAngle - innerTheta) / (outerTheta - innerTheta), 2.f);
                        redColor = lighting[i].color.x * (1.f - falloff);
                        greenColor = lighting[i].color.y * (1.f - falloff);
                        blueColor = lighting[i].color.z * (1.f - falloff);
                    } else {
                        break;
                    }

                    interpolationR = (blend * fragColor.r) + ((1.f - blend) * (diffuse * materialDiffuse.x));
                    interpolationG = (blend * fragColor.g) + ((1.f - blend) * (diffuse * materialDiffuse.y));
                    interpolationB = (blend * fragColor.b) + ((1.f - blend) * (diffuse * materialDiffuse.z));

                    red += (atten * redColor * interpolationR * diffuseDotProd);
                    green += (atten * greenColor * interpolationG * diffuseDotProd);
                    blue += (atten * blueColor * interpolationB * diffuseDotProd);

                    // specular component
                    reflectedLight = normalize(reflect(-directionToLight, normalize(worldNormal)));
                    specularDotProd = min(1.f, max(0.f, dot(reflectedLight, directionToCamera)));
                    shininessCalc = shininess <= 0.f ? 1.f : pow(specularDotProd, shininess);

                    red += (atten * redColor * specular * materialSpecular.x * shininessCalc);
                    green += (atten * greenColor * specular * materialSpecular.y * shininessCalc);
                    blue += (atten * blueColor * specular * materialSpecular.z * shininessCalc);
                    break;

                default : // Error, should never be hit
                    break;
            }

        }
        fragColor = vec4(red, green, blue, 1.f);
        if (shapeType == 7) {

            fragColor = vec4(143.f/256.f, 219.f/256.f, 242.f/256.f, 0.18f);
        }

    }
}
