# pyms3d
C++ wrapper library for libms3d to provide python with everything necessary to develop games in modern OpenGL

# Features:
Load and render 3d models (through libms3d)

Load OpenGL textures from png and jpeg files (through libms3d)

Compile and load GLSL shaders (through libms3d)

A Matrix stack and easy to use methods for common matrix operations

Dynamic shadows

Lights

Render Text

# Installation and Usage:

Run "make" and then copy the ms3d.py and _ms3d.so files to the location of your python project

To use in your code:
```
from ms3d import ms3d, Tex, shader, Lights, GLM, Shadows, Text, MATRIX
```
And after you set up an OpenGL context call:
```
ms3d.initGlew()
```

To load a 3d model from a file:
```
model = ms3d("path/to/file.ms3d")
model.prepare(shader) # only necessary if using modern OpenGL
```
To render a 3d model:
```
model.draw()  # Draw using old OpenGL
model.drawGL3()  # Draw using modern OpenGL
```

To load a shader program:
```
Shader = shader("shaders/vert.shader", "shaders/frag.shader")
```

The ms3d and Tex classes work for both old and modern OpenGL, all the others work exclusively with shaders (example shaders are provided in the shaders folder)

# Screenshot

![Screenshot](screenshot.png)
