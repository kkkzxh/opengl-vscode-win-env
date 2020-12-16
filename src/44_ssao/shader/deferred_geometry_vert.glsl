#version 330 core
layout (location = 0) in vec3 iPosition;
layout (location = 1) in vec3 iNormal;
layout (location = 2) in vec2 iTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
  vec4 worldPosition = model * vec4(iPosition, 1.0f);
  FragPos = worldPosition.xyz;
  TexCoords = iTexCoords;
  
  mat3 normalMatrix = transpose(inverse(mat3(model)));
  Normal = normalMatrix * iNormal;

   gl_Position = projection * view * worldPosition;
}