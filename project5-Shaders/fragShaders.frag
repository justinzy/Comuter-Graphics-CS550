#version 130

out vec4 color; // Ƭ����ɫ������ı����������������������ͱ�����vec4
uniform vec3 ourColor;
void main()
{
    gl_FragColor =  vec4( ourColor, 1. );
}