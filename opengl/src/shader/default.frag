#version 330 core

// Input from the vertex shader
in vec2 fragCoord;

out vec4 FragColor;

void main() {
    // Output the fragment coordinates as a color (normalized)
    FragColor = vec4(fragCoord / 1.0, 0.0, 1.0); // example to visualize
}
