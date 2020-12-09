#version 330 core
out vec4 FragColor;

in VS_OUT{
    vec3 FragPos;
    vec2 TexCoords;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D iDiffuseMap;
uniform sampler2D iNormalMap;
uniform sampler2D iHeightMap;

uniform bool iNormalMapping;
uniform float iHeightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{
	float height = texture(iHeightMap, texCoords).r;
	//vec2 p = texCoords - viewDir.xy / viewDir.z * (height * iHeightScale);
	//return texCoords - p;
	return texCoords - viewDir.xy * (height * iHeightScale);
}

vec2 ParallaxMappingLayer(vec2 texCoords, vec3 viewDir)
{
	// ��Ȳ���
	//const float numLayers = 32.0;

	// ��̬������������
	const float minLayers = 8;
	const float maxLayers = 32;

	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

	// ����ÿһ��Ĵ�С
	float layerDepth = 1.0 / numLayers;

	// ��ǰ�����
	float currentLayerDepth = 0.0;
		
	// ÿ���ƶ������������ ��p����ʼ
	vec2 p = viewDir.xy * iHeightScale;
	vec2 deltaTexCoords = p / numLayers;

	// ��ȡ��ʼֵ
	vec2 currentTexCoords = texCoords;
	float currentDepthMapValue = texture(iHeightMap, currentTexCoords).r;

	while(currentLayerDepth < currentDepthMapValue)
	{
		// ��p�����ƶ���������
		currentTexCoords -= deltaTexCoords;

		// ��ȡ��ǰ�������괦�Ĳ���ֵ
		currentDepthMapValue = texture(iHeightMap, currentTexCoords).r;

		// ��ȡ��һ������
		currentLayerDepth += layerDepth;
	}


	// ���ζ����Ӳ�ӳ�������

	// ����ײǰ��ȡ�������꣨���������
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// Ϊ���Բ�ֵ��ȡ��ײǰ������
	float afterDepth  = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = texture(iHeightMap, prevTexCoords).r - currentLayerDepth + layerDepth;

	// ���������ֵ
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;

	// return currentTexCoords;

}


void main(){

	vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	vec2 texCoords = fs_in.TexCoords;
	
	//texCoords = ParallaxMapping(fs_in.TexCoords, viewDir);

	texCoords = ParallaxMappingLayer(fs_in.TexCoords,  viewDir);
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
		discard;


    vec3 normal = texture(iNormalMap, texCoords).rgb;
    // vector to [-1, 1] range
    normal = normalize(normal * 2.0 - 1.0);

    // diffuse color
    vec3 color = texture(iDiffuseMap, texCoords).rgb;

    // ambient
    vec3 ambient = 0.1* color;

    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;

    // specular
    // vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0); 
    vec3 specular = vec3(0.2) * spec;

    FragColor = vec4(ambient + diffuse + specular, 1.0f);
}

