#version 330 core

// Input vertex position in normalized device coordinates
layout(location = 0) in vec3 inPosition;

// Output to the fragment shader
out vec2 fragCoord;

uniform vec2 screenSize; // Screen size in pixels

void main() {
    // Convert normalized device coordinates to window coordinates
    fragCoord = (inPosition.xy + vec2(1.0, 1.0)) * 0.5 * screenSize;
    
    // Set the clip space position
    gl_Position = vec4(inPosition, 1.0);
}
