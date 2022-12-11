#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "utils/sceneparser.h"
#include "camera/camera.h"
#include "shapes/Cone.h"
#include "shapes/Cube.h"
#include "shapes/Cylinder.h"
#include "shapes/Sphere.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    QImage raytraceScene();

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void bindCone();
    void bindCube();
    void bindCylinder();
    void bindSphere();
    void bindMesh();
    int determineTesselation();

    void makeFBO();
    void makeSecondFBO();
    void paintTexture(GLuint texture, GLuint texture2, bool perPixel, bool kernelBased, bool extraCredit1, bool extraCredit2);

    void translate(float dx, float dy, float dz);
    void rotate(glm::mat4 rotation);

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;

    Camera m_camera;
    RenderData m_data;

    // Shape to render
    Cone m_cone;
    Cube m_cube;
    Cylinder m_cylinder;
    Sphere m_sphere;

    // Params
    int m_currParam1;
    int m_currParam2;
    float m_nearPlane;
    float m_farPlane;
    int m_screen_width;
    int m_screen_height;
    int m_fbo_width;
    int m_fbo_height;
    glm::mat4 m_viewMatrix = glm::mat4(1);
    glm::mat4 m_projMatrix = glm::mat4(1);

    std::vector<GLfloat> m_coneData;
    std::vector<GLfloat> m_cubeData;
    std::vector<GLfloat> m_cylinderData;
    std::vector<GLfloat> m_sphereData;
    std::vector<GLfloat> m_meshData;
    int m_numTriangles;
    bool m_setupComplete = false;
    bool m_updated = false;
    bool m_isMesh = false;

    GLuint m_lighting_shader; // Stores id of lighting shader program
    GLuint m_texture_shader; // Stores id of texture shader program
    GLuint m_defaultFBO; // Stores default FBO value
    GLuint m_fbo;
    GLuint m_fbo2;
    GLuint m_fbo_texture;
    GLuint m_fbo2_texture;
    GLuint m_intermediate_texture;
    GLuint m_fbo_renderbuffer;
    GLuint m_fbo2_renderbuffer;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    GLuint m_coneVbo; // Stores id of cone VBO
    GLuint m_cubeVbo; // Stores id of cube VBO
    GLuint m_cylinderVbo; // Stores id of cylinder VBO
    GLuint m_sphereVbo; // Stores id of sphere VBO
    GLuint m_meshVbo;

    GLuint m_coneVao; // Stores id of cone VAO
    GLuint m_cubeVao; // Stores id of cube VAO
    GLuint m_cylinderVao; // Stores id of cylinder VAO
    GLuint m_sphereVao; // Stores id of sphere VAO
    GLuint m_meshVao;

    QImage m_water_image;
    QImage m_displacement_image;
    GLuint m_water_texture;
    GLuint m_displacement_texture;
    int m_water_time;
    int m_displacement_time;
};
