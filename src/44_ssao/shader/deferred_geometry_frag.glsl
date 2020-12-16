#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gColorSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main(){
  gPosition = FragPos; // 将位置向量存储在第一个gbuffer纹理中
  gNormal = normalize(Normal);
  gColorSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
  gColorSpec.a = texture(texture_specular1, TexCoords).r;
}