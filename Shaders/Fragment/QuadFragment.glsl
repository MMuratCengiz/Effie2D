#version 450

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 finalImage;

const float gridUnit = 0.1;
const float lineWidth = 0.001;

void main() {
    if (abs((-0.5 + inUV.x) * 10 - sin(inUV.y * 20)) < 0.01) {
        finalImage = vec4(vec3(0.1), 1.0);
    } else if (mod(inUV.x, gridUnit) < lineWidth || mod(inUV.y, gridUnit) < lineWidth) {
        finalImage = vec4(vec3(0.1), 1.0);
    } else {
        finalImage = vec4(vec3(inUV.xyy), 1.0);
    }
}