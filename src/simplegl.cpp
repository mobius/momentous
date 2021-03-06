//========================================================================
// Simple GLFW example
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//! [code]
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include "math_types.h"


static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

const char* vertex_shader =
"#version 400\n"
"layout(location = 0) in vec3 vp;"
"layout(location = 1) in vec3 color;"
"out vec3 fragmentColor;"
"void main () {"
"  gl_Position = vec4 (vp, 1.0);"
"  fragmentColor = color;"
"}";

const char* fragment_shader =
"#version 400\n"
"in vec3 fragmentColor;"
"out vec4 color;"
"void main () {"
"  color = vec4(fragmentColor,1.0);"
"}";


GLuint vbo, vao, vs, ps, shader;

void init()
{
    using namespace math;

    float points[] = 
    {
        -1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 1.0f,
    };

    // bind and upload vertex data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    // bind and create vertex layout( currect active array buffer object is "vbo")
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (const GLvoid*)(3*sizeof(float)));

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    ps = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(ps, 1, &fragment_shader, NULL);
    glCompileShader(ps);

    shader = glCreateProgram();
    glAttachShader(shader, ps);
    glAttachShader(shader, vs);
    glLinkProgram(shader);

}

int main(void)
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK )
        exit(EXIT_FAILURE);

    if( !GLEW_VERSION_4_2 )
        exit(EXIT_FAILURE);
    

    glfwSetKeyCallback(window, key_callback);

    init();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind shader
        glUseProgram(shader);

        // bind to layout
        glBindVertexArray(vao);
        // bind to vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // draw
        glDrawArrays(GL_TRIANGLES, 0, 3);


        glfwSwapBuffers(window);
        glfwPollEvents();

        Sleep(1);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
