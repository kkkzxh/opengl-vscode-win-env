#version 330 core
out vec4 FragColor;

// �����
struct DirLight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

// ���Դ
struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Spotlight {
	vec3  position; // �۹�λ��
	vec3  direction;// �۹�ķ���
	float cutOff; // �й��
	float outerCutOff; // ���й��

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant; // ������ 1.0
	float linear; // һ����
	float quadratic;// ������
};

// ���ղ��ʽṹ
struct Material {
	sampler2D  diffuse; // ������
	sampler2D specular; // �������
	float shininess;// �߹�ǿ��
};

uniform vec3 lightColor;
uniform vec3 objectColor;
uniform vec3 viewPos; // ���λ��

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform Material material;
uniform DirLight dirLight;
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Spotlight spotlight;

// ��������
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main() {

	vec3 outputColor;
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	
	// �����
	outputColor += CalcDirLight(dirLight, norm, viewDir);

	// 4�����Դ
	for (int i = 0; i < NR_POINT_LIGHTS; i++) {
		outputColor += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}

	// �۹��Դ
	outputColor += CalcSpotLight(spotlight, norm, FragPos, viewDir);

	FragColor = vec4(outputColor, 1.0f);
}


// ��������
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {

	vec3 lightDir = normalize(-light.direction);

	// ������
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// ������
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

	// ����߹�
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)); // �߹���ɫֵ

	return (ambient + diffuse + specular);

}

// ���Դ����
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {

	float distance = length(light.position - fragPos); // �����Դ�ľ���
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// ������
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// ������
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

	// ����߹�
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)); // �߹���ɫֵ

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(Spotlight light, vec3 normal, vec3 fragPos, vec3 viewDir) {

	vec3 lightDir = normalize(light.position - fragPos);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// ִ�й��ռ���
	float distance = length(light.position - fragPos); // �����Դ�ľ���
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// ������
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// ������
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

	// ����߹�
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords)); // �߹���ɫֵ

	ambient *= attenuation;
	diffuse *= intensity;
	specular *= intensity;

	return (ambient + diffuse + specular);
}