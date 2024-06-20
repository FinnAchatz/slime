#version 330 core

in vec2 fragPos;
out vec4 FragColor;

uniform vec2 uSize;
// uniform float uValues[300];
uniform ivec2 screenSize;
uniform sampler2D sampler; 

void main() {
    // ivec2 fragCoord = ivec2(fragPos * vec2(uSize));
    // float val = uValues[int(floor(fragPos.x * uSize.x) + floor(fragPos.y * uSize.y) * uSize.x)];

    float r = 0.2;
    // FragColor = vec4(texture(sampler, fragPos).rgb, 1.0);
    vec3 col = vec3(0.4, 1.0, 1.0) * texture(sampler, fragPos).r;
    FragColor = vec4(col, 1.0);
}
