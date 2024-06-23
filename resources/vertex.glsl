#version 460 core
layout (location = 0) in vec3 POS;

out vec3 color;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
        color = vec3(1.0f,1.0f,0.0f);
        gl_Position = proj * view * model * vec4(POS.xyz, 1.0f);
}
