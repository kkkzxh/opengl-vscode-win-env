#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;  //  0
uniform sampler2D gNormal;    //  1
uniform sampler2D gColorSpec; //  2

struct Light {
  vec3 Position;
  vec3 Color;
  float Linear;
  float Quadratic;
};

const int NR_LIGHTF = 32;
uniform Light lights[NR_LIGHTF];
uniform vec3 viewPos;

void main(){

  // 获取数据
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Diffuse = texture(gColorSpec, TexCoords).rgb;
  float Specular = texture(gColorSpec, TexCoords).r;

  // 计算光照
  vec3 lighting = Diffuse * 0.5;
  vec3 viewDir = normalize(viewPos - FragPos);
  for(int i = 0; i < NR_LIGHTF; ++i)
  {
      // diffuse
      vec3 lightDir = normalize(lights[i].Position - FragPos);
      vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;

      // specular
      vec3 halfwayDir = normalize(lightDir + viewDir);
      float spec = pow(max(dot(Normal, halfwayDir), 0.0), 64.0);
      vec3 specular = lights[i].Color * spec * Specular;

      // attenuation 衰减
      float distance = length(lights[i].Position - FragPos);
      float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
      diffuse *= attenuation;
      specular *= attenuation;
      lighting += diffuse + specular;
  }
  FragColor = vec4(lighting, 1.0);
}