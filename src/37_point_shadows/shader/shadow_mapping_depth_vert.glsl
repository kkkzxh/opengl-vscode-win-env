#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;

void main()
{
	// ������λ�ñ任������ռ䲢���͵�������ɫ��
	gl_Position =  model * vec4(aPos, 1.0);
}