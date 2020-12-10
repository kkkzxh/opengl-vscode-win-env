#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <map>

#include <tool/shader.h>
#include <tool/camera.h>
#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>
#include <tool/gui.h>

#include <tool/mesh.h>
#include <tool/model.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int loadTexture(char const *path, bool gammaCorrection);
unsigned int loadCubemap(vector<std::string> faces);

void renderQuad();
void renderCube();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 3.0f, 5.0f));

float deltaTime = 0.0f; // 当前帧与上一帧之间的时间差
float lastTime = 0.0f;  // 上一帧的时间

float lastX = SCR_WIDTH / 2.0f; // 鼠标上一帧的位置
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// bloom 设置
bool bloomKeyPressed = false;
bool bloom = true;
float exposure = 1.0;

int main()
{

  // 初始化GLFW
  glfwInit();

  // 配置GLFW
  // --------
  const char *glsl_version = "#version 330";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // 创建GLFW窗口对象
  // ----------------
  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

  if (window == NULL)
  {
    cout << "初始化glfw窗口失败!" << endl;
    glfwTerminate();
    return -1;
  }

  // 注测窗口监听
  // ------------
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // 初始化GLAD
  // ----------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    cout << "初始化GLAD失败!" << endl;
    return -1;
  }

  // 创建imgui上下文,需要在glad初始化之后
  // ---------------
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // 深度测试
  // --------
  glEnable(GL_DEPTH_TEST);

  // 启用gamma校正
  //glEnable(GL_FRAMEBUFFER_SRGB);

  // 编译shader
  // ----------
  Shader shader("./src/42_deferred_shading/shader/cube_vert.glsl", "./src/42_deferred_shading/shader/cube_frag.glsl");
  Shader shaderGeometry("./src/42_deferred_shading/shader/deferred_geometry_vert.glsl", "./src/42_deferred_shading/shader/deferred_geometry_frag.glsl");
  Shader shaderRender("./src/42_deferred_shading/shader/deferred_render_light_vert.glsl", "./src/42_deferred_shading/shader/deferred_render_light_frag.glsl");

  // 加载贴图
  // --------

  // 加载模型
  // -------
  Model nanosuitModel("./static/model/nanosuit/nanosuit.obj");
  vector<glm::vec3> modelPositions;
  modelPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
  modelPositions.push_back(glm::vec3(0.0, -0.5, -3.0));
  modelPositions.push_back(glm::vec3(3.0, -0.5, -3.0));
  modelPositions.push_back(glm::vec3(-3.0, -0.5, 0.0));
  modelPositions.push_back(glm::vec3(0.0, -0.5, 0.0));
  modelPositions.push_back(glm::vec3(3.0, -0.5, 0.0));
  modelPositions.push_back(glm::vec3(-3.0, -0.5, 3.0));
  modelPositions.push_back(glm::vec3(0.0, -0.5, 3.0));
  modelPositions.push_back(glm::vec3(3.0, -0.5, 3.0));

  // 配置 G-Buffer 缓冲区
  // ---------
  GLuint gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  GLuint gPosition, gNormal, gColorSpec;

  // - 位置颜色缓冲
  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // - 法线颜色缓冲
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // - 颜色和镜面颜色缓冲
  glGenTextures(1, &gColorSpec);
  glBindTexture(GL_TEXTURE_2D, gColorSpec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gColorSpec, 0);

  // - 告诉OpenGL我们要使用（帧缓冲的）那种颜色附件来进行渲染
  GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  // 创建并附加到深度缓冲区
  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT, GL_RENDERBUFFER, rboDepth);

  // 检查framebuffer 是否编译成功
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    cout << "Framebuffer 编译失败！" << endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // 灯光设置
  // -------
  const unsigned int NR_LIGHTS = 32;
  vector<glm::vec3> lightPositions;
  vector<glm::vec3> lightColors;

  srand(13);
  for (unsigned int i = 0; i < NR_LIGHTS; i++)
  {
    // 计算随机偏移
    float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
    float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
    float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
    lightPositions.push_back(glm::vec3(xPos, yPos, zPos));

    // 计算随机颜色
    float rColor = ((rand() % 100) / 200.0) + 0.5;
    float gColor = ((rand() % 100) / 200.0) + 0.5;
    float bColor = ((rand() % 100) / 200.0) + 0.5;
    lightColors.push_back(glm::vec3(rColor, gColor, bColor));
  }

  // 设置shader
  // -----------------
  shaderRender.use();
  shaderRender.setInt("gPosition", 0);
  shaderRender.setInt("gNormal", 1);
  shaderRender.setInt("gColorSpec", 2);

  // 渲染循环
  // --------
  while (!glfwWindowShouldClose(window))
  {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastTime;
    lastTime = currentFrame;

    // input
    // -----
    processInput(window);

    // imgui
    // -----
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("view value");
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // render
    // ------
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. Geometry pass 将场景的几何数据和颜色数据渲染到Gbuffer
    // -----------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shaderGeometry.use();
    shaderGeometry.setMat4("projection", projection);
    shaderGeometry.setMat4("view", view);
    for (unsigned int i = 0; i < modelPositions.size(); i++)
    {
      model = glm::mat4(1.0f);
      model = glm::translate(model, modelPositions[i]);
      model = glm::scale(model, glm::vec3(0.25f));
      shaderGeometry.setMat4("model", model);
      nanosuitModel.Draw(shaderGeometry);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    renderCube();
    // 2. Lighting pass 通过使用gbuffer的内容逐像素的遍历四分之一的屏幕来迭代计算照明
    // -------------------------------------------------------------------------
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderRender.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gColorSpec);

    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
      shaderRender.setVec3("lights[" + to_string(i) + "].Position", lightPositions[i]);
      shaderRender.setVec3("lights[" + to_string(i) + "].Color", lightColors[i]);
      const float constant = 1.0;
      const float linear = 0.7;
      const float quadratic = 1.8;

      shaderRender.setFloat("lights[" + to_string(i) + "].Linear", linear);
      shaderRender.setFloat("lights[" + to_string(i) + "].Quadratic", quadratic);
    }
    shaderRender.setVec3("viewPos", camera.Position);

    renderQuad();

    // 渲染立方体
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    for (unsigned int i = 0; i < lightPositions.size(); i++)
    {
      model = glm::mat4(1.0);
      model = glm::translate(model, lightPositions[i]);
      model = glm::scale(model, glm::vec3(0.45));
      shader.setVec3("lightColor", lightColors[i]);
      shader.setMat4("model", model);
      renderCube();
    }

    //cout << "bloom: " << (bloom ? "on" : "off") << " | exposure: " << exposure << endl;

    // ImGui
    // -----
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // glfw 交换颜色缓冲区和轮询IO事件
    // ------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // 删除/释放资源
  glfwTerminate();
  return 0;
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
  // initialize (if necessary)
  if (cubeVAO == 0)
  {
    float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
        // front face
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
        // left face
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
        // right face
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-right
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-left
        // bottom face
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
        // top face
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  // render Cube
  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
  if (quadVAO == 0)
  {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        -1.0f,
        -1.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        0.0f,
        1.0f,
        0.0f,
    };
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
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

// 窗口大小变换监听
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

// 输入监听
void processInput(GLFWwindow *window)
{
  // 退出窗口
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window, true);
  }

  // 相机移动
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
  {
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
  {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
  {
    camera.ProcessKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
  {
    camera.ProcessKeyboard(RIGHT, deltaTime);
  }

  // 切换bloom
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
  {
    bloom = !bloom;
    bloomKeyPressed = true;
  }
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
  {
    bloomKeyPressed = false;
  }

  // 曝光度
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
  {
    if (exposure > 0.0f)
      exposure -= 0.001f;
    else
      exposure = 0.0f;
  }
  else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
  {
    exposure += 0.001f;
  }

  // 将鼠标指针从窗口中施放
  if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
  {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}

unsigned int loadTexture(char const *path, bool gammaCorrection)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  //stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum internalFormat;
    GLenum dataFormat;

    if (nrComponents == 1)
    {
      internalFormat = dataFormat = GL_RED;
    }
    else if (nrComponents == 3)
    {
      internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
      dataFormat = GL_RGB;
    }
    else if (nrComponents == 4)
    {
      internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
      dataFormat = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
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
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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