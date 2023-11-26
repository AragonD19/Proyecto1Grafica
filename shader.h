#include "glm/glm.hpp"
#include "uniform.h"
#include "fragment.h"
#include "color.h"
#include "vertex.h"
#include "FastNoiseLite.h"

// Add the following include for GLSL types
#include "glm/gtc/type_ptr.hpp"

void printMatrix(const glm::mat4& myMatrix) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << myMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printVec4(const glm::vec4& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
}

void printVec3(const glm::vec3& vector) {
    std::cout << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
}

Vertex vertexShader(const Vertex& vertex, const Uniform& uniform) {
    glm::vec4 transformedVertex = uniform.projection * uniform.view * uniform.model * glm::vec4(vertex.position, 1.0f);
    double z = transformedVertex.z;
    transformedVertex = uniform.viewport * transformedVertex;
    glm::vec3 vertexRedux;
    vertexRedux.x = transformedVertex.x / transformedVertex.w;
    vertexRedux.y = transformedVertex.y / transformedVertex.w;
    vertexRedux.z = transformedVertex.z / transformedVertex.w;

    Color fragmentColor(255, 0, 0, 255);
    glm::vec3 normal = glm::normalize(glm::mat3(uniform.model) * vertex.normal);

    Fragment fragment;
    fragment.position = glm::ivec2(transformedVertex.x, transformedVertex.y);
    fragment.color = fragmentColor;

    return Vertex {vertexRedux, normal, vertex.position, z};
}

float nextTime = 0.5f;

Color interpolateColor(const Color& color1, const Color& color2, float t) {
    // Interpolación lineal entre dos colores
    t = glm::clamp(t, 0.0f, 1.0f);
    return Color(
            static_cast<uint8_t>((1.0f - t) * color1.r + t * color2.r),
            static_cast<uint8_t>((1.0f - t) * color1.g + t * color2.g),
            static_cast<uint8_t>((1.0f - t) * color1.b + t * color2.b),
            static_cast<uint8_t>((1.0f - t) * color1.a + t * color2.a)
    );
}

Color fragmentShaderSky(Fragment& fragment) {
    // Generate random positions for stars
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Crear un objeto FastNoiseLite para generar ruido
    FastNoiseLite noise;

    // Configuración del ruido
    noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    noise.SetSeed(18340); // Semilla para la generación de ruido (puedes cambiarla)
    noise.SetFrequency(0.035f); // Frecuencia del ruido (ajusta según tus preferencias)

    // Configuración de ruido fractal para variaciones
    noise.SetFractalType(FastNoiseLite::FractalType_PingPong); // Tipo de ruido fractal
    noise.SetFractalOctaves(2); // Número de octavas
    noise.SetFractalLacunarity(8 + nextTime); // Lacunarity (variación en la frecuencia)
    noise.SetFractalGain(0.9f); // Ganancia
    noise.SetFractalWeightedStrength(0.80f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Parámetros para la rotación de las estrellas
    float ox = 15.0f; // Desplazamiento en X
    float oy = 15.0f; // Desplazamiento en Y
    float zoom = 3000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Configurar el color de las estrellas (blanco)
    Color starColor(255, 255, 255, 255);

    // Si el valor de ruido es mayor que cierto umbral, muestra una estrella
    if (noiseValue > 0.90f) {
        return starColor; // Color blanco para las estrellas
    }
    // Si no es una estrella, devuelve un color negro para el espacio vacío
    return Color(0, 0, 0, 255); // Color negro para el espacio
}




Color fragmentShaderSun(Fragment& fragment) {
    // Obtiene las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Crear un objeto FastNoiseLite para generar ruido
    FastNoiseLite noise;

    // Configuración del ruido
    noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular); // Tipo de ruido celular
    noise.SetSeed(1500); // Semilla para la generación de ruido (puedes cambiarla)
    noise.SetFrequency(0.005f); // Frecuencia del ruido (ajusta según tus preferencias)

    // Configuración de ruido fractal para variaciones
    noise.SetFractalType(FastNoiseLite::FractalType_PingPong); // Tipo de ruido fractal
    noise.SetFractalOctaves(2); // Número de octavas
    noise.SetFractalLacunarity(10 + nextTime); // Lacunarity (variación en la frecuencia)
    noise.SetFractalGain(1.0f); // Ganancia
    noise.SetFractalWeightedStrength(0.80f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(20); // Jitter (variación en las celdas)

    // Parámetros para la rotación del sol
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 15000.0f; // Factor de zoom más grande (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Definir el rango de colores desde rojo hasta naranja para el sol
    Color redColor(255, 0, 0, 255);
    Color orangeColor(255, 165, 0, 255);
    Color flareColor(255, 140, 0, 255); // Color de las llamaradas (ajusta según tus preferencias)

    // Interpolar entre rojo y naranja según el valor de ruido
    Color tmpColor = interpolateColor(redColor, orangeColor, noiseValue);

    // Agregar efecto de llamaradas de fuego
    float flareIntensity = 2.0f;
    float oxf = 6000.0f; // Desplazamiento en X
    float oyf = 6000.0f; // Desplazamiento en Y
    float zoomf = 8000.0f; // Factor de zoom (ajusta según tus preferencias)

    float noiseValuef = abs(noise.GetNoise((fragment.original.x + oxf) * zoomf, (fragment.original.y + oyf) * zoomf, fragment.original.z * zoomf));

    if (noiseValuef > 0.9f) {
        // Aplicar un color naranja para las llamaradas
        tmpColor = tmpColor + (flareColor * flareIntensity);
    }

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = tmpColor * fragment.z;

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return fragment.color;
}

Color fragmentShaderRoky(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar ruido para simular las características de la superficie de Mercurio
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(12000);
    noise.SetFrequency(0.0002f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(10.0f + nextTime);
    noise.SetFractalGain(0.2f);
    noise.SetFractalWeightedStrength(0.50f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(6); // Jitter (variación en las celdas)

    // Definir colores para representar la superficie de Mercurio
    Color rockyColor(128, 64, 0, 255);  // Color rocoso para Mercurio
    Color darkRedColor(139, 0, 0, 255);  // Color rojo oscuro

    // Parámetros para la rotación y escala del ruido
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 5000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Seleccionar el color en función del valor de ruido y el umbral
    Color tmpColor = (noiseValue < 0.3f) ? rockyColor : darkRedColor;

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = tmpColor * fragment.z;

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return fragment.color;
}

Color fragmentShaderGasGiant(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar ruido para simular la atmósfera y variaciones en la superficie
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(3000);
    noise.SetFrequency(0.002f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(4);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);

    // Parámetros para la rotación y escala del ruido
    float ox = 5000.0f; // Desplazamiento en X
    float oy = 5000.0f; // Desplazamiento en Y
    float zoom = 8000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Definir colores para representar la atmósfera del gigante gaseoso
    Color atmosphereColor(100, 100, 255, 255);  // Color azul para la atmósfera
    Color cloudColor(200, 200, 200, 255);  // Color de las nubes

    // Interpolar entre el color de la atmósfera y el color de las nubes según el valor de ruido
    Color baseColor = interpolateColor(atmosphereColor, cloudColor, noiseValue);

    // Ajustar parámetros para un efecto de partículas más notorio
    float particleIntensity = 5.0f;
    float oxp = 6000.0f; // Desplazamiento en X para partículas
    float oyp = 6000.0f; // Desplazamiento en Y para partículas
    float zoomp = 8000.0f; // Factor de zoom para partículas

    float noiseValuep = abs(noise.GetNoise((fragment.original.x + oxp) * zoomp, (fragment.original.y + oyp) * zoomp, fragment.original.z * zoomp));

    if (noiseValuep > 0.7f) {
        // Aplicar un color más brillante para las partículas
        baseColor = baseColor + (Color(255, 255, 255, 255) * particleIntensity);
    }

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = baseColor * fragment.z;

    return fragment.color;
}

Color fragmentShaderEarthRealistic(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar un ruido para simular las características de la superficie terrestre
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(12000);
    noise.SetFrequency(0.0002f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(10.0f + nextTime);
    noise.SetFractalGain(0.2f);
    noise.SetFractalWeightedStrength(0.50f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(6); // Jitter (variación en las celdas)

    // Definir colores para el agua y la tierra
    Color waterColor(0, 0, 255, 255); // Color del océano
    Color landColor(0, 128, 0, 255);  // Color de la tierra
    Color cloudColor(233, 239, 240, 200);  // Color de las nubes

    // Parámetros para la rotación del planeta
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 5000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Seleccionar el color en función del valor de ruido y el umbral
    Color baseColor = (noiseValue < 0.3f) ? waterColor : landColor;

    // Agregar nubes
    float cloudIntensity = 0.2f;
    float oxn = 6000.0f; // Desplazamiento en X para nubes
    float oyn = 6000.0f; // Desplazamiento en Y para nubes
    float zoomn = 8000.0f; // Factor de zoom para nubes

    float noiseValueN = abs(noise.GetNoise((fragment.original.x + oxn) * zoomn, (fragment.original.y + oyn) * zoomn, fragment.original.z * zoomn));

    if (noiseValueN > 0.8f) {
        // Aplicar un color más brillante para las nubes
        baseColor = interpolateColor(baseColor, cloudColor, cloudIntensity);
    }

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = baseColor * fragment.z;

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return fragment.color;
}

Color fragmentShaderMars(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar un ruido para simular las características de la superficie de Marte
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(2050);
    noise.SetFrequency(0.006f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(2);
    noise.SetFractalLacunarity(4.0f + nextTime);
    noise.SetFractalGain(0.8f);
    noise.SetFractalWeightedStrength(0.80f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(8); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(6); // Jitter (variación en las celdas)

    // Definir colores para representar la superficie de Marte
    Color marsColor1(184, 73, 46, 255); // Color de Marte principal
    Color marsColor2(138, 77, 62, 255);

    // Parámetros para la rotación del planeta
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 5000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Seleccionar el color en función del valor de ruido y el umbral
    Color tmpColor = (noiseValue < 0.4f) ? marsColor1 : marsColor2;

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = tmpColor * fragment.z;

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return fragment.color;
}

Color fragmentShaderMercury(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar ruido para simular las características de la superficie de Mercurio
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(5000); // Puedes ajustar la semilla según tus preferencias
    noise.SetFrequency(0.001f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(10.0f + nextTime);
    noise.SetFractalGain(0.2f);
    noise.SetFractalWeightedStrength(0.50f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(6); // Jitter (variación en las celdas)

    // Definir colores para representar la superficie de Mercurio
    Color rockyColor(128, 64, 0, 255);  // Color rocoso para Mercurio
    Color darkRedColor(139, 0, 0, 255);  // Color rojo oscuro

    // Parámetros para la rotación y escala del ruido
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 5000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Seleccionar el color en función del valor de ruido y el umbral
    Color tmpColor = (noiseValue < 0.3f) ? rockyColor : darkRedColor;

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    fragment.color = tmpColor * fragment.z;

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return fragment.color;
}


Color fragmentShaderMoon(Fragment& fragment) {
    // Obtener las coordenadas del fragmento en el espacio 2D
    // Obtener las coordenadas del fragmento en el espacio 2D
    glm::vec2 fragmentCoords(fragment.original.x, fragment.original.y);

    // Aplicar un ruido para simular las características de la superficie lunar
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(3000);
    noise.SetFrequency(0.004f);
    noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
    noise.SetFractalOctaves(3);
    noise.SetFractalLacunarity(10.0f + nextTime);
    noise.SetFractalGain(0.2f);
    noise.SetFractalWeightedStrength(0.50f); // Fuerza ponderada
    noise.SetFractalPingPongStrength(10); // Fuerza de ping pong

    // Configuración de ruido celular
    noise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_Euclidean); // Función de distancia celular
    noise.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2Add); // Tipo de retorno celular
    noise.SetCellularJitter(6); // Jitter (variación en las celdas)

    // Definir colores para representar la superficie lunar y el color de los cráteres
    Color moonColor(200, 200, 200, 255); // Color de la Luna
    Color craterColor(50, 50, 50, 255);   // Color de los cráteres

    // Parámetros para la rotación de la Luna
    float ox = 3000.0f; // Desplazamiento en X
    float oy = 3000.0f; // Desplazamiento en Y
    float zoom = 5000.0f; // Factor de zoom (ajusta según tus preferencias)

    // Obtener el valor de ruido en función de la posición y el zoom
    float noiseValue = abs(noise.GetNoise((fragment.original.x + ox) * zoom, (fragment.original.y + oy) * zoom, fragment.original.z * zoom));

    // Multiplicar el color por la coordenada Z para simular la perspectiva
    Color baseColor = moonColor * fragment.z;

    // Agregar cráteres utilizando el valor de ruido
    float craterIntensity = 0.1f;
    if (noiseValue > 0.8f) {
        // Mezclar con el color de los cráteres
        baseColor = interpolateColor(baseColor, craterColor, craterIntensity);
    }

    // Incrementar la variable "nextTime" para animar el ruido (rotación)
    nextTime += 0.5f; // Puedes ajustar la velocidad de rotación

    return baseColor;
}

Color fragmentShaderShip(Fragment& fragment) {
    Color ship(0, 0, 255);
    fragment.color = ship;

    return fragment.color;
}



