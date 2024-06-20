#version 460 core
layout (location = 0) in vec3 POS;

out vec3 color;

uniform mat4 model;
uniform mat4 proj_view;

void main()
{
        color = vec3(1.0f,1.0f,0.0f);
        gl_Position = proj_view * model * vec4(POS.xyz, 1.0f);
}
