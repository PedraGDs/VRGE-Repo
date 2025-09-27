#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "util/intrinsics.h"

const char* vertexShaderSource = "#version 330 core\n layout (location = 0) in vec3 aPos;\n "
"void main(){\n"
"FragColor = vec4(aPos.x, aPos.y, aPos.Z, 1.0F);\n"
"}n\0";

const char* fragmentShaderSource = "#version 330 core\n out vec4 FragColor;\n "
"void main(){\n"
"FragColor = vec4(0.8F, 0.3F, 0.02F, 1.0F);\n"
"}n\0";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main(int argc, char** args) 
{
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Sexy Window Title", NULL, NULL);
    if (!window) {
        std::cout << "Failed to open GLFW window" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glViewport(0, 0, 800, 600);

    GLfloat vtxs[] = {
        -0.5F, sqrtf(3) / -6, 0.0F,
         0.5F, sqrtf(3) / -6, 0.0F,
         0.0F, sqrtf(3) / 3, 0.0F
    };

    GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vtxShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vtxShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vtxShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vtxShader);
    glDeleteShader(fragmentShader);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtxs), vtxs, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSwapBuffers(window);

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.07F, 0.13F, 0.17F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
