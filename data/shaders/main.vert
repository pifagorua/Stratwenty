#version 430 core
layout(location = 0) in vec3 vertCoord;
layout(location = 1) in vec2 texCoord;

out vec2 texCoordOut;

uniform mat4 View, Model; // ModelViewProjection (Total matrix)

void main(){
    gl_Position = View * Model * vec4(vertCoord, 1);
    texCoordOut = texCoord;
}
