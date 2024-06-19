#version 330 core
layout(location = 0) in vec3 aPos;
out vec2 fragCoord;
uniform vec2 screenSize;
void main() {
    fragCoord = (aPos.xy + vec2(0.5, 0.5)) * screenSize;
    gl_Position = vec4(aPos, 1.0);
}