#version 330 core

layout(location = 0) in vec4 vWorldMatPart0;
layout(location = 1) in vec4 vWorldMatPart1;
layout(location = 2) in vec4 vWorldMatPart2;

out vec4 gWorldMatPart0;
out vec4 gWorldMatPart1;
out vec4 gWorldMatPart2;

void main()
{
    gWorldMatPart0 = vWorldMatPart0;
    gWorldMatPart1 = vWorldMatPart1;
    gWorldMatPart2 = vWorldMatPart2;
}
