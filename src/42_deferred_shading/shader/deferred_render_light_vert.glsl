#version 330 core
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec2 iTexCoords;

out vec2 TexCoords;

void main(){
    gl_Position = vec4(iPosition, 1.0);
    TexCoords = iTexCoords;
}