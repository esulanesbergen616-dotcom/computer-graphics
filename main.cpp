#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH  = 1280;
const int HEIGHT = 720;

// ---------------------------------------------------------------------
// 3-ДӘРІС: Шейдерлерді компиляциялау және қатені тексеру функциясы
// ---------------------------------------------------------------------
unsigned int makeShader(const char* vsSrc, const char* fsSrc) {
    int ok;
    char log[1024];

    // Vertex Shader
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(vs, 1024, nullptr, log);
        std::cerr << "--- VERTEX SHADER COMPILE ERROR ---\n" << log << std::endl;
    }

    // Fragment Shader
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(fs, 1024, nullptr, log);
        std::cerr << "--- FRAGMENT SHADER COMPILE ERROR ---\n" << log << std::endl;
    }

    // Program Linking
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "--- SHADER PROGRAM LINK ERROR ---\n" << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ---------------------------------------------------------------------
// ШЕЙДЕРЛЕР (GLSL)
// ---------------------------------------------------------------------
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform vec2 uOffset;
uniform float uScale;

out vec3 vColor;

void main() {
    // Тапсырма 4: Скейл және ығысу қолдану
    vec2 scaledPos = aPos.xy * uScale;
    gl_Position = vec4(scaledPos + uOffset, aPos.z, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    // 3-дәріс: Градиентті түсті шығару
    FragColor = vec4(vColor, 1.0);
}
)";

// ---------------------------------------------------------------------
// 4-ДӘРІС: Батырмалармен жылдамдықты басқару (W/S)
// ---------------------------------------------------------------------
void processInput(GLFWwindow* window, float dt, float& speed) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // W - жылдамдықты арттыру, S - азайту (dt қолданылған)
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        speed += 2.0f * dt;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        speed -= 2.0f * dt;
    }
}

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Seminar 3 & 4 - OpenGL", nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1); // VSync

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    // 1. Шейдер программасын жасау
    unsigned int shaderProgram = makeShader(vertexShaderSource, fragmentShaderSource);

    // 2. 3-ДӘРІС: Төбелер дерегі (Position: 3 float, Color: 3 float)
    float vertices[] = {
        //  X,      Y,     Z,      R,    G,    B
        -0.2f, -0.2f,  0.0f,   1.0f, 0.0f, 0.0f,  // Қызыл бұрыш
         0.2f, -0.2f,  0.0f,   0.0f, 1.0f, 0.0f,  // Жасыл бұрыш
         0.0f,  0.2f,  0.0f,   0.0f, 0.0f, 1.0f   // Көк бұрыш
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Location = 0 (Position), Stride = 6 * sizeof(float)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Location = 1 (Color), Stride = 6 * sizeof(float), Offset = 3 * sizeof(float)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 3. 4-ДӘРІС: Uniform орнын кэштеу (циклге дейін)
    int locOffset = glGetUniformLocation(shaderProgram, "uOffset");
    int locScale  = glGetUniformLocation(shaderProgram, "uScale");

    // Анимация айнымалылары
    float lastFrame  = (float)glfwGetTime(); // Нөл емес!
    float angle      = 0.0f;
    float speed      = 1.5f; // радиан / секунд
    float scaleTimer = 0.0f;

    // -----------------------------------------------------------------
    // НЕГІЗГІ РЕНДЕР ЦИКЛІ
    // -----------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        // --- 1. УАҚЫТ (Delta Time есептеу) ---
        float now = (float)glfwGetTime();
        float dt  = now - lastFrame;
        lastFrame = now;

        // --- 2. ЖАҢАРТУ (Күйді есептеу) ---
        processInput(window, dt, speed);

        // Бұрышты жаңарту (dt қолданылған)
        angle += speed * dt;
        scaleTimer += dt;

        // Пульсация эффектісі (uScale)
        float currentScale = 1.0f + 0.25f * std::sin(scaleTimer * 4.0f);

        // --- 3. СЫЗУ (Күйді көрсету) ---
        glClearColor(0.1f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // НАЗАР: glUseProgram бірінші шақырылуы тиіс!
        glUseProgram(shaderProgram);

        // Uniform-ға мәндерді жіберу
        glUniform2f(locOffset, std::cos(angle) * 0.4f, std::sin(angle) * 0.4f);
        glUniform1f(locScale, currentScale);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Тазалау
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}