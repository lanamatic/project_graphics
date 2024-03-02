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
unsigned int loadTexture(char const * path);
void renderGround();


Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

bool firstMouse = true;

float lastX = SCR_WIDTH/2.0;
float lastY = SCR_HEIGHT/2.0;
float heightScale = 0.1;
bool gammaEnabled = false;
bool gammaKeyPressed = false;


float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    float constant;
    float linear;
    float quadratic;
};


int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    //create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "you got this", nullptr, nullptr);
    if(window == nullptr){
        std::cerr << "Failed to create a window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //load opengl functions
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to init GLAD!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    glEnable(GL_DEPTH_TEST);

    //Shader groundShader(FileSystem::getPath("resources/shaders/ground.vs").c_str(), FileSystem::getPath("resources/shaders/ground.fs").c_str());
    Shader modelShader(FileSystem::getPath("resources/shaders/models.vs").c_str(), FileSystem::getPath("resources/shaders/models.fs").c_str());
    Model tatooine(FileSystem::getPath("resources/objects/tatooine/scene.gltf"));
    stbi_set_flip_vertically_on_load(true);


    //TODO: lights
//    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/dune.jpg").c_str());
//    unsigned int normalMap  = loadTexture(FileSystem::getPath("resources/textures/dune_normal.jpg").c_str());
//    unsigned int heightMap  = loadTexture(FileSystem::getPath("resources/textures/dune_height.png").c_str());
//
//
//    groundShader.use();
//    groundShader.setInt("diffuseMap", 0);
//    groundShader.setInt("normalMap", 1);
//    groundShader.setInt("depthMap", 2);

//    glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

    //translation for houses
    glm::vec3 translation[10];
    float z[12] = {
            0.0f, -6.0f, -7.0f,-9.0f, 0.0f, -2.0f,
            9.0f, 15.0f, 13.0f,5.0f, 6.0f, 10.0f
    };
    float x[12]  = {
            0.0f, -10.0, 8.0f, 2.0f, 12.0f, -15.0,
            0.0f, -6.0f, 9.f, -10.0f, 11.0, -15.0f
    };

    while(!glfwWindowShouldClose(window)){

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        procesInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        modelShader.use();
        //view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        modelShader.setMat4("projection",projection);
        modelShader.setMat4("view",view);

        //drawing houses one by one, it was faster then instancing
        for(unsigned int i = 0; i < 12;i++){
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x[i], 0.0f, z[i]));
            if(i < 5){
                model = glm::rotate(model, (float)glm::radians(-90.0), glm::vec3(0, 0, 1));
                model = glm::rotate(model, (float)glm::radians(-90.0), glm::vec3(0, 1, 0));
                model = glm::scale(model, glm::vec3(0.5));
            }else{
                model = glm::rotate(model, (float)glm::radians(-90.0), glm::vec3(0, 0, 1));
                model = glm::rotate(model, (float)glm::radians(90.0), glm::vec3(0, 1, 0));
                model = glm::rotate(model, (float)glm::radians(180.0), glm::vec3(1, 0, 0));
                model = glm::scale(model, glm::vec3(0.5));
            }

            modelShader.setMat4("model", model);
            tatooine.Draw(modelShader);
        }
//        groundShader.use();
//        groundShader.setMat4("projection", projection);
//        groundShader.setMat4("view", view);
//
//        // render parallax-mapped ground
//        glm::mat4 model = glm::mat4(1.0f);
//        groundShader.setMat4("model", model);
//        groundShader.setVec3("viewPos", camera.Position);
//        groundShader.setVec3("lightPos", lightPos);
//        groundShader.setFloat("heightScale", heightScale); // adjust with Q and E keys
//        groundShader.setBool("gamma",gammaEnabled);
//        std::cout << heightScale << std::endl;
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, diffuseMap);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_2D, normalMap);
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, heightMap);
//        renderGround();
//
//        std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
//

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

    //gamma correction
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !gammaKeyPressed)
    {
        gammaEnabled = !gammaEnabled;
        gammaKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        gammaKeyPressed = false;
    }

    //heightScale for parallax mapping
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    } else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }

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

//loading 2D textures
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//Normal-Parallax mapping for ground
unsigned int groundVAO = 0;
unsigned int groundVBO;
void renderGround(){

    if (groundVAO == 0)
    {
        // positions
        glm::vec3 pos1(60.0f,  0.0f, 60.0f);
        glm::vec3 pos2(60.0f, 0.0f, -60.0f);
        glm::vec3 pos3( -60.0f, 0.0f, -60.0f);
        glm::vec3 pos4( -60.0f,  0.0f, 60.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float groundVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &groundVAO);
        glGenBuffers(1, &groundVBO);
        glBindVertexArray(groundVAO);
        glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), &groundVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}