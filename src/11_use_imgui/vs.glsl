#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;// ģ�;��� ���� ������ת������������ϵ
uniform mat4 view; // ��ͼ���� ���� ����������ת�����۲�ռ�
uniform mat4 projection; // ͶӰ���� ���� ����������ͶӰ���ü�����

out vec3 FragPos; // ����λ��ת��������ռ������
out vec3 Normal;

void main()
{
    gl_Position = projection * view *  model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0));
	Normal = mat3(transpose(inverse(model))) * aNormal;
}