#version 130
out vec4 vertexColor; // ΪƬ����ɫ��ָ��һ����ɫ���

void main()
{
	gl_Position =  gl_ModelViewProjectionMatrix * gl_Vertex;
   // gl_Position = vec4(position, 1.0); // ��һ��vec3��Ϊvec4�Ĺ������Ĳ���
    vertexColor = vec4(0.5f, 0.0f, 0.0f, 1.0f); // �������ɫ����Ϊ����ɫ
}