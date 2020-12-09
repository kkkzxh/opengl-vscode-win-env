#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// 顶点着色器小程序 对应OpenGL版本 3.3
const char *vertexShaderSuorce =
		"#version 330 core\n"										// 对应使用核心版本
		"layout (location = 0) in vec3 aPos;\n" // 声明输入顶点属性
		"void main()\n"
		"{\n"
		" vec4 v4Pos = vec4(aPos.xy, 0.0, 1.0);\n"
		" gl_Position = v4Pos;\n" // 内置变量 gl_Position
		"}";

// 片元着色器小程序 对应版本3.3 核心模式
const char *fragementShaderSource =
		"#version 330 core\n"
		"out vec4 FragColor;\n" // 输出变量 最终输出的颜色
		"void main()\n"
		"{"
		"	FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
		"}";

const char *fragementShaderSource2 =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{"
		" 	//FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
		"}";

int main()
{

	// 初始化GLFW
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建GLFW窗口对象
	GLFWwindow *window = glfwCreateWindow(800, 600, "demo01", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "初始化glfw窗口失败!" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// 初始化GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "初始化GLAD失败!" << std::endl;
		return -1;
	}

	// 设置视口
	glViewport(0, 0, 800, 600);

	// 注测窗口监听
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 顶点数组
	float vertices1[] = {
			// 第一个三角形
			0.5f,
			0.75f,
			0.0f,
			0.25f,
			0.25f,
			0.0f,
			0.75f,
			0.25f,
			0.0f,
	};

	float vertices2[] = {
			// 第二个三角形
			-0.5f,
			-0.25f,
			0.0f,
			-0.75f,
			-0.75f,
			0.0f,
			-0.25f,
			-0.75f,
			0.0f,
	};

	// 生成VBO对象
	unsigned int VBO1, VBO2;
	glGenBuffers(1, &VBO1);
	glGenBuffers(1, &VBO2);

	// 创建顶点数组对象
	unsigned int VAO1, VAO2;
	glGenVertexArrays(1, &VAO1);
	glGenVertexArrays(1, &VAO2);
	// 绑定VAO
	glBindVertexArray(VAO1);
	// 绑定缓冲到目标缓冲
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	// 启用顶点属性
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO2);
	// 绑定缓冲到目标缓冲
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	// 传递数据到缓冲
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	// 启用顶点属性
	glEnableVertexAttribArray(0);

	// 编译着色器
	unsigned int vertexShader;
	unsigned int fragmentShader;
	unsigned int fragementShader2;

	// 创建着色器对象
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragementShader2 = glCreateShader(GL_FRAGMENT_SHADER);

	// 附加顶点着色器源码到着色器对象上
	glShaderSource(vertexShader, 1, &vertexShaderSuorce, NULL);
	glCompileShader(vertexShader);

	// 附加片元着色器源码到着色对象上
	glShaderSource(fragmentShader, 1, &fragementShaderSource, NULL);
	// 编译着色器器
	glCompileShader(fragmentShader);

	glShaderSource(fragementShader2, 1, &fragementShaderSource2, NULL);
	glCompileShader(fragementShader2);

	// 检测是否成功
	int success;			 // 检测是否成功
	char infoLog[512]; // 存储错误信息
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	// 如果编译失败打印错误信息
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "顶点着色器编译失败！" << infoLog << std::endl;
	}

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "片元着色器编译失败！" << infoLog << std::endl;
	}

	glGetShaderiv(fragementShader2, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragementShader2, 512, NULL, infoLog);
		std::cout << "片元着色器2 编译失败！" << infoLog << std::endl;
	}

	// 2 着色器程序对象
	unsigned int shaderProgram, shaderProgram2;
	shaderProgram = glCreateProgram();
	shaderProgram2 = glCreateProgram();

	// 将编译的着色器附加到程序对象上
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glAttachShader(shaderProgram2, vertexShader);
	glAttachShader(shaderProgram2, fragementShader2);

	// 链接着色器
	glLinkProgram(shaderProgram);
	glLinkProgram(shaderProgram2);

	glUseProgram(shaderProgram);

	// 查询属性的位置
	int aPos = glGetAttribLocation(shaderProgram, "aPos");
	std::cout << "aPos 的 location 是：" << aPos << std::endl;

	// 检测链接是否失败
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "着色器程序对象链接失败！" << infoLog << std::endl;
	}

	glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
		std::cout << "着色器程序对象2 链接失败！" << infoLog << std::endl;
	}

	// 3 使用着色器程序
	// glUseProgram(shaderProgram);

	// 4 删除着色器对象
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(fragementShader2);

	// 5 解析顶点数据
	// 参数1 location
	// 参数2 顶点属性的大小
	// 参数3 数据类型
	// 参数4 是否数据标准化
	// 参数5 步长
	// 参数6 起始位置的偏移量
	// glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	// // 启用顶点属性
	// glEnableVertexAttribArray(0);

	// 绑定buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// 使用现况绘制模式
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// 获取可以支持的顶点属性的数量
	int numAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);

	std::cout << "所能支持的顶点属性的最大数量是： " << numAttributes << std::endl;

	//渲染循环
	while (!glfwWindowShouldClose(window))
	{

		// 检测是否需要退出窗口
		processInput(window);

		// 渲染指令

		// 设置清除颜色 -- 状态设置
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		//开始清空屏幕并且使用上面的颜色 -- 状态使用
		glClear(GL_COLOR_BUFFER_BIT);

		// 绘制物体
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO1);
		glDrawArrays(GL_TRIANGLES, 0, (sizeof(vertices1) / sizeof(vertices1[0])) / 3);

		//glUseProgram(shaderProgram2);
		//glBindVertexArray(VAO2);
		//glDrawArrays(GL_TRIANGLES, 0, (sizeof(vertices2) / sizeof(vertices2[0])) / 3);
		//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

		// 参数1 图元类型
		// 参数2 顶点数组的起始索引
		// 参数3 绘制的顶点个数
		// glDrawArrays(GL_LINE_LOOP, 0, 3);

		// 交换颜色缓冲区
		glfwSwapBuffers(window);

		// 检查是否出发相关事件
		glfwPollEvents();
	}

	// 删除相关资源
	glDeleteBuffers(1, &VBO1);
	glDeleteBuffers(1, &VBO2);
	glDeleteVertexArrays(1, &VAO1);
	glDeleteVertexArrays(1, &VAO2);

	glDeleteProgram(shaderProgram);
	glDeleteProgram(shaderProgram2);

	// 删除/释放资源
	glfwTerminate();
	return 0;
}

// 窗口大小变换监听
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// 输入监听
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
}