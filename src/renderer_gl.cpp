f32 vertices[] = 
{
  -0.5f, -0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f, 0.5f, 0.0f,
  0.5f, 0.5f, 0.0f
};

u32 indices[] = 
{
  0, 1, 2,
  1, 2, 3
};

const char* vertexSource =
"#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}"

const char* fragmentSource = 
"#version 460 core
out vec4 fragColor;

void main()
{
  fragColor = vec4(0.2f, 1.0f, 0.4f, 1.0f);
}"

unsigned int VBO;
unsigned int EBO;
unsigned int shaderProgram;
unsigned int VAO;

void InitRenderer()
{
  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexSource, NULL);
  glCompileShader(vertexShader);
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "Vertex shader failed to compile: " << infoLog;
  }

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShader);
  int success;
  char infoLog[512];
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    std::cout << "Vertex shader failed to compile: " << infoLog;
  }

  shaderProgam = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);

  glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
}