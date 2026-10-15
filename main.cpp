#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <vector>


const int WIDTH  = 1280;
const int HEIGHT = 960;


bool isPaused = false;
bool prevSpaceState = false;


void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}


unsigned int makeShader(const char* vsSrc, const char* fsSrc) {
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSrc, nullptr);
    glCompileShader(vs);
    int ok;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(vs, 1024, nullptr, log);
        std::cerr << "Vertex Shader қатесі:\n" << log << "\n";
    }

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSrc, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(fs, 1024, nullptr, log);
        std::cerr << "Fragment Shader қатесі:\n" << log << "\n";
    }

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, 1024, nullptr, log);
        std::cerr << "Shader Program линковка қатесі:\n" << log << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

const char* vertexShaderSrc = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 uModel;

out vec3 vColor;

void main() {
    gl_Position = uModel * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

int main() {

    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2D Scene - Right to Left Square Movement", nullptr, nullptr);
    if (!window) {
        std::cerr << "OpenGL терезесі жасалмады!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1); 


    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді!\n";
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL сәтті іске қосылды!\n";
    std::cout << "Басқару: Space - Ойынды тоқтату/жалғастыру, ESC - Шығу.\n";

    const int STRIDE = 6 * sizeof(float); // 3 позиция (x,y,z) + 3 түс (r,g,b)


    std::vector<float> circleVertices;
    std::vector<unsigned int> circleIndices;

    float radius = 0.10f; 
    int segments = 48;    

    circleVertices.push_back(0.0f); circleVertices.push_back(0.0f); circleVertices.push_back(0.0f);
    circleVertices.push_back(0.2f); circleVertices.push_back(0.9f); circleVertices.push_back(1.0f); 


    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159265f * float(i) / float(segments);
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle);

        circleVertices.push_back(x); circleVertices.push_back(y); circleVertices.push_back(0.0f);
        circleVertices.push_back(0.0f); circleVertices.push_back(0.4f); circleVertices.push_back(0.8f); 
    }


    for (int i = 0; i < segments; ++i) {
        circleIndices.push_back(0);
        circleIndices.push_back(i + 1);
        circleIndices.push_back(((i + 1) % segments) + 1);
    }

    unsigned int circleVAO, circleVBO, circleEBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glGenBuffers(1, &circleEBO);

    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, circleIndices.size() * sizeof(unsigned int), circleIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    float sqSize = 0.10f; 
    float squareVertices[] = {

        -sqSize,  sqSize, 0.0f,    0.9f, 0.2f, 0.8f, 
         sqSize,  sqSize, 0.0f,    1.0f, 0.4f, 0.3f, 
         sqSize, -sqSize, 0.0f,    0.7f, 0.1f, 0.6f, 
        -sqSize, -sqSize, 0.0f,    1.0f, 0.3f, 0.5f  
    };

    unsigned int squareIndices[] = {
        0, 1, 2, 
        0, 2, 3  
    };

    unsigned int squareVAO, squareVBO, squareEBO;
    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &squareEBO);

    glBindVertexArray(squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, STRIDE, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);


    unsigned int shaderProgram = makeShader(vertexShaderSrc, fragmentShaderSrc);
    int modelLoc = glGetUniformLocation(shaderProgram, "uModel");


    float ballX = 0.0f;          
    float ballBaseY = -0.55f;    
    float maxHeight = 0.65f;     
    float bouncePeriod = 1.8f;   

    float accumulatedTime = 0.0f;
    float lastRealTime = (float)glfwGetTime();


    while (!glfwWindowShouldClose(window)) {
        float currentRealTime = (float)glfwGetTime();
        float dt = currentRealTime - lastRealTime;
        lastRealTime = currentRealTime;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        bool spacePressed = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);
        if (spacePressed && !prevSpaceState) {
            isPaused = !isPaused;
            std::cout << (isPaused ? "Ойын тоқтатылды (Пауза)\n" : "Ойын жалғасуда...\n");
        }
        prevSpaceState = spacePressed;

        if (!isPaused) {
            accumulatedTime += dt;
        }

        float t = accumulatedTime; 

        float phase = std::fmod(t, bouncePeriod) / bouncePeriod; 
        float ballY = ballBaseY + maxHeight * std::sin(phase * 3.14159265f);


        float tripDuration = 4.0f * bouncePeriod; 
        float span = 3.6f;                        
        float squareSpeed = span / tripDuration;

        float tPeakOffset = 0.5f * bouncePeriod;
        float shiftedTime = t - tPeakOffset;
        float normalizedTime = std::fmod(shiftedTime, tripDuration);
        if (normalizedTime < 0.0f) normalizedTime += tripDuration;


        float squareX = (span / 2.0f) - (normalizedTime * squareSpeed);
        float squareY = ballBaseY;                    


        glClearColor(0.06f, 0.07f, 0.12f, 1.0f); // Терең қанық түсті фон
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);


        glm::mat4 ballModel = glm::mat4(1.0f);
        ballModel = glm::translate(ballModel, glm::vec3(ballX, ballY, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(ballModel));
        glBindVertexArray(circleVAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(circleIndices.size()), GL_UNSIGNED_INT, 0);


        glm::mat4 squareModel = glm::mat4(1.0f);
        squareModel = glm::translate(squareModel, glm::vec3(squareX, squareY, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(squareModel));
        glBindVertexArray(squareVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 9. Жадты тазалау
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteBuffers(1, &circleEBO);
    glDeleteVertexArrays(1, &squareVAO);
    glDeleteBuffers(1, &squareVBO);
    glDeleteBuffers(1, &squareEBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}