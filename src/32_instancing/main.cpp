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
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(vector<std::string> faces);

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

Camera camera(glm::vec3(1.0f, 100.0f, 300.0f));

float deltaTime = 0.0f; // ��ǰ֡����һ֮֡���ʱ���
float lastTime = 0.0f;	// ��һ֡��ʱ��

float lastX = SCREEN_WIDTH / 2.0f; // �����һ֡��λ��
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

int main()
{

	// ��ʼ��GLFW
	glfwInit();

	// ����GLFW
	const char *glsl_version = "#version 330";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// ����GLFW���ڶ���
	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL", NULL, NULL);

	if (window == NULL)
	{
		cout << "��ʼ��glfw����ʧ��!" << endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// ���û���ģʽ
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// ��ʼ��GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "��ʼ��GLADʧ��!" << endl;
		return -1;
	}

	// �����ӿ�
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// ע�ⴰ�ڼ���
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// ��Ȳ���
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// ���
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// ���޳�
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // �޳��������
	glFrontFace(GL_CCW); //��˳ʱ����涨��Ϊ�������

	glEnable(GL_PROGRAM_POINT_SIZE);

	Shader screenShader("./src/32_instancing/framebuffer_vert.glsl", "./src/32_instancing/framebuffer_frag.glsl");
	Shader skyboxShader("./src/32_instancing/skybox_vert.glsl", "./src/32_instancing/skybox_frag.glsl");
	Shader shader("./src/32_instancing/model_vert.glsl", "./src/32_instancing/model_frag.glsl");
	Shader instanceShader("./src/32_instancing/instance_model_vert.glsl", "./src/32_instancing/instance_model_frag.glsl");

	float screenVertices[] = {
			// positions   // texCoords
			-1.0f, 1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f, 1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 1.0f, 1.0f};

	float skyboxVertices[] = {
			// positions
			-1.0f, 1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, -1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 1.0f,
			-1.0f, -1.0f, 1.0f,

			-1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, -1.0f,
			1.0f, 1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f, 1.0f,
			1.0f, -1.0f, 1.0f};

	float points[] = {
			-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, // ����
			0.5f, 0.5f, 0.0f, 1.0f, 0.0f,	 // ����
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // ����
			-0.5f, -0.5f, 1.0f, 1.0f, 0.0f // ����
	};

	// screen quad VAO
	unsigned int screenVAO, screenVBO;
	glGenVertexArrays(1, &screenVAO);
	glGenBuffers(1, &screenVBO);
	glBindVertexArray(screenVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), &screenVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

	// skyboxVBO quad VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

	// ����imgui������
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	vector<glm::vec3> windows{
			glm::vec3(-1.5f, 0.0f, -0.48f),
			glm::vec3(1.5f, 0.0f, 0.51f),
			glm::vec3(0.0f, 0.0f, 0.7f),
			glm::vec3(-0.3f, 0.0f, -2.3f),
			glm::vec3(0.5f, 0.0f, -0.6f)};

	// ����һ��֡������󲢰���
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// ����һ����ɫ�������������ӵ�֡������
	// ��������
	unsigned int texColorBuffer;
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// ����ɫ���Ӹ��ӵ���ǰ�󶨵�֡������
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	// ������Ⱦ�������
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// ����Ⱦ������󸽼ӵ�֡�������Ⱥ�ģ�帽����
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// ���֡�����Ƿ�����
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		cout << "ERROR:: FRAMEBUFFER :: framebuffer is not complete�� " << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	vector<string> faces{
			"./static/texture/skybox/right.jpg",
			"./static/texture/skybox/left.jpg",
			"./static/texture/skybox/top.jpg",
			"./static/texture/skybox/bottom.jpg",
			"./static/texture/skybox/front.jpg",
			"./static/texture/skybox/back.jpg"};

	unsigned int cubemapTexture = loadCubemap(faces);

	unsigned int amount = 100000;
	glm::mat4 *modelMtrices;
	modelMtrices = new glm::mat4[amount];
	srand(glfwGetTime());
	float radius = 150.0;
	float offset = 25.0;

	for (unsigned int i = 0; i < amount; i++)
	{

		glm::mat4 model = glm::mat4(1.0);

		// 1.λ�� : �ֲ��ڰ뾶Ϊ radius ��Բ���ϣ�ƫ�Ƶķ�Χ [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // �����Ǵ��ĸ߶ȱ�x��z��ҪС
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. ���� : �� 0.05f �� 0.25f ֮������
		float scale = (rand() % 20) / 100.0f + 0.05;
		model = glm::scale(model, glm::vec3(scale));

		// 3. ��ת : ����һ�����ѡ�����ת�����������������ת
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. ��ӵ������������
		modelMtrices[i] = model;
	}

	Model planetModel("./static/model/planet/planet.obj");
	Model rockModel("./static/model/rock/rock.obj");

	// ���㻺�����
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMtrices[0], GL_DYNAMIC_DRAW);

	for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
	{
		unsigned int VAO = rockModel.meshes[i].VAO;
		glBindVertexArray(VAO);
		// ��������
		GLsizei vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

	//��Ⱦѭ��
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastTime;
		lastTime = currentFrame;

		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// ����
		map<float, glm::vec3> sorted;
		for (unsigned int i = 0; i < windows.size(); i++)
		{
			float distance = glm::length(camera.Position - windows[i]);
			sorted[distance] = windows[i];
		}

		// ��֡����
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// ����Ƿ���Ҫ�˳�����
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		ImGui::Begin("view value");
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();

		glCullFace(GL_FRONT);

		// ��������
		shader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		shader.setMat4("model", model);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), float(SCREEN_WIDTH) / float(SCREEN_HEIGHT), 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		planetModel.Draw(shader);

		// ����С����
		/*for (unsigned int i = 0; i < amount; i++) {
		model = glm::mat4(1.0f);
		model = glm::rotate(model, (float)glm::radians(glfwGetTime() * 2.0), glm::vec3(0.0f, -3.0f, 0.0f));
		model = model * modelMtrices[i];
		model = glm::rotate(model, (float)glm::radians(glfwGetTime() * (rand() % 360)), glm::vec3(0.0f, -3.0f, 0.0f));
		shader.setMat4("model", model);
		rockModel.Draw(shader);
		}*/
		instanceShader.use();
		instanceShader.setMat4("projection", projection);
		instanceShader.setMat4("view", view);
		for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
		{
			glBindVertexArray(rockModel.meshes[i].VAO);
			glDrawElementsInstanced(GL_TRIANGLES, rockModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
		}

		glDepthFunc(GL_LEQUAL);
		glDisable(GL_CULL_FACE);

		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		screenShader.use();
		screenShader.setFloat("offset", 1 / 300.0);

		glBindVertexArray(screenVAO);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// ������ɫ������
		glfwSwapBuffers(window);

		// ����Ƿ��������¼�
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &screenVAO);
	glDeleteVertexArrays(1, &skyboxVAO);

	glDeleteBuffers(1, &screenVBO);
	glDeleteBuffers(1, &skyboxVBO);

	// ɾ��/�ͷ���Դ
	glfwTerminate();
	return 0;
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

// ���ڴ�С�任����
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// �������
void processInput(GLFWwindow *window)

{
	float cameraSpeed = deltaTime * 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}

unsigned int loadTexture(char const *path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	//stbi_set_flip_vertically_on_load(true);
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