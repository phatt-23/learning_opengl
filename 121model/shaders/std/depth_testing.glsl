
/// Non-linear function OpenGL uses for depth:
///
/// F_depth = (1/z - 1/near)/(1/far - 1/near)
///
/// Near values are high precision and far values have less precision.
/// Grow like a function of a sqaure root (+ side).
/// Values that are considered are in range [0.0, 1.0].
/// That is why most values are close to 1.0.
///
/// This function inverses this process and returns
/// the unprocessed depth value.
float linearizeDepth(float depth, float near, float far) {
    float z = depth * 2.f - 1.f; // NDC depth coord
    return (2.0 * near * far) / (far + near - z * (far - near));
}


