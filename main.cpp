#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <vector>

const int WIDTH  = 1280;
const int HEIGHT = 720;

bool isSpacePressed = false;

// ---------------------------------------------------------------------
// ШЕЙДЕРЛЕР
// ---------------------------------------------------------------------
const char* vertexSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main() { 
    gl_Position = vec4(aPos, 1.0); 
}
)";

// Түсті сырттан (Uniform арқылы) береміз — бұл ортадағы объектінің 
// түсін фоннан тәуелсіз тұрақты етуге мүмкіндік береді.
const char* fragmentSrc = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 uColor;

void main() { 
    FragColor = uColor; 
}
)";

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        isSpacePressed = true;
    } else {
        isSpacePressed = false;
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Квадрат торшалар мен диагональдар", nullptr, nullptr);
    if (!window) {
        std::cerr << "Терезе жасалмады.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(0);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    // Шейдерлерді компиляциялау
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, nullptr);
    glCompileShader(fs);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int colorLoc = glGetUniformLocation(shaderProgram, "uColor");

    // -----------------------------------------------------------------
    // ТОРШАЛАРДЫ ДИНАМИКАЛЫҚ ТҮРДЕ ГЕНЕРАЦИЯЛАУ (4x4 Квадраттар)
    // -----------------------------------------------------------------
    std::vector<float> fillVertices; // Боялатын жартысы (үшбұрыштар)
    std::vector<float> lineVertices; // Бояусыз жартысы мен торша контуры

    int gridCount = 4;        // 4x4 торша
    float startX = -0.4f;     // Квадраттың сол жақ шеті
    float startY = -0.4f;     // Квадраттың төменгі шеті
    float totalSize = 0.8f;   // Жалпы өлшемі
    float step = totalSize / gridCount;

    for (int i = 0; i < gridCount; ++i) {
        for (int j = 0; j < gridCount; ++j) {
            float x1 = startX + i * step;
            float x2 = x1 + step;
            float y1 = startY + j * step;
            float y2 = y1 + step;

            // 1. Боялатын жартысы (төменгі-сол үшбұрыш)
            fillVertices.push_back(x1); fillVertices.push_back(y1); fillVertices.push_back(0.0f);
            fillVertices.push_back(x2); fillVertices.push_back(y1); fillVertices.push_back(0.0f);
            fillVertices.push_back(x1); fillVertices.push_back(y2); fillVertices.push_back(0.0f);

            // 2. Торшаның сыртқы контуры мен диагональ сызығы
            // Квадрат контуры
            lineVertices.push_back(x1); lineVertices.push_back(y1); lineVertices.push_back(0.0f);
            lineVertices.push_back(x2); lineVertices.push_back(y1); lineVertices.push_back(0.0f);

            lineVertices.push_back(x2); lineVertices.push_back(y1); lineVertices.push_back(0.0f);
            lineVertices.push_back(x2); lineVertices.push_back(y2); lineVertices.push_back(0.0f);

            lineVertices.push_back(x2); lineVertices.push_back(y2); lineVertices.push_back(0.0f);
            lineVertices.push_back(x1); lineVertices.push_back(y2); lineVertices.push_back(0.0f);

            lineVertices.push_back(x1); lineVertices.push_back(y2); lineVertices.push_back(0.0f);
            lineVertices.push_back(x1); lineVertices.push_back(y1); lineVertices.push_back(0.0f);

            // Диагональ сызығы (қиғашынан бөлу)
            lineVertices.push_back(x2); lineVertices.push_back(y1); lineVertices.push_back(0.0f);
            lineVertices.push_back(x1); lineVertices.push_back(y2); lineVertices.push_back(0.0f);
        }
    }

    // VAO / VBO — Боялатын үшбұрыштар үшін
    unsigned int fillVAO, fillVBO;
    glGenVertexArrays(1, &fillVAO);
    glGenBuffers(1, &fillVBO);
    glBindVertexArray(fillVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fillVBO);
    glBufferData(GL_ARRAY_BUFFER, fillVertices.size() * sizeof(float), fillVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // VAO / VBO — Сызықтар/контурлар үшін
    unsigned int lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    double lastTime = glfwGetTime();
    int frameCount = 0;

    // -----------------------------------------------------------------
    // НЕГІЗГІ ЦИКЛ
    // -----------------------------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // FPS есептеу
        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - lastTime >= 1.0) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastTime += 1.0;
        }

        // 1. ФОННЫҢ ТҮСІ БҰРЫНҒЫША ӨЗГЕРЕДІ НЕМЕСЕ СЕКУНД САЙЫН/ПРОБЕЛ АРҚЫЛЫ
        float t = (float)glfwGetTime();
        float r = (std::sin(t * 2.5f) + 1.0f) * 0.5f * 0.3f;
        float g = (std::sin(t * 1.8f) + 1.0f) * 0.5f * 0.3f;

        if (isSpacePressed) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Пробел басылғанда фон АҚ
        } else {
            glClearColor(r, g, 0.35f, 1.0f);      // Уақыт өте өзгеретін фон
        }
        glClear(GL_COLOR_BUFFER_BIT);

        // 2. ОРТАДАҒЫ ФИГУРАНЫ САЛУ (Оның түсі фонға тәуелсіз тұрақты)
        glUseProgram(shaderProgram);

        // А) Боялатын жарты үшбұрыштар (мысалы: Көгілдір / Turquoise түс)
        glUniform4f(colorLoc, 0.0f, 0.7f, 0.9f, 1.0f); 
        glBindVertexArray(fillVAO);
        glDrawArrays(GL_TRIANGLES, 0, (int)(fillVertices.size() / 3));

        // Б) Бояусыз жартының контуры мен торша сызықтары (Тұрақты қара түс)
        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f); 
        glBindVertexArray(lineVAO);
        glLineWidth(2.0f); // Сызық қалыңдығы
        glDrawArrays(GL_LINES, 0, (int)(lineVertices.size() / 3));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Тазалау
    glDeleteVertexArrays(1, &fillVAO);
    glDeleteBuffers(1, &fillVBO);
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}