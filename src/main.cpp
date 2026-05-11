#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <fstream>
#include <string>

#include "ourshader/Shader.h"
#include "Model/model.h"
#include "camera/camera.h"

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
std::string resolvePath(const std::string& path);

Camera camera(glm::vec3(0.0f, 1.5f, 5.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assimp FBX Test", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 뒷면이 사라지는 문제 확인 전까지는 일단 끄는 걸 추천합니다.
    // glEnable(GL_CULL_FACE);

    std::string vertexShaderPath = resolvePath("shaders/shader.vs");
    std::string fragmentShaderPath = resolvePath("shaders/shader.fs");
    std::string modelPath = resolvePath("resource/Models/CleGLTF.glb");


    Shader shader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

    // FBX 경로
    Model testModel(modelPath);
    glm::vec3 modelCenter = (testModel.minBounds + testModel.maxBounds) * 0.5f;
    glm::vec3 modelSize = testModel.maxBounds - testModel.minBounds;
    float maxSize = glm::max(modelSize.x, glm::max(modelSize.y, modelSize.z));
    float modelScale = maxSize > 0.0f ? 2.0f / maxSize : 1.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );

        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::scale(model, glm::vec3(modelScale));
        model = glm::translate(model, -modelCenter);

        // 방향이 누워있으면 회전
        //model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        shader.setVec3("lightDir", glm::vec3(-0.3f, -1.0f, -0.5f));
        shader.setVec3("lightColor", glm::vec3(1.0f, 0.96f, 0.88f));
        shader.setVec3("viewPos", camera.Position);
        shader.setBool("renderOutline", false);

        testModel.Draw(shader);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glDepthFunc(GL_LEQUAL);

        shader.setBool("renderOutline", true);
        shader.setFloat("outlineScale", 0.001f);
        shader.setVec3("outlineColor", glm::vec3(0.02f, 0.018f, 0.015f));

        testModel.Draw(shader);

        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
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

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessScroll(static_cast<float>(yoffset));
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

std::string resolvePath(const std::string& path)
{
    std::ifstream direct(path);
    if (direct.good())
    {
        return path;
    }

    std::string parentPath = "../" + path;
    std::ifstream parent(parentPath);
    if (parent.good())
    {
        return parentPath;
    }

    return path;
}
