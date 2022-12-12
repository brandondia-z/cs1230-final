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
uniform vec4 materialSpecular;

uniform vec4 cameraPos;

//void main() {
//    fragColor = vec4(1.f);
////     Set fragColor using the sampler2D at the UV coordinate
//    vec4 displacement = texture(displacement_sampler,
//                                 vec2(uvCoords.x+(displacement_time/1200.f), uvCoords.y-(displacement_time/1200.f)));
//    fragColor = texture(water_sampler, vec2(uvCoords.x+displacement.g-(water_time/640.f), uvCoords.y+displacement.g+(water_time/640.f)));
//}

void main() {

    if (shapeType == 6) {
        vec4 displacement = texture(displacement_sampler,vec2(uvCoords.x+(displacement_time/1200.f), uvCoords.y-(displacement_time/1200.f)));
        fragColor = texture(water_sampler,vec2(uvCoords.x+displacement.g-(water_time/640.f), uvCoords.y+displacement.g+(water_time/640.f)));
    } else {
        // Add ambient component to output
        float red = ambient * materialAmbient.x;
        float green = ambient * materialAmbient.y;
        float blue = ambient * materialAmbient.z;

        // variable declarations
        float distance, atten, diffuseDotProd, specularDotProd, shininessCalc;
        vec3 directionToLight, reflectedLight;

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

                    red += (atten * lighting[i].color.x * diffuse * materialDiffuse.x * diffuseDotProd);
                    green += (atten * lighting[i].color.y * diffuse * materialDiffuse.y * diffuseDotProd);
                    blue += (atten * lighting[i].color.z * diffuse * materialDiffuse.z * diffuseDotProd);

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

                    red += (lighting[i].color.x * diffuse * materialDiffuse.x * diffuseDotProd);
                    green += (lighting[i].color.y * diffuse * materialDiffuse.y * diffuseDotProd);
                    blue += (lighting[i].color.z * diffuse * materialDiffuse.z * diffuseDotProd);

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

                    red += (atten * redColor * diffuse * materialDiffuse.x * diffuseDotProd);
                    green += (atten * greenColor * diffuse * materialDiffuse.y * diffuseDotProd);
                    blue += (atten * blueColor * diffuse * materialDiffuse.z * diffuseDotProd);

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

    }
}
