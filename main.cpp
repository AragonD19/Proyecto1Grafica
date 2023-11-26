#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <cmath>
#include <random>
#include <SDL.h>
#include <thread>
#include "mutex"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "color.h"
#include "framebuffer.h"
#include "point.h"
#include "fragment.h"
#include "uniform.h"
#include "shader.h"
#include "vertex.h"
#include "loadOBJ.h"
#include "FastNoiseLite.h"

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
float pi = 3.14f / 3.0f;
std::mutex mutex;
Uint32 startingFrame;
Uint32 frameTime;
int frameCounter = 0;
int fps = 0;


Color colorClear = {0, 0, 0, 255};


SDL_Renderer* renderer;
std::array<double, WINDOW_WIDTH * WINDOW_HEIGHT> zBuffer;

glm::vec3 light = glm::vec3(0.0f, 0.0f, 0.0f);

enum Planets {
    SPACE,
    SUN,
    MERC,
    EARTH,
    MOON,
    MARS,
    ROKY,
    GAS,
    SHIP
};


struct Model {
    Uniform uniform;
    std::vector<Vertex>* v;
    Planets i;
};

SDL_Window* window;

Color interpolateColor(const glm::vec3& barycentricCoord, const Color& color1, const Color& color2, const Color& color3) {
    float u = barycentricCoord.x;
    float v = barycentricCoord.y;
    float w = barycentricCoord.z;

    // Realiza una interpolación lineal para cada componente del color
    uint8_t r = static_cast<uint8_t>(u * color1.r + v * color2.r + w * color3.r);
    uint8_t g = static_cast<uint8_t>(u * color1.g + v * color2.g + w * color3.g);
    uint8_t b = static_cast<uint8_t>(u * color1.b + v * color2.b + w * color3.b);
    uint8_t a = static_cast<uint8_t>(u * color1.a + v * color2.a + w * color3.a);

    return Color(r, g, b, a);
}

bool isBarycentricCoord(const glm::vec3& barycentricCoord) {
    return barycentricCoord.x >= 0 && barycentricCoord.y >= 0 && barycentricCoord.z >= 0 &&
           barycentricCoord.x <= 1 && barycentricCoord.y <= 1 && barycentricCoord.z <= 1 &&
           glm::abs(1 - (barycentricCoord.x + barycentricCoord.y + barycentricCoord.z)) < 0.00001f;
}

glm::vec3 calculateBarycentricCoord(const glm::vec2& A, const glm::vec2& B, const glm::vec2& C, const glm::vec2& P) {
    float denominator = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
    float u = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
    float v = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
    float w = 1 - u - v;
    return glm::vec3(u, v, w);
}


glm::mat4 createModelMatrix() {
    glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, -10.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(40.0f, 40.0f, 10.0f));
    glm::mat4 rotation = glm::mat4(1);
    return translation * scale * rotation;
}

glm::mat4 createPlanetModelMatrix(glm::vec3 translationM, glm::vec3 scaleM, glm::vec3 rotationM, float radianSpeed)  {
    glm::mat4 translation = glm::translate(glm::mat4(1), translationM);
    glm::mat4 scale = glm::scale(glm::mat4(1), scaleM);
    glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians((pi++)*radianSpeed), rotationM);
    return translation * scale * rotation;
}


glm::mat4 createModelShip(glm::vec3 cameraPosition, glm::vec3 targetPosition,glm::vec3 upVector, float rotX, float rotY) {
    glm::mat4 translation = glm::translate(glm::mat4(1), (targetPosition - cameraPosition) / 3.0f + cameraPosition - upVector * 0.15f);
    glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(0.05f, 0.05f, 0.05f));
    glm::mat4 rotationX = glm::rotate(glm::mat4(1), glm::radians(-rotX), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1), glm::radians(-rotY), glm::vec3(0.0f, 0.0f, 1.0f));
    return translation * scale * rotationX * rotationY;
}

glm::mat4 createViewMatrix() {
    return glm::lookAt(
            // En donde se encuentra
            glm::vec3(0, 0, 2),
            // En donde está viendo
            glm::vec3(0, 0, 0),
            // Hacia arriba para la cámara
            glm::vec3(0, 1, 0)
    );
}

glm::mat4 createProjectionMatrix() {
    float fovInDegrees = 45.0f;
    float aspectRatio = WINDOW_WIDTH / WINDOW_HEIGHT;
    float nearClip = 0.1f;
    float farClip = 100.0f;
    return glm::perspective(glm::radians(fovInDegrees), aspectRatio, nearClip, farClip);
}

glm::mat4 createViewportMatrix() {
    glm::mat4 viewport = glm::mat4(1.0f);
    viewport = glm::scale(viewport, glm::vec3(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.5f));
    viewport = glm::translate(viewport, glm::vec3(1.0f, 1.0f, 0.5f));
    return viewport;
}


glm::vec3 calculatePosition(float angleR, float radius){
    float posX = glm::cos(angleR) * radius;
    float posZ = glm::sin(angleR) * radius;
    return glm::vec3(posX, 0.0f, posZ);
}


Uniform uniform;
Uniform uniform2;
Uniform uniform3;
Uniform uniform4;
Uniform uniform5;
Uniform uniform6;
Uniform uniform7;
Uniform uniform8;
Uniform uniform9;

Model model1;
Model model2;
Model model3;
Model model4;
Model model5;
Model model6;
Model model7;
Model model8;
Model model9;



void render(const std::vector<Vertex>& vertexArray,  const Uniform& uniform, int planetIdentifier) {
    std::vector<Vertex> transformedVertexArray;
    for (const auto& vertex : vertexArray) {
        auto transformedVertex = vertexShader(vertex, uniform);
        transformedVertexArray.push_back(transformedVertex);
    }

    for (size_t i = 0; i < transformedVertexArray.size(); i += 3) {
        const Vertex& a = transformedVertexArray[i];
        const Vertex& b = transformedVertexArray[i + 1];
        const Vertex& c = transformedVertexArray[i + 2];

        glm::vec3 A = a.position;
        glm::vec3 B = b.position;
        glm::vec3 C = c.position;

        int minX = static_cast<int>(std::min({A.x, B.x, C.x}));
        int minY = static_cast<int>(std::min({A.y, B.y, C.y}));
        int maxX = static_cast<int>(std::max({A.x, B.x, C.x}));
        int maxY = static_cast<int>(std::max({A.y, B.y, C.y}));

        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                if (y>0 && y<WINDOW_HEIGHT && x>0 && x<WINDOW_WIDTH) {
                    glm::vec2 pixelPosition(static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f);
                    glm::vec3 barycentricCoord = calculateBarycentricCoord(A, B, C, pixelPosition);

                    double cam = barycentricCoord.x * a.z + barycentricCoord.y * b.z + barycentricCoord.z * c.z;

                    if (isBarycentricCoord(barycentricCoord) && cam > 0) {
                        Color modelColor {0, 0, 0};
                        Color interpolatedColor = interpolateColor(barycentricCoord, modelColor, modelColor, modelColor);

                        float depth = barycentricCoord.x * A.z + barycentricCoord.y * B.z + barycentricCoord.z * C.z;

                        glm::vec3 normal = a.normal * barycentricCoord.x + b.normal * barycentricCoord.y+ c.normal * barycentricCoord.z;

                        float fragmentIntensity = (abs(glm::dot(normal, light)) > 1 ) ? 1: abs(glm::dot(normal, light));

                        if (planetIdentifier == SPACE) {
                            fragmentIntensity = glm::dot(normal, glm::vec3(0.0f,0.0f,1.0f));
                        }
                        if (fragmentIntensity <= 0){
                            continue;
                        }

                        Color finalColor = interpolatedColor * fragmentIntensity;
                        glm::vec3 original = a.original * barycentricCoord.x + b.original * barycentricCoord.y + c.original * barycentricCoord.z;

                        Fragment fragment;
                        fragment.position = glm::ivec2(x, y);
                        fragment.color = finalColor;
                        fragment.z = depth;
                        fragment.original = original;

                        int index = y * WINDOW_WIDTH + x;
                        if (depth < zBuffer[index]) {
                            mutex.lock();
                            Color fragmentShaderf;

                            switch (planetIdentifier) {
                                case SPACE:
                                    fragmentShaderf = fragmentShaderSky(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case SUN:
                                    fragmentShaderf = fragmentShaderSun(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case MERC:
                                    fragmentShaderf = fragmentShaderMercury(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case EARTH:
                                    fragmentShaderf = fragmentShaderEarthRealistic(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case MOON:
                                    fragmentShaderf = fragmentShaderMoon(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case MARS:
                                    fragmentShaderf = fragmentShaderMars(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case ROKY:
                                    fragmentShaderf = fragmentShaderRoky(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case GAS:
                                    fragmentShaderf = fragmentShaderGasGiant(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                                case SHIP:
                                    fragmentShaderf = fragmentShaderShip(fragment);
                                    SDL_SetRenderDrawColor(renderer, fragmentShaderf.r, fragmentShaderf.g, fragmentShaderf.b, fragmentShaderf.a);
                                    break;
                            }

                            SDL_RenderDrawPoint(renderer, x, WINDOW_HEIGHT-y);
                            nextTime = 0.5f + 1.0f;
                            zBuffer[index] = depth;
                            mutex.unlock();
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* args[]) {
    SDL_Init(SDL_INIT_EVERYTHING);


    uniform.view = createViewMatrix();

    srand(time(nullptr));

    window = SDL_CreateWindow("SR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int renderWidth, renderHeight;
    SDL_GetRendererOutputSize(renderer, &renderWidth, &renderHeight);



    std::vector<glm::vec3> SphereVertices;
    std::vector<glm::vec3> SphereNormals;
    std::vector<Face> SphereFaces;

    bool planetSuccess = loadOBJ("../modelos/Esfera.obj", SphereVertices, SphereNormals, SphereFaces);
    if (!planetSuccess) {
        return 1;
    }

    std::vector<Vertex> SphereVertexArray = setupVertexArray(SphereVertices, SphereNormals, SphereFaces);


    std::vector<glm::vec3> shipVertices;
    std::vector<glm::vec3> shipNormal;
    std::vector<Face> shipFaces;

    bool success2 = loadOBJ("../modelos/Lab3.obj", shipVertices, shipNormal, shipFaces);
    if (!success2) {
        return 1;
    }
    std::vector<Vertex> vertexArrayShip = setupVertexArray(shipVertices, shipNormal, shipFaces);


    float forwardBackwardMovementSpeed = 0.1f;
    float leftRightMovementSpeed = 0.06f;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float sunRotation = 0.0f;
    float mercRotation = 0.0f;
    float earthRotation = 0.0f;
    float moonRotation = 0.0f;
    float marsRotation = 0.0f;
    float rokyRotation = 0.0f;
    float gasRotation = 0.0f;

    glm::vec3 cameraPosition(0.0f, 0.0f, 17.0f);
    glm::vec3 targetPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 upVector(0.0f, 1.0f, 0.0f);

    std::vector<Model> models;

    glm::vec3 newCameraPosition;
    glm::vec3 newShipCameraPosition;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type = SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_w:
                        cameraPosition += forwardBackwardMovementSpeed * glm::normalize(targetPosition - cameraPosition);
                        targetPosition += forwardBackwardMovementSpeed * (targetPosition - cameraPosition);
                        break;
                    case SDLK_a:
                        cameraPosition -= leftRightMovementSpeed * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector)) * 2.5f;
                        targetPosition -= leftRightMovementSpeed * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector)) * 2.5f;
                        break;
                    case SDLK_s:
                        cameraPosition -= forwardBackwardMovementSpeed * glm::normalize(targetPosition - cameraPosition);
                        targetPosition -= forwardBackwardMovementSpeed * (targetPosition - cameraPosition);
                        break;
                    case SDLK_d:
                        cameraPosition += leftRightMovementSpeed * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector)) * 2.5f;
                        targetPosition += leftRightMovementSpeed * glm::normalize(glm::cross((targetPosition - cameraPosition), upVector)) * 2.5f;
                        break;
                }
            }

        }

        models.clear();
        light = cameraPosition - targetPosition;
        sunRotation += 0.01f;
        mercRotation += 0.2f;
        earthRotation += 0.15f;
        moonRotation += 0.15f;
        marsRotation += 0.1f;
        rokyRotation += 0.3f;
        gasRotation += 0.085f;
        targetPosition = glm::vec3(5.0f * sin(glm::radians(rotationX)) * cos(glm::radians(rotationY)), 5.0f * sin(glm::radians(rotationY)), -5.0f * cos(glm::radians(rotationX)) * cos(glm::radians(rotationY))) + cameraPosition;

        glm::vec3 translateMerc = calculatePosition(mercRotation, 3.0f);
        glm::vec3 translateEarth = calculatePosition(earthRotation, 4.0f);
        glm::vec3 translateMoon = calculatePosition(moonRotation, 4.8f);
        glm::vec3 translateMars = calculatePosition(marsRotation, 5.5f);
        glm::vec3 translateRoky = calculatePosition(rokyRotation, 6.5f);
        glm::vec3 translateGas = calculatePosition(gasRotation, 7.5f);

        uniform.model = createModelMatrix();
        uniform.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform.projection = createProjectionMatrix();
        uniform.viewport = createViewportMatrix();

        model1.uniform = uniform;
        model1.v = &SphereVertexArray;
        model1.i = SPACE;

        uniform2.model = createPlanetModelMatrix(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 1.0f, 0.0f), 0.1f);
        uniform2.view =  glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform2.projection = createProjectionMatrix();
        uniform2.viewport = createViewportMatrix();

        model2.uniform = uniform2;
        model2.v = &SphereVertexArray;
        model2.i = SUN;

        uniform3.model = createPlanetModelMatrix(translateMerc, glm::vec3(0.25, 0.25, 0.25), glm::vec3(0.0f, 1.0f, 0.0f), 0.35f);
        uniform3.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform3.projection = createProjectionMatrix();
        uniform3.viewport = createViewportMatrix();

        model3.uniform = uniform3;
        model3.v = &SphereVertexArray;
        model3.i = MERC;

        uniform4.model = createPlanetModelMatrix(translateEarth, glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(0.0f, 1.0f, 0.0f), 0.3f);
        uniform4.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform4.projection = createProjectionMatrix();
        uniform4.viewport = createViewportMatrix();

        model4.uniform = uniform4;
        model4.v = &SphereVertexArray;
        model4.i = EARTH;

        uniform5.model = createPlanetModelMatrix(translateMoon, glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.0f, 1.0f, 0.0f), 0.15f);
        uniform5.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform5.projection = createProjectionMatrix();
        uniform5.viewport = createViewportMatrix();

        model5.uniform = uniform5;
        model5.v = &SphereVertexArray;
        model5.i = MOON;

        uniform6.model = createPlanetModelMatrix(translateMars, glm::vec3(0.65f, 0.65f, 0.65f), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniform6.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform6.projection = createProjectionMatrix();
        uniform6.viewport = createViewportMatrix();

        model6.uniform = uniform6;
        model6.v = &SphereVertexArray;
        model6.i = MARS;

        uniform7.model = createPlanetModelMatrix(translateRoky, glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniform7.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform7.projection = createProjectionMatrix();
        uniform7.viewport = createViewportMatrix();

        model7.uniform = uniform7;
        model7.v = &SphereVertexArray;
        model7.i = ROKY;

        uniform8.model = createPlanetModelMatrix(translateGas, glm::vec3(0.8f, 0.8f, 0.8f), glm::vec3(0.0f, 1.0f, 0.0f), 0.2f);
        uniform8.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform8.projection = createProjectionMatrix();
        uniform8.viewport = createViewportMatrix();

        model8.uniform = uniform8;
        model8.v = &SphereVertexArray;
        model8.i = GAS;

        uniform9.model = createModelShip(cameraPosition, targetPosition, upVector, rotationX, rotationY);
        uniform9.view = glm::lookAt(cameraPosition, targetPosition, upVector);
        uniform9.projection = createProjectionMatrix();
        uniform9.viewport = createViewportMatrix();

        model9.uniform = uniform9;
        model9.v = &vertexArrayShip;
        model9.i = SHIP;

        models.push_back(model1);
        models.push_back(model2);
        models.push_back(model3);
        models.push_back(model4);
        models.push_back(model5);
        models.push_back(model6);
        models.push_back(model7);
        models.push_back(model8);
        models.push_back(model9);

        SDL_SetRenderDrawColor(renderer, colorClear.r, colorClear.g, colorClear.b, colorClear.a);
        SDL_RenderClear(renderer);
        std::fill(zBuffer.begin(), zBuffer.end(), std::numeric_limits<double>::max());


        std::vector<std::thread> threadS;

        for (const Model& model : models) {
            threadS.emplace_back(render, *model.v, model.uniform, model.i);
        }
        for (std::thread& thread : threadS) {
            thread.join();
        }

        SDL_RenderPresent(renderer);
        frameTime = SDL_GetTicks() - startingFrame;
        frameCounter++;
        if (frameTime >= 1000) {
            fps = frameCounter;
            frameCounter = 0;
            startingFrame = SDL_GetTicks();
        }
        std::string fpsText = "Space Travel | FPS: " + std::to_string(fps);
        SDL_SetWindowTitle(window, fpsText.c_str());

    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
