#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "debug.h"
#include "glm/gtx/transform.hpp"
#include "raytracer/raytracer.h"
#include "raytracer/raytracescene.h"
#include "settings.h"
#include "shaderloader.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"
#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"
#include "shapes/mesh.h"

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &m_coneVbo);
    glDeleteBuffers(1, &m_cubeVbo);
    glDeleteBuffers(1, &m_cylinderVbo);
    glDeleteBuffers(1, &m_sphereVbo);
    glDeleteBuffers(1, &m_meshVbo);
    glDeleteBuffers(1, &m_planeVbo);
    glDeleteBuffers(1, &m_invertCubeVbo);

    glDeleteVertexArrays(1, &m_coneVao);
    glDeleteVertexArrays(1, &m_cubeVao);
    glDeleteVertexArrays(1, &m_cylinderVao);
    glDeleteVertexArrays(1, &m_sphereVao);
    glDeleteVertexArrays(1, &m_meshVao);
    glDeleteVertexArrays(1, &m_planeVao);
    glDeleteVertexArrays(1, &m_invertCubeVao);

    glDeleteTextures(1, &m_water_texture);
    glDeleteTextures(1, &m_displacement_texture);
    glDeleteTextures(1, &m_height_texture);

    glDeleteProgram(m_lighting_shader);

    glDeleteProgram(m_postprocess_shader);
    glDeleteProgram(m_pixeloutline_shader);

    glDeleteVertexArrays(1, &m_fullscreen_vao);
    glDeleteBuffers(1, &m_fullscreen_vbo);

    glDeleteTextures(1, &default_texture);
    glDeleteRenderbuffers(1, &default_rb);
    glDeleteFramebuffers(1, &renderFBO);

    glDeleteTextures(2, pingpongBuffer);
    glDeleteFramebuffers(2, pingpongFBO);

    this->doneCurrent();
}

void Realtime::generateFullQuadData(){
    std::vector<GLfloat> fullscreen_quad_data =
          { //     POSITIONS    //
              -1.f,  1.f, 0.0f,
               0.f, 1.f, //uv
              -1.f, -1.f, 0.0f,
               0.f, 0.f, //uv
               1.f, -1.f, 0.0f,
               1.f, 0.f, //uv
               1.f,  1.f, 0.0f,
               1.f, 1.f, //uv
              -1.f,  1.f, 0.0f,
               0.f, 1.f, //uv
               1.f, -1.f, 0.0f,
               1.f, 0.f //uv
          };

        // generate and bind vbo/vao for fullscreen quad
        glGenBuffers(1, &m_fullscreen_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
        glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
        glGenVertexArrays(1, &m_fullscreen_vao);
        glBindVertexArray(m_fullscreen_vao);

        // add attributes to vao
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(0*sizeof(GLfloat)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(3*sizeof(GLfloat)));

        // unbind vbo and vao
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
}

void Realtime::initializePixelation(){
        //pass in uniforms
        glUseProgram(m_postprocess_shader);
        // keep it at 3 to make sure texture binds at right spot!!!
        glUniform1i(glGetUniformLocation(m_postprocess_shader, "m_texture"), 3);
        glUniform1i(glGetUniformLocation(m_postprocess_shader, "image_width"), size().width());
        glUniform1i(glGetUniformLocation(m_postprocess_shader, "image_height"), size().height());

        //pass in uniforms
        glUseProgram(m_pixeloutline_shader);
        glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "m_texture"), 4);
        glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "image_width"), size().width());
        glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "image_height"), size().height());

        //create fullscreen quad data
        generateFullQuadData();

        //create first renderFBO
        glGenFramebuffers(1, &renderFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

        // create default texture
        glGenTextures(1, &default_texture);
        /// ACTIVATES SLOT 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, default_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, default_texture, 0);

        // tell opengl to draw to attachment 0
        GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);

       //Generate and bind a renderbuffer of the right size, set its format, then unbind
        glGenRenderbuffers(1, &default_rb);
        glBindRenderbuffer(GL_RENDERBUFFER, default_rb);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, default_rb);

        // create pingpong

        glGenFramebuffers(2, pingpongFBO);
        glGenTextures(2, pingpongBuffer);
        int textureSlot = 4;
        for (unsigned int i=0; i<2; i++){
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

            // binds at correct texture slot
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
            //glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "m_texture"), textureSlot);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                std::cout << "failed" << std::endl;
            }
            textureSlot++;
        }
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();
    m_water_time = 0;
    m_displacement_time = 0;

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Set the clear color here
    glClearColor(0, 0, 0, 0);

    // Shader setup and variable setting
    m_lighting_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
    m_postprocess_shader = ShaderLoader::createShaderProgram(":/resources/shaders/pixelation.vert", ":/resources/shaders/pixelation.frag");
    m_pixeloutline_shader = ShaderLoader::createShaderProgram(":/resources/shaders/pixelation.vert", ":/resources/shaders/pixeloutline.frag");


    initializePixelation();
    m_currParam1 = settings.shapeParameter1;
    m_currParam2 = settings.shapeParameter2;
    m_nearPlane = settings.nearPlane;
    m_farPlane = settings.farPlane;

    glGenVertexArrays(1, &m_coneVao); // Generates VAOs and stores them in m_vaos
    glGenVertexArrays(1, &m_cubeVao);
    glGenVertexArrays(1, &m_cylinderVao);
    glGenVertexArrays(1, &m_sphereVao);
    glGenVertexArrays(1, &m_meshVao);
    glGenVertexArrays(1, &m_planeVao);
    glGenVertexArrays(1, &m_invertCubeVao);

    glGenBuffers(1, &m_coneVbo); // Generates VBOs and stores them in m_vbos
    glGenBuffers(1, &m_cubeVbo);
    glGenBuffers(1, &m_cylinderVbo);
    glGenBuffers(1, &m_sphereVbo);
    glGenBuffers(1, &m_meshVbo);
    glGenBuffers(1, &m_planeVbo);
    glGenBuffers(1, &m_invertCubeVbo);

    glBindVertexArray(m_coneVao);
    bindCone();
    glBindVertexArray(0);

    glBindVertexArray(m_cubeVao);
    bindCube();
    glBindVertexArray(0);

    glBindVertexArray(m_invertCubeVao);
    bindInvertCube();
    glBindVertexArray(0);

    glBindVertexArray(m_planeVao);
    bindPlane();
    glBindVertexArray(0);

    glBindVertexArray(m_cylinderVao);
    bindCylinder();
    glBindVertexArray(0);

    glBindVertexArray(m_sphereVao);
    bindSphere();
    glBindVertexArray(0);

    /// WATER AND DISPLACEMENT
    QString water_filepath = QString(":/resources/images/water_texture.png"); // Prepare filepath
    m_water_image = QImage(water_filepath); // Obtain image from filepath
    m_water_image = m_water_image.convertToFormat(QImage::Format_RGBA8888).mirrored(); // Format image to fit OpenGL
    glGenTextures(1, &m_water_texture); // Generate water texture

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_water_texture); // Bind water texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_water_image.width(), m_water_image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, m_water_image.bits()); // Load image into water texture
    // Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind water texture

    QString displacement_filepath = QString(":/resources/images/displacement_texture.png");
    m_displacement_image = QImage(displacement_filepath);
    m_displacement_image = m_displacement_image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    glGenTextures(1, &m_displacement_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_displacement_texture); // Bind displacement texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_displacement_image.width(), m_displacement_image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, m_displacement_image.bits()); // Load image into displacement texture
    // Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind displacement texture

    glUseProgram(m_lighting_shader);
    GLint waterTextureLocation = glGetUniformLocation(m_lighting_shader, "water_sampler");
    glUniform1i(waterTextureLocation, 0);
    GLint displacementTextureLocation = glGetUniformLocation(m_lighting_shader, "displacement_sampler");
    glUniform1i(displacementTextureLocation, 1);

    /// HEIGHT MAPPING
    QString height_filepath = QString(":/resources/images/heightNoise2.png"); // Prepare filepath
    QImage height_image = QImage(height_filepath); // Obtain image from filepath
    height_image = height_image.convertToFormat(QImage::Format_RGBA8888).mirrored(); // Format image to fit OpenGL

    glGenTextures(1, &m_height_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_height_texture); // Bind texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, height_image.width(), height_image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, height_image.bits()); // Load image into texture
    // Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    GLuint loc = glGetUniformLocation(m_lighting_shader, "height_sampler");
    glUniform1i(loc, 2);

    m_setupComplete = true;
}

void Realtime::paintGL() {
    // activate main FBO
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_screen_width, m_screen_height);

    // Activate the shader program by calling glUseProgram with `m_lighint_shader`
    glUseProgram(m_lighting_shader);

    // Time variables to help create scrolling textures
    GLint waterTimeLocation = glGetUniformLocation(m_lighting_shader, "water_time");
    glUniform1i(waterTimeLocation, m_water_time);
    GLint dispTimeLocation = glGetUniformLocation(m_lighting_shader, "displacement_time");
    glUniform1i(dispTimeLocation, m_displacement_time);
    GLint heightTimeLocation = glGetUniformLocation(m_lighting_shader, "height_time");
    glUniform1i(heightTimeLocation, m_displacement_time);

    // Pass in m_viewMatrix and m_projMatrix
    GLint viewLocation = glGetUniformLocation(m_lighting_shader, "viewMat");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &m_viewMatrix[0][0]);

    GLint projLocation = glGetUniformLocation(m_lighting_shader, "projMat");
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &m_projMatrix[0][0]);

    // Pass m_ka into the fragment shader as a uniform
    GLint ambientLocation = glGetUniformLocation(m_lighting_shader, "ambient");
    glUniform1f(ambientLocation, m_data.globalData.ka);

    // Pass light position and m_kd into the fragment shader as a uniform
    GLint diffuseLocation = glGetUniformLocation(m_lighting_shader, "diffuse");
    glUniform1f(diffuseLocation, m_data.globalData.kd);

    // Pass shininess, m_ks, and world-space camera position
    GLint specularLocation = glGetUniformLocation(m_lighting_shader, "specular");
    glUniform1f(specularLocation, m_data.globalData.ks);

    glm::vec4 cameraPos = inverse(m_viewMatrix) * glm::vec4{0, 0, 0, 1}; // viewMat * origin
    GLint cameraPosLocation = glGetUniformLocation(m_lighting_shader, "cameraPos");
    glUniform4fv(cameraPosLocation, 1, &cameraPos[0]);

    int numLights = m_data.lights.size();

    GLint numLightsLocation = glGetUniformLocation(m_lighting_shader, "numLights");
    glUniform1i(numLightsLocation, numLights);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_water_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_displacement_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_height_texture);

    GLint typeLocation;
    GLint colorLocation;
    GLint lightDirLocation;
    GLint lightPosLocation;
    GLint attenuationLocation;
    GLint angleLocation;
    GLint penumbraLocation;
    for (int i = 0; i < numLights; i++) {
        int lightType;
        switch(m_data.lights[i].type) {
            case LightType::LIGHT_POINT :
                lightType = 0;
                lightPosLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].lightPos").c_str());
                glUniform4fv(lightPosLocation, 1, &m_data.lights[i].pos[0]);
                break;
            case LightType::LIGHT_DIRECTIONAL :
                lightType = 1;
                lightDirLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].lightDir").c_str());
                glUniform4fv(lightDirLocation, 1, &m_data.lights[i].dir[0]);
                break;
            case LightType::LIGHT_SPOT :
                lightType = 2;
                lightDirLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].lightDir").c_str());
                glUniform4fv(lightDirLocation, 1, &m_data.lights[i].dir[0]);

                lightPosLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].lightPos").c_str());
                glUniform4fv(lightPosLocation, 1, &m_data.lights[i].pos[0]);

                angleLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].angle").c_str());
                glUniform1f(angleLocation, m_data.lights[i].angle);

                penumbraLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].penumbra").c_str());
                glUniform1f(penumbraLocation, m_data.lights[i].penumbra);
                break;
            default : // default -> Directional
                lightType = 1;
                lightDirLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].lightDir").c_str());
                glUniform4fv(lightDirLocation, 1, &m_data.lights[i].dir[0]);
                break;
        }

        typeLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].type").c_str());
        glUniform1i(typeLocation, lightType);

        colorLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].color").c_str());
        glUniform4fv(colorLocation, 1, &m_data.lights[i].color[0]);

        attenuationLocation = glGetUniformLocation(m_lighting_shader, ("lighting[" + std::to_string(i) + "].attenuation").c_str());
        glUniform3fv(attenuationLocation, 1, &m_data.lights[i].function[0]);
    }

    // Loop over shapes in scene
    for (RenderShapeData &shape : m_data.shapes) {

        GLint modelLocation = glGetUniformLocation(m_lighting_shader, "modelMat");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &shape.ctm[0][0]);

        glm::mat3 itModelMat = glm::inverse(glm::transpose(shape.ctm));
        GLint inverseLocation = glGetUniformLocation(m_lighting_shader, "itModelMat");
        glUniformMatrix3fv(inverseLocation, 1, GL_FALSE, &itModelMat[0][0]);

        GLint isMeshLocation = glGetUniformLocation(m_lighting_shader, "isMesh");
        glUniform1i(isMeshLocation, m_isMesh);

        GLint shininessLocation = glGetUniformLocation(m_lighting_shader, "shininess");
        glUniform1f(shininessLocation, shape.primitive.material.shininess);

        GLint materialAmbient = glGetUniformLocation(m_lighting_shader, "materialAmbient");
        glUniform4fv(materialAmbient, 1, &shape.primitive.material.cAmbient[0]);

        GLint materialDiffuse = glGetUniformLocation(m_lighting_shader, "materialDiffuse");
        glUniform4fv(materialDiffuse, 1, &shape.primitive.material.cDiffuse[0]);

        GLint materialSpecular = glGetUniformLocation(m_lighting_shader, "materialSpecular");
        glUniform4fv(materialSpecular, 1, &shape.primitive.material.cSpecular[0]);

        glUniform1i(glGetUniformLocation(m_lighting_shader, "shapeType"), (int)shape.primitive.type);

        GLint rotationLocation;
        float coneMult = m_cone_id % 2 == 0 ? 1.f : -1.f;

        // Draw Command
        switch (shape.primitive.type) {
            case PrimitiveType::PRIMITIVE_CONE:
                rotationLocation = glGetUniformLocation(m_lighting_shader, "rotationMat");

                m_rotationMatrix = glm::rotate(coneMult * float(m_rotation_time), glm::vec3{0, 1, 0});
                glUniformMatrix4fv(rotationLocation, 1, GL_FALSE, &m_rotationMatrix[0][0]);
                m_cone_id++;
                glBindVertexArray(m_coneVao);
                m_numTriangles = m_coneData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            case PrimitiveType::PRIMITIVE_CUBE:
                glBindVertexArray(m_cubeVao);
                m_numTriangles = m_cubeData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            case PrimitiveType::PRIMITIVE_PLANE:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glBindVertexArray(m_planeVao);
                m_numTriangles = m_planeData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                glDisable(GL_BLEND);
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                glBindVertexArray(m_cylinderVao);
                m_numTriangles = m_cylinderData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            case PrimitiveType::PRIMITIVE_SPHERE:
                glBindVertexArray(m_sphereVao);
                m_numTriangles = m_sphereData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            case PrimitiveType::PRIMITIVE_MESH:
                glBindVertexArray(m_meshVao);
                m_numTriangles = m_meshData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            case PrimitiveType::PRIMITIVE_INVERTCUBE:
                glBindVertexArray(m_invertCubeVao);
                m_numTriangles = m_invertCubeData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
            default:
                glBindVertexArray(m_cubeVao);
                m_numTriangles = m_cubeData.size() / 6.f;
                glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);
                glBindVertexArray(0);
                break;
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // PING PONGGG
    glUseProgram(m_pixeloutline_shader);

    bool isOutline = true;
    bool first_iteration = true;
    int textureSlot = 5;

    for (unsigned int i =0; i<2; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[isOutline]);
        glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "isOutline"), isOutline);


        if (first_iteration){
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, default_texture);
            //glUniform1i(glGetUniformLocation(m_pixeloutline_shader, "m_texture"), 4);
            first_iteration = false;

        } else {
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!isOutline]);
        }

        glBindVertexArray(m_fullscreen_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        isOutline = !isOutline;
        textureSlot ++;
    }

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete: " << fboStatus << std::endl;


    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_postprocess_shader);
    glBindVertexArray(m_fullscreen_vao);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!isOutline]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUseProgram(0);

    // Deactivate the shader program by passing 0 into
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here

    glClearColor(0, 0, 0, 0);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    makeFBO();

    // reset camera size/recalculate proj matrix
    m_camera.init(m_data.cameraData, size().width(), size().height());
    m_projMatrix = m_camera.getProjectionMatrix();
    m_viewMatrix = m_camera.getViewMatrix();
}

void Realtime::sceneChanged() {
    makeCurrent();
    int success;
    if (settings.sceneFilePath.ends_with(".xml")) {
        success = SceneParser::parse(settings.sceneFilePath, m_data);
    } else {
        success = SceneParser::parseMesh(settings.sceneFilePath, m_data);
        m_isMesh = true;
        if (m_data.shapes.empty()) {
            std::cout << "some error while parsing mesh";
        } else {
            std::cout << "binding mesh" << std::endl;
            bindMesh();
        }
    }

    if (success == -1) {
        std::cerr << "Error loading scene: \"" << settings.sceneFilePath << "\"" << std::endl;
        exit(1);
    }


    m_camera.init(m_data.cameraData, size().width(), size().height());
    m_projMatrix = m_camera.getProjectionMatrix();
    m_viewMatrix = m_camera.getViewMatrix();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    makeCurrent();
    bool planeChanged = false;
    // update near plane
    if (settings.nearPlane != m_nearPlane) {
        m_nearPlane = settings.nearPlane;
        planeChanged = true;
    }
    // update far plane
    if (settings.farPlane != m_farPlane) {
        m_farPlane = settings.farPlane;
        planeChanged = true;
    }

    // updates camera projection matrix if near or far plane change
    if (planeChanged) {
        m_camera.init(m_data.cameraData, size().width(), size().height());
        m_projMatrix = m_camera.getProjectionMatrix();
    }

//    // updates shapes
    if (settings.shapeParameter1 != m_currParam1 || settings.shapeParameter2 != m_currParam2) {
        bool updateCube = false;
        if (settings.shapeParameter1 != m_currParam1) {
            updateCube = true;
        }
        m_currParam1 = settings.shapeParameter1;
        m_currParam2 = settings.shapeParameter2;

        if (m_setupComplete) { // only updates VBOs if initializeGL has already been called
            glBindVertexArray(m_coneVao);
            bindCone();
            glBindVertexArray(0);
            if (updateCube) { // only updates cube if param 1 is changed, since it does not use param2
                glBindVertexArray(m_cubeVao);
                bindCube();
                glBindVertexArray(0);

            }
            glBindVertexArray(m_planeVao);
            bindPlane();
            glBindVertexArray(0);
            glBindVertexArray(m_cylinderVao);
            bindCylinder();
            glBindVertexArray(0);
            glBindVertexArray(m_sphereVao);
            bindSphere();
            glBindVertexArray(0);
            m_updated = true;
        }
    }

    update(); // asks for a PaintGL() call to occur
}

void Realtime::bindCone() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_coneVbo); // Binds the cone VBO
    if (m_updated) {
        m_cone.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    } else {
        m_cone.updateParams(1, 6);
    }
    m_coneData = m_cone.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_coneData.size(), m_coneData.data(), GL_STATIC_DRAW); // passes cone data into vbo
    glEnableVertexAttribArray(0); // adds position attribute
    glEnableVertexAttribArray(1); // adds normal attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds cone Vbo
}

void Realtime::bindCube() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVbo); // Binds the cube VBO
    if (m_updated) {
        m_cube.updateParams(settings.shapeParameter1, false);
    } else {
        m_cube.updateParams(determineTesselation(), false);
    }
    m_cubeData = m_cube.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_cubeData.size(), m_cubeData.data(), GL_STATIC_DRAW); // passes cube data into vbo
    // ~~ PRE SARAH CODE
//    glEnableVertexAttribArray(0); // adds position attribute
//    glEnableVertexAttribArray(1); // adds normal attribute
//    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
//    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    // ~~ SARAH CODE
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(6 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds cube Vbo
}

void Realtime::bindInvertCube() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_invertCubeVbo); // Binds the cube VBO
    if (m_updated) {
        m_cube.updateParams(settings.shapeParameter1, true);
    } else {
        m_cube.updateParams(determineTesselation(), true);
    }
    m_invertCubeData = m_cube.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_invertCubeData.size(), m_invertCubeData.data(), GL_STATIC_DRAW); // passes cube data into vbo

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(6 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds cube Vbo
}

void Realtime::bindPlane() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_planeVbo); // Binds the cube VBO
    if (m_updated) {
        m_plane.updateParams(settings.shapeParameter1);
    } else {
        m_plane.updateParams(determineTesselation());
    }
    m_planeData = m_plane.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_planeData.size(), m_planeData.data(), GL_STATIC_DRAW); // passes cube data into vbo

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, reinterpret_cast<void*>(6 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds cube Vbo
}

void Realtime::bindCylinder() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_cylinderVbo); // Binds the cylinder VBO
    if (m_updated) {
        m_cylinder.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    } else {
        m_cylinder.updateParams(determineTesselation(), determineTesselation());
    }
    m_cylinderData = m_cylinder.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_cylinderData.size(), m_cylinderData.data(), GL_STATIC_DRAW); // passes cylinder data into vbo
    glEnableVertexAttribArray(0); // adds position attribute
    glEnableVertexAttribArray(1); // adds normal attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds cylinder Vbo
}

void Realtime::bindSphere() {
    makeCurrent();
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereVbo); // Binds the sphere VBO
    if (m_updated) {
        m_sphere.updateParams(settings.shapeParameter1, settings.shapeParameter2);
    } else {
        m_sphere.updateParams(determineTesselation(), determineTesselation());
    }
    m_sphereData = m_sphere.generateShape();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_sphereData.size(), m_sphereData.data(), GL_STATIC_DRAW); // passes sphere data into vbo
    glEnableVertexAttribArray(0); // adds position attribute
    glEnableVertexAttribArray(1); // adds normal attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds sphere Vbo
}

void Realtime::bindMesh() {
    makeCurrent();
    glBindVertexArray(m_meshVao);
    Mesh mesh;
    mesh.init();
    glBindBuffer(GL_ARRAY_BUFFER, m_meshVbo);
    m_meshData = mesh.generateShape(m_data.shapes[0].meshData);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_meshData.size(), m_meshData.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); // adds position attribute
    glEnableVertexAttribArray(1); // adds normal attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbinds mesh Vbo
    glBindVertexArray(0);
}

void Realtime::makeFBO() {

    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        if (deltaX != 0) {
            float cos = glm::cos(deltaX/400.f);
            float sin = glm::sin(deltaX/400.f);
            glm::vec3 u = {0, 1, 0};
            glm::mat4 rotationMat = {
                cos,         0,                                 -(u.y * sin), 0,
                0,           cos + pow(u.y, 2.f) * (1.f - cos), 0,            0,
                (u.y * sin), 0,                                 cos,          0,
                0,           0,                                 0,            1.f
            };

            rotate(rotationMat);

            update(); // asks for a PaintGL() call to occur
        }
        if (deltaY != 0) {
            float cos = glm::cos(glm::radians(deltaY/10.f));
            float sin = glm::sin(glm::radians(deltaY/10.f));
            glm::vec3 u = m_camera.getPerpLookVector();
            glm::mat4 rotationMat = {
                cos + pow(u.x, 2.f) * (1.f - cos),       (u.x * u.y) * (1.f - cos) + (u.z * sin), (u.x * u.z) * (1.f - cos) - (u.y * sin), 0,
                (u.x * u.y) * (1.f - cos) - (u.z * sin), cos + pow(u.y, 2.f) * (1.f - cos),       (u.y * u.z) * (1.f - cos) + (u.x * sin), 0,
                (u.x * u.z) * (1.f - cos) + (u.y * sin), (u.y * u.z) * (1.f - cos) + (u.x * sin), cos + pow(u.z, 2.f) * (1.f - cos),       0,
                0,                                       0,                                       0,                                       1.f
            };

            rotate(rotationMat);

            update(); // asks for a PaintGL() call to occur
        }
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();
    m_water_time++;
    m_displacement_time++;
    if (m_water_time > 640) {
        m_water_time = 0;
    }
    if (m_displacement_time > 1280) {
        m_displacement_time = 0;
    }
    if (m_water_time % 2 == 0) {
        m_rotation_time++;
    }
    update();

    // Use deltaTime and m_keyMap here to move around
    float dx = 0.f, dy = 0.f, dz = 0.f;
    glm::vec3 look = m_camera.getLookVector();
    glm::vec3 side = m_camera.getPerpLookVector();

    // If statement tower because multiple keys can be pressed at once
    if (m_keyMap[Qt::Key_W] == true) {
        dx += look.x;
        dy += look.y;
        dz += look.z;
    }
    if (m_keyMap[Qt::Key_S] == true) {
        dx -= look.x;
        dy -= look.y;
        dz -= look.z;
    }
    if (m_keyMap[Qt::Key_A] == true) {
        dx -= side.x;
        dy -= side.y;
        dz -= side.z;
    }
    if (m_keyMap[Qt::Key_D] == true) {
        dx += side.x;
        dy += side.y;
        dz += side.z;
    }
    if (m_keyMap[Qt::Key_Control] == true) {
        dy -= 1;
    }
    if (m_keyMap[Qt::Key_Space] == true) {
        dy += 1;
    }

    if (dx != 0 || dy != 0 || dz != 0) { // if a translation is occurring
        translate(5.f * deltaTime * dx, 5.f * deltaTime * dy, 5.f * deltaTime * dz);
        update(); // asks for a PaintGL() call to occur
    }

}

void Realtime::translate(float dx, float dy, float dz) {
    glm::mat4 translationMat(1.f); // identity matrix
    translationMat[3][0] = dx;
    translationMat[3][1] = dy;
    translationMat[3][2] = dz;
    m_data.cameraData.pos = translationMat * m_data.cameraData.pos;
    // since position is changing, recalculate view matrix
    m_camera.init(m_data.cameraData, size().width(), size().height());
    m_viewMatrix = m_camera.getViewMatrix();
    // updates cameraPos in shader since viewMatrix is changing
    glUseProgram(m_lighting_shader);
    glm::vec4 cameraPos = inverse(m_viewMatrix) * glm::vec4{0, 0, 0, 1}; // viewMat * origin
    GLint cameraPosLocation = glGetUniformLocation(m_lighting_shader, "cameraPos");
    glUniform4fv(cameraPosLocation, 1, &cameraPos[0]);
    glUseProgram(0);
}

void Realtime::rotate(glm::mat4 rotation) {
    m_data.cameraData.look = rotation * m_data.cameraData.look;
    // since look vector is changing, update both projection and view matrices
    m_camera.init(m_data.cameraData, size().width(), size().height());
    m_projMatrix = m_camera.getProjectionMatrix();
    m_viewMatrix = m_camera.getViewMatrix();
    // updates cameraPos in shader since viewMatrix is changing
    glUseProgram(m_lighting_shader);
    glm::vec4 cameraPos = inverse(m_viewMatrix) * glm::vec4{0, 0, 0, 1}; // viewMat * origin
    GLint cameraPosLocation = glGetUniformLocation(m_lighting_shader, "cameraPos");
    glUniform4fv(cameraPosLocation, 1, &cameraPos[0]);
    glUseProgram(0);
}

QImage Realtime::raytraceScene() {

    QImage image = QImage(m_screen_width, m_screen_height, QImage::Format_RGBX8888);
    image.fill(Qt::black);
    RGBA *data = reinterpret_cast<RGBA *>(image.bits());

    // Setting up the raytracer
    RayTracer::Config rtConfig{};
    RayTracer raytracer{ rtConfig };

    RayTraceScene rtScene{ m_screen_width, m_screen_height, m_data };

    raytracer.render(data, rtScene);
    return image;
}

int Realtime::determineTesselation() {
    int numShapes = m_data.shapes.size();
    if (numShapes > 30) {
        return 3;
    } else if (numShapes > 25) {
        return 5;
    } else if (numShapes > 20) {
        return 10;
    } else if (numShapes > 15) {
        return 15;
    } else if (numShapes > 10) {
        return 18;
    } else if (numShapes > 5) {
        return 22;
    } else {
        return 25;
    }
}
