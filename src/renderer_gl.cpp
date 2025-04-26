const char *vertexSource =
        "#version 460 core\n"
        "layout (location = 0) in vec3 aPos;"
        "layout(std430, binding = 0) buffer objectBuffer"
        "{"
            "mat4 models[];"
        "};"

        "uniform mat4 view;"
        "uniform mat4 projection;"

        "void main()"
        "{"
        "  gl_Position = projection * view * models[gl_InstanceID + gl_BaseInstance] * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
        "}";

const char *fragmentSource =
        "#version 460 core\n"
        "out vec4 fragColor;"

        "void main()"
        "{"
        "  fragColor = vec4(0.2f, 1.0f, 0.4f, 1.0f);"
        "}";

struct Mesh
{
    GLuint vertArray;
    GLuint vertBuffer;
    GLuint elemBuffer;

    Mesh(u32 vertCount, f32* vertices, u32 indexCount, u32* indices)
    {
        glGenVertexArrays(1, &vertArray);
        glBindVertexArray(vertArray);

        glGenBuffers(1, &vertBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertBuffer);
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(f32) * vertCount, vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &elemBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * indexCount, indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(f32), (void *) 0);
        glEnableVertexAttribArray(0);
    }

    ~Mesh()
    {
        glDeleteBuffers(1, &vertBuffer);
        glDeleteBuffers(1, &elemBuffer);
        glDeleteVertexArrays(1, &vertArray);
    }
};

GLuint objectBuffer;
GLuint shaderProgram;


void InitRenderer()
{
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    GLint vertSuccess;
    GLchar vertInfoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertSuccess);
    if (!vertSuccess)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, vertInfoLog);
        std::cout << "Vertex shader failed to compile: " << vertInfoLog;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    GLint fragSuccess;
    GLchar fragInfoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragSuccess);
    if (!fragSuccess)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragInfoLog);
        std::cout << "Vertex shader failed to compile: " << fragInfoLog;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGenBuffers(1, &objectBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, objectBuffer);

    glUseProgram(shaderProgram);
}

void InitFrame()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

void SetCamera(glm::mat4 view, glm::mat4 proj)
{
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));
}

void SendModelMatrices(std::vector<glm::mat4>& modelMatrices)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * modelMatrices.size(), modelMatrices.data(), GL_DYNAMIC_DRAW);
}

void SetMesh(Mesh* mesh)
{
    glBindVertexArray(mesh->vertArray);
}

void DrawObject(int index)
{
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 1, index);
}

void DrawObjects(int count, int startIndex)
{
    glDrawElementsInstancedBaseInstance(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, count, startIndex);
}