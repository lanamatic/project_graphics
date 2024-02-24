//
// Created by Lana Matic on 13.2.24..
//

#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "learnopengl/shader.h"
#include "rg/Error.h"
#include "learnopengl/filesystem.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <cmath>
#include "stb_image.h"
#include "learnopengl/camera.h"
#include "learnopengl/model.h"

const unsigned SCR_WIDTH = 800;
const unsigned SCR_HEIGHT = 600;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void procesInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);


Camera camera;

bool firstMouse = true;

float lastX = SCR_WIDTH/2.0;
float lastY = SCR_HEIGHT/2.0;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "you got this", nullptr, nullptr);
    if(window == nullptr){
        std::cerr << "Failed to create a window!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to init GLAD!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);


    camera.Position = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.Front = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    while(!glfwWindowShouldClose(window)){

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        procesInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);





        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
    glViewport(0, 0, width, height);
}

void procesInput(GLFWwindow *window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){

    if(firstMouse){
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){

    camera.ProcessMouseScroll(yoffset);
}
