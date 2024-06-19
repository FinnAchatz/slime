#version 330 core
in vec2 fragCoord;
out vec4 FragColor;
void main() {
    FragColor = vec4(fragCoord / 800.0, 0.0, 1.0);
}