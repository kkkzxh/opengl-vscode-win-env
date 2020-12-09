#version 330 core
out vec4 FragColor;

in VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
}fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
	// ִ��͸�ӳ���
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// �任��[0,1]��Χ��
	projCoords = projCoords * 0.5 + 0.5;

	// �ӹ�Դ�ǶȻ�ȡ��������ֵ��(ʹ��[0,1]��Χ�µ�fragPosLight��Ϊ����)
	float closestDepth = texture(shadowMap, projCoords.xy).r;

	// �ӹ�ĽǶȻ�ȡƬ�ε����
	float currentDepth = projCoords.z;

	// ��Ӱƫ��ȥ������
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

	// ��鵱ǰ����Ƿ���������ȣ�����������Ӱ��
	float shadow = (currentDepth - bias)> closestDepth ? 1.0 : 0.0;
		
	// PCF�����Ӱ
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

	if(projCoords.z > 1.0)
        shadow = 0.0;

	return shadow;
}

void main()
{
	// ����Blinn Phong���ռ���
	vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightColor = vec3(0.3);
	// ambient
	vec3 ambient = 0.3 * color;
	
	// diffuse
	vec3 lightDir = normalize(lightPos - fs_in.FragPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diff * lightColor;
	
	// specular
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = 0.0;
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
	vec3 specular = spec * lightColor;

	// ������Ӱ
	float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal, lightDir);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

	FragColor = vec4(lighting, 1.0);
}