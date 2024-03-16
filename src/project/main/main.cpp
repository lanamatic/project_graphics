//
// Created by Lana Matic on 13.2.24..
//

#define GLFW_INCLUDE_NONE
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
void keycallback(GLFWwindow* window, int key, int scanode, int action, int mods);
unsigned int loadTexture(char const * path);
unsigned int loadCubemap(vector<std::string> faces);
void renderQuad();
void renderCube();
void renderGround();


//Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));

bool firstMouse = true;

float lastX = SCR_WIDTH/2.0;
float lastY = SCR_HEIGHT/2.0;
float heightScale = 0.1;

bool gammaEnabled = false;
bool gammaKeyPressed = false;
bool blinn = false;
bool blinnKeyPressed = false;
bool hdr = true;
bool hdrKeyPressed = false;
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 0.77f;
bool grayscale = false;
bool grayscaleKeyPressed = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

};

struct DirLight {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

};

struct ProgramState{
    bool ImGuiEnabled = false;
    Camera camera;
    ProgramState() : camera(glm::vec3(0.0f, 0.0f, 4.0f)){}

    float angular_speed = 1.5f;

    void LoadFromDisk(std::string path);
    void SaveToDisk(std::string path);

};

void ProgramState::SaveToDisk(std::string path){

    std::ofstream out(path);
    out << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.x << '\n'
        << camera.Front.x << '\n'
        << camera.Pitch << '\n'
        << camera.Yaw << '\n';
}

void ProgramState::LoadFromDisk(std::string path){

    std::ifstream in(path);
    if(in){
        in >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.x
           >> camera.Front.x
           >> camera.Pitch
           >> camera.Yaw;
    }
}

ProgramState* programState;
void DrawImGui(ProgramState* programState);
bool cameraInfo = false;
static void Help(const char* desc, bool extraText = false);

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
    glfwSetKeyCallback(window, keycallback);

    //load opengl functions
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)){
        std::cerr << "Failed to init GLAD!\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    //ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    programState = new ProgramState;
    programState->LoadFromDisk(FileSystem::getPath("resources/programState.txt"));

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if(programState->ImGuiEnabled){
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    Shader groundShader(FileSystem::getPath("resources/shaders/ground.vs").c_str(), FileSystem::getPath("resources/shaders/ground.fs").c_str());
    Shader modelShader(FileSystem::getPath("resources/shaders/models.vs").c_str(), FileSystem::getPath("resources/shaders/models.fs").c_str());
    Shader lightShader(FileSystem::getPath("resources/shaders/models.vs").c_str(), FileSystem::getPath("resources/shaders/lightCubes.fs").c_str());
    Shader blurShader(FileSystem::getPath("resources/shaders/blur.vs").c_str(), FileSystem::getPath("resources/shaders/blur.fs").c_str());
    Shader finalShader(FileSystem::getPath("resources/shaders/final.vs").c_str(), FileSystem::getPath("resources/shaders/final.fs").c_str());
    Shader blendShader(FileSystem::getPath("resources/shaders/blending.vs").c_str(), FileSystem::getPath("resources/shaders/blending.fs").c_str());
    Shader skyboxShader(FileSystem::getPath("resources/shaders/skybox.vs").c_str(), FileSystem::getPath("resources/shaders/skybox.fs").c_str());

    Model tatooine(FileSystem::getPath("resources/objects/tatooine/scene.gltf"));
    tatooine.SetTextureNamePrefix("material.");

    Model mando(FileSystem::getPath("resources/objects/mando/scene.gltf"));
    mando.SetTextureNamePrefix("material.");

    Model sphere(FileSystem::getPath("resources/objects/sfera/scene.gltf"));
    sphere.SetTextureNamePrefix("material.");

    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());

    stbi_set_flip_vertically_on_load(true);

    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/dune.jpg").c_str());
    unsigned int normalMap  = loadTexture(FileSystem::getPath("resources/textures/dune_normal.jpg").c_str());
    unsigned int heightMap  = loadTexture(FileSystem::getPath("resources/textures/dune_height.png").c_str());


    //configure floating point framebuffer
    //multisample anti-aliacing
//----------------------------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    //2 floating point color buffers: 1-normal rendering, 2-brightness values
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++){
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBuffers[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, colorBuffers[i], 0);
    }

    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);


    //depth attachment
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    //which color attachment we'll use for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    //is framebuffer complete?
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++){
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpongColorbuffers[i]);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, pingpongColorbuffers[i], 0);
        //are framebuffers complete?
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    //Lights
//----------------------------------------------------------

    //directional light init
    DirLight directional;
    directional.direction = glm::vec3(-0.7f, -1.0f, -0.4f);
    directional.ambient = glm::vec3(0.09f);
    directional.diffuse = glm::vec3(0.4f);
    directional.specular = glm::vec3(0.5f);

    //point light positions
    std::vector<glm::vec3> pointLightPositions;
    pointLightPositions.push_back(glm::vec3(-9.0f, 0.1f, -2.0f));
    pointLightPositions.push_back(glm::vec3(-5.0f, 0.1f, -4.0f));
    pointLightPositions.push_back(glm::vec3(-13.5f, 0.1f, -7.0f));
    pointLightPositions.push_back(glm::vec3(-5.0f, 0.1f, -8.0f));
    pointLightPositions.push_back(glm::vec3(0.3f, 0.1f, -6.0f));
    pointLightPositions.push_back(glm::vec3(7.0f, 0.1f, -2.0f));
    pointLightPositions.push_back(glm::vec3(12.0f, 0.1f, -5.0f));
    pointLightPositions.push_back(glm::vec3(-8.0f, 0.1f, 3.0f));
    pointLightPositions.push_back(glm::vec3(-2.5f, 0.1f, 5.0f));
    pointLightPositions.push_back(glm::vec3(-13.0f, 0.1f, 8.5f));
    pointLightPositions.push_back(glm::vec3(-5.0f, 0.1f, 11.0f));
    pointLightPositions.push_back(glm::vec3(7.0f, 0.1f, 4.5f));
    pointLightPositions.push_back(glm::vec3(2.0f, 0.1f, 6.5f));
    pointLightPositions.push_back(glm::vec3(13.0f, 0.1f, 11.0f));
    pointLightPositions.push_back(glm::vec3(4.0f, 0.1f, -2.0f));
    pointLightPositions.push_back(glm::vec3(-3.5f, 0.1f, -2.1f));
    pointLightPositions.push_back(glm::vec3(9.0f, 0.1f, 3.0f));
    pointLightPositions.push_back(glm::vec3 (3.0f, 0.1f, 2.3f));

    //point light init
    PointLight pointLights;
    pointLights.ambient = glm::vec3(5.5f, 3.7f, 1.0f);
    pointLights.diffuse = glm::vec3(5.5f, 3.7f, 1.0f);
    pointLights.specular = glm::vec3(5.5f, 3.7f, 1.0f);

    //spotlight init
    SpotLight spotlight;
    spotlight.position = programState->camera.Position;
    spotlight.direction = programState->camera.Front;
    spotlight.ambient = glm::vec3(0.2f);
    spotlight.diffuse = glm::vec3(1.0f, 0.894f, 0.627f);
    spotlight.specular = glm::vec3(1.0f, 0.894f, 0.627f);
    spotlight.cutOff = glm::cos(glm::radians(12.5f));
    spotlight.outerCutOff = glm::cos(glm::radians(17.0f));

//----------------------------------------------------------

    //translation for houses
    glm::vec3 translation[10];
    float z[12] = {
            -1.0f, -6.0f, -7.0f,-9.0f, 0.0f, -2.0f,
            9.0f, 15.0f, 13.0f,5.0f, 6.0f, 10.0f
    };
    float x[12]  = {
            0.0f, -10.0, 8.0f, 2.0f, 12.0f, -15.0,
            0.0f, -6.0f, 9.f, -10.0f, 11.0, -15.0f
    };

    float transparentVertices[] = {
            // positions         // texture Coords
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    vector<glm::vec3> grassPosition{
            glm::vec3( 1.6f, -0.2f, -1.4f),
            glm::vec3(1.9f, -0.2f, -1.5f),
            glm::vec3(9.5f, -0.2f, -7.9f),
            glm::vec3(9.8f, -0.2f, -7.8f),
            glm::vec3(-2.6f, -0.2f, 9.6f),
            glm::vec3(-2.8f, -0.2f, 9.7f)
    };

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> faces
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg")
            };
    unsigned int cubemapTexture = loadCubemap(faces);

    blurShader.use();
    blurShader.setInt("image", 0);
    finalShader.use();
    finalShader.setInt("scene", 0);
    finalShader.setInt("bloomBlur", 1);
    groundShader.use();
    groundShader.setInt("diffuseMap", 0);
    groundShader.setInt("normalMap", 1);
    groundShader.setInt("depthMap", 2);
    blendShader.use();
    blendShader.setInt("texture1", 0);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);


    while(!glfwWindowShouldClose(window)){

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        procesInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //sort the transparent grass before rendering
        std::map<float, glm::vec3> sorted;
        for (unsigned int i = 0; i < grassPosition.size(); i++)
        {
            float distance = glm::length(programState->camera.Position - grassPosition[i]);
            sorted[distance] = grassPosition[i];
        }

        //render scene into floating point framebuffer
//----------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelShader.use();
        //view/projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();

        modelShader.setMat4("projection",projection);
        modelShader.setMat4("view",view);

        modelShader.setVec3("viewPos", programState->camera.Position);
        modelShader.setFloat("material.shininess", 16.0f);

        //Directional light
        modelShader.setVec3("directional.direction", directional.direction);
        modelShader.setVec3("directional.ambient", directional.ambient);
        modelShader.setVec3("directional.diffuse", directional.diffuse);
        modelShader.setVec3("directional.specular", directional.specular);

        //Point Lights
        for(unsigned int i = 0; i < pointLightPositions.size(); i++){
            modelShader.setVec3("pointlight[" + std::to_string(i) + "].position", pointLightPositions[i]);
            modelShader.setVec3("pointlight[" + std::to_string(i) + "].ambient", pointLights.ambient * 0.05f);
            modelShader.setVec3("pointlight[" + std::to_string(i) + "].diffuse", pointLights.diffuse * 0.8f);
            modelShader.setVec3("pointlight[" + std::to_string(i) + "].specular", pointLights.specular * 0.1f);
        }

        //Spotlight
        modelShader.setVec3("spotlight.position", programState->camera.Position);
        modelShader.setVec3("spotlight.direction", programState->camera.Front);
        modelShader.setVec3("spotlight.ambient", spotlight.ambient);
        modelShader.setVec3("spotlight.diffuse", spotlight.diffuse);
        modelShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        modelShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);

        modelShader.setBool("blinn", blinn);
        std::cout << (blinn ? "Blinn-Phong" : "Phong") << std::endl;

        //drawing houses one by one was faster than instancing :)
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

        //face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        //mandalorian & grogu
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, -0.6f, 1.0f));
        model = glm::rotate(model, (float)glm::radians(-90.0), glm::vec3(1, 0, 0));
        model = glm::rotate(model, (float)glm::radians(-45.0), glm::vec3(0, 0, 1));
        model = glm::scale(model, glm::vec3(0.1));
        modelShader.setMat4("model", model);
        mando.Draw(modelShader);

        //spheres
        float center_x = 3.0f;
        float center_y = -0.4f;
        float center_z = 1.0f;
        float helix_radius = 0.5f;
        float helix_loop_height = 0.3f;

        for (int i = 0; i < 3; ++i) {
            float angle = glfwGetTime() * programState->angular_speed + (2.0f * M_PI / 3.0f) * i;
            float x = center_x + helix_radius * cos(angle);
            float y = center_y + i * helix_loop_height ;
            float z = center_z + helix_radius * sin(angle);

            // Draw the sphere
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3 (x, y, z));
            model = glm::scale(model, glm::vec3(0.05));
            modelShader.setMat4("model", model);
            sphere.Draw(modelShader);

        }

        //grass
        glDisable(GL_CULL_FACE);
        blendShader.use();
        blendShader.setMat4("projection", projection);
        blendShader.setMat4("view", view);
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, it->second);
            model = glm::scale(model, glm::vec3(1.0f));
            blendShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        glEnable(GL_CULL_FACE);

        //light cubes
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", glm::translate(view, glm::vec3(0.0f, 0.15f + 0.1*sin(glfwGetTime()), 0.0f)));
        model = glm::mat4(1.0f);
        for(unsigned int i = 0; i < pointLightPositions.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pointLightPositions[i]));
            model = glm::scale(model, glm::vec3(0.09f));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor",glm::vec3(5.5f, 3.7f, 1.0f));
            renderCube();
        }

        //ground
        groundShader.use();
        groundShader.setMat4("projection", projection);
        groundShader.setMat4("view", view);
        groundShader.setVec3("viewPos", programState->camera.Position);
        //dirlight
        groundShader.setVec3("directional.direction", directional.direction);
        groundShader.setVec3("directional.ambient", directional.ambient);
        groundShader.setVec3("directional.diffuse", directional.diffuse);
        groundShader.setVec3("directional.specular", directional.specular);
        //Point Lights
        for(unsigned int i = 0; i < pointLightPositions.size(); i++){
            groundShader.setVec3("pointlight[" + std::to_string(i) + "].position", pointLightPositions[i]);
            groundShader.setVec3("pointlight[" + std::to_string(i) + "].ambient", pointLights.ambient * 0.05f);
            groundShader.setVec3("pointlight[" + std::to_string(i) + "].diffuse", pointLights.diffuse * 0.7f);
            groundShader.setVec3("pointlight[" + std::to_string(i) + "].specular", pointLights.specular * 0.0f);
        }
        //Spotlight
        groundShader.setVec3("spotlight.position", programState->camera.Position);
        groundShader.setVec3("spotlight.direction", programState->camera.Front);
        groundShader.setVec3("spotlight.ambient", spotlight.ambient);
        groundShader.setVec3("spotlight.diffuse", spotlight.diffuse);

        groundShader.setFloat("spotlight.cutOff", spotlight.cutOff);
        groundShader.setFloat("spotlight.outerCutOff", spotlight.outerCutOff);

        groundShader.setBool("blinn", blinn);

        //normal and parallax mapping
        model = glm::mat4(1.0f);
        groundShader.setMat4("model", model);
        groundShader.setFloat("height_scale", heightScale);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, heightMap);

        renderGround();
        glDisable(GL_CULL_FACE);

        //skybox at last
        glDepthFunc(GL_LEQUAL);  // depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//----------------------------------------------------------

        //blur bright fragments with two-pass Gaussian Blur
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 12;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            blurShader.setInt("SCR_WIDTH", SCR_WIDTH);
            blurShader.setInt("SCR_HEIGHT", SCR_HEIGHT);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        finalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, pingpongColorbuffers[!horizontal]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
        finalShader.setInt("SCR_WIDTH", SCR_WIDTH);
        finalShader.setInt("SCR_HEIGHT", SCR_HEIGHT);

        finalShader.setInt("hdr", hdr);
        finalShader.setInt("bloom", bloom);
        finalShader.setFloat("exposure", exposure);
        finalShader.setInt("gammaEnabled",gammaEnabled);
        finalShader.setInt("grayscale", grayscale);
        renderQuad();

        std::cout << "hdr: " << (hdr ? "on" : "off") << std::endl;
        std::cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << std::endl;
        std::cout << (gammaEnabled ? "Gamma enabled" : "Gamma disabled") << std::endl;
//----------------------------------------------------------

        if(programState->ImGuiEnabled)
            DrawImGui(programState);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToDisk(FileSystem::getPath("resources/programState.txt"));

    //Imgui CleanUp
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    delete programState;

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
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    //gamma correction
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !gammaKeyPressed){
        gammaEnabled = !gammaEnabled;
        gammaKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE){
        gammaKeyPressed = false;
    }


    //Blinn-Phong
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed){
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE){
        blinnKeyPressed = false;
    }

    //HDR
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hdrKeyPressed){
        hdr = !hdr;
        hdrKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE){
        hdrKeyPressed = false;
    }

    //Bloom
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed){
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE){
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
        exposure += 0.001f;
    }

    //post-processing grayscale
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && !grayscaleKeyPressed){
        grayscale = !grayscale;
        grayscaleKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE){
        grayscaleKeyPressed= false;
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

    if(!programState->ImGuiEnabled){
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
    }

}
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){

    programState->camera.ProcessMouseScroll(yoffset);
}

//loading 2D textures
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data){
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

// renderQuad
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(){
    if (quadVAO == 0){
        float quadVertices[] = {
                // positions            // texCoords
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,

                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
        };

        //setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

//light cubes
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube(){
    // initialize (if necessary)
    if (cubeVAO == 0){
        float vertices[] = {
                //position                      //normals                           //texture coords
                // back face
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
                -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
                -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
                // front face
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
                -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
                -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
                // left face
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
                -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
                // right face
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
                1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
                1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
                // bottom face
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
                -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
                -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
                // top face
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
                1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
                -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
                -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

//Normal-Parallax mapping for ground
unsigned int groundVAO = 0;
unsigned int groundVBO;
void renderGround(){

    if (groundVAO == 0){
        // positions
        glm::vec3 pos1(60.0f,  -0.8f, 60.0f);
        glm::vec3 pos2(60.0f, -0.8f, -60.0f);
        glm::vec3 pos3( -60.0f, -0.8f, -60.0f);
        glm::vec3 pos4( -60.0f,  -0.8f, 60.0f);
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

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

//ImGui
void DrawImGui(ProgramState* programState){

    //ImGui Frame Init
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {

        ImGui::Begin("General settings:");
        ImGui::Text("Welcome to Tatooine!");
        ImGui::Checkbox("Camera info", &cameraInfo);

        ImGui::Bullet();
        ImGui::Checkbox("Grayscale (shortcut: X)", &grayscale);


        ImGui::Bullet();
        ImGui::Checkbox("Blinn-Phong (shortcut: B)", &blinn);

        ImGui::Bullet();
        ImGui::Checkbox("Gamma Correction (shortcut: C)", &gammaEnabled);
        ImGui::SameLine();
        Help("Better without gamma correction!");

        ImGui::Bullet();
        ImGui::Checkbox("HDR (shortcut: H)", &hdr);
        ImGui::SameLine();
        ImGui::Bullet();
        ImGui::Checkbox("Bloom (shortcut: SPACE)", &bloom);

        ImGui::Spacing();
        ImGui::Bullet();
        ImGui::DragFloat("Exposure", &exposure, 0.05f, 0.13f, 2.0f);
        ImGui::SameLine();
        Help("Use Q and E to decrease/increase exposure level");

        ImGui::Bullet();
        ImGui::DragFloat("Sphere rotation speed", (float*)&programState->angular_speed, 0.05f, 0.0f, 3.0f);

        ImGui::End();
    }

    if(cameraInfo){
        ImGui::Begin("Camera settings:");
        ImGui::Text("Camera Info:");

        ImGui::Indent();
        ImGui::Bullet();
        ImGui::Text("Camera position: (%f, %f, %f)", programState->camera.Position.x, programState->camera.Position.y, programState->camera.Position.z);

        ImGui::Bullet();
        ImGui::Text("(Yaw, Pitch): (%f, %f)", programState->camera.Yaw, programState->camera.Pitch);

        ImGui::Bullet();
        ImGui::Text("Camera front: (%f, %f, %f)", programState->camera.Front.x, programState->camera.Front.y, programState->camera.Front.z);

        if (ImGui::Button("Close Me"))
            cameraInfo = false;

        ImGui::End();
    }


    //ImGui render
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void keycallback(GLFWwindow* window, int key, int scanode, int action, int mods){
    if(key == GLFW_KEY_G && action == GLFW_PRESS){

        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if(programState->ImGuiEnabled)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    }
}

static void Help(const char* desc, bool extraText)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        if(extraText){
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 0.7f), "`What did that sign say again?`");
        }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
