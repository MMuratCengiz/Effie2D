@vertex fn main(@location(0) pos : vec4<f32>) -> @builtin(position) vec4<f32> {
    return pos;
}