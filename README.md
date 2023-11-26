# Descripción General del Proyecto 1

El código proporcionado es una implementación en C++ de un programa de renderizado gráfico utilizando la biblioteca Simple DirectMedia Layer (SDL). El objetivo principal del programa parece ser la simulación visual de un sistema solar en 3D, con capacidad para explorar el espacio y observar los planetas y objetos celestes.

## Estructura del Programa

### Archivos Principales
- **main.cpp:** Este es el archivo principal que contiene la función `main`. Controla el bucle principal del programa, maneja eventos del teclado, actualiza las posiciones y orientaciones de los objetos en el espacio, y coordina el proceso de renderizado.

### Archivos de Renderizado
- **fragment.h:** Contiene la definición de la estructura `Fragment`, que probablemente representa un píxel en el espacio de la pantalla.
- **framebuffer.h:** Contiene la definición de un framebuffer, que se utiliza para almacenar temporalmente la información de píxeles antes de enviarla al monitor.
- **line.h:** Probablemente contiene funciones o clases relacionadas con la representación y manipulación de líneas en el espacio 3D.
- **triangle.h:** Define funciones o clases relacionadas con la representación y manipulación de triángulos en el espacio 3D.

### Archivos de Gráficos
- **color.h:** Define la estructura `Color` que se utiliza para representar colores en el sistema de renderizado.
- **shader.h:** Se espera que contenga definiciones o implementaciones de shaders, que son programas ejecutados en la tarjeta gráfica para determinar cómo se renderizan los objetos.

### Archivos de Modelado 3D
- **loadOBJ.h:** Contiene funciones para cargar modelos 3D en el formato de archivo OBJ, que es común en gráficos 3D y se utiliza para importar geometría y otros datos.
- **vertex.h:** Define la estructura `Vertex`, que probablemente representa un vértice en un modelo 3D.

### Archivos de Utilidad
- **point.h:** Contiene definiciones relacionadas con puntos en el espacio 3D.
- **uniform.h:** Define la estructura `Uniform`, que probablemente se utiliza para gestionar variables uniformes en shaders.

### Bibliotecas Externas
- **SDL:** Se utiliza para la creación de ventanas, manipulación de eventos y renderizado gráfico.

## Funcionalidad del Programa

- **Simulación del Sistema Solar:** El programa simula visualmente un sistema solar en 3D con varios planetas, el sol, una nave espacial y otros elementos celestes.
- **Exploración del Espacio:** El usuario puede controlar una cámara para explorar el espacio y observar los diferentes objetos celestes desde diferentes perspectivas.
- **Movimiento y Rotación:** Los planetas y otros objetos realizan movimientos y rotaciones simulados para dar la apariencia de un sistema solar dinámico.

## Video
[![Demo del proyecto]([https://youtu.be/7k8NllgjH9A](https://youtu.be/7Kvn67wgDS0)https://youtu.be/7Kvn67wgDS0)]
