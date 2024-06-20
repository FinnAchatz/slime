#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "mygl/base.h"
#include "mygl/shader.h"

#include "config.h"

void checkShaderCompilation(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void checkProgramLinking(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

/* GLFW callback function for window resize event */
void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    g_ProgramConfiguration.width = width;
    g_ProgramConfiguration.height = height;
}

int main() {
    GLFWwindow* window = windowCreate("Assignment 1 - Transformations, User Input and Camera", 800, 600);
    if(!window) { return EXIT_FAILURE; }

    // set glfw callbacks
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);

    // Viewport setup
    glViewport(0, 0, 800, 600);

    // Vertex data
    float vertices[] = {
        -1.f, -1.f, 0.0f,
         1.f, -1.f, 0.0f,
         1.f,  1.f, 0.0f,
        -1.f,  1.f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create VAO, VBO, and EBO
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    ShaderProgram shaderProgram = shaderLoad("shader/default.vert", "shader/default.frag");

    glUseProgram(shaderProgram.id);


    srand(1);

    int size_x = 800;
    int size_y = 600;
    int len = size_x * size_y;

    float * values = new float [len];
    for (size_t i = 0; i < len; i++)
    {
        values[i] = float(rand()) / RAND_MAX; 
        // values[i] = i % 4 == 0 ? 0.0 : 1.0;
    }

    // transfer pixels
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size_x, size_y, 0, GL_RED, GL_FLOAT, &values[0]);
    
    // Set the screen size uniform
    // int screenSizeLoc = glGetUniformLocation(shaderProgram.id, "screenSize");
    // glUniform2d(screenSizeLoc, 800, 600);
    glUniform2f( glGetUniformLocation(shaderProgram.id, "uSize"), float(size_x), float(size_y));
    glUniform1fv( glGetUniformLocation(shaderProgram.id, "uValues"), len, values);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Render
        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < len; i++)
        {
            values[i] = float(rand()) / RAND_MAX; 
            // values[i] = i % 4 == 0 ? 0.0 : 1.0;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size_x, size_y, 0, GL_RED, GL_FLOAT, &values[0]);


        glUseProgram(shaderProgram.id);
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    shaderDelete(shaderProgram);

    glfwTerminate();
    return 0;
}
