#version 130

out vec4 color; // 片段着色器输出的变量名可以任意命名，类型必须是vec4
uniform vec3 ourColor;
void main()
{
    gl_FragColor =  vec4( ourColor, 1. );
}