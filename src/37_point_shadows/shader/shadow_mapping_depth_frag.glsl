#version 330 core
in vec4 FragPos;


uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	// ��ȡƬԪλ�õ���λ�õľ���
	float lightDistance = length(FragPos.xyz - lightPos);

	// ������ӳ�䵽0��1�ķ�Χ
	lightDistance = lightDistance / far_plane;

	gl_FragDepth = lightDistance;
}