/// #shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
    TexCoords = aTexCoords;
}

/// #shader fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    // No effect
    FragColor = vec4(vec3(texture(screenTexture, TexCoords)), 1.0);

    // Color inversion
    // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    // Grayscale
    // FragColor = texture(screenTexture, TexCoords);
    // float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0;
    // float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    // FragColor = vec4(average, average, average, 1.0);

    // Kernel
//    const float offset = 1.0 / 300.0;
//
//    vec2 offsets[9] = vec2[](
//        vec2(-offset,  offset), // top-left
//        vec2( 0.0f,    offset), // top-center
//        vec2( offset,  offset), // top-right
//        vec2(-offset,  0.0f),   // center-left
//        vec2( 0.0f,    0.0f),   // center-center
//        vec2( offset,  0.0f),   // center-right
//        vec2(-offset, -offset), // bottom-left
//        vec2( 0.0f,   -offset), // bottom-center
//        vec2( offset, -offset)  // bottom-right
//    );
//
//    // sharpen kernel
//    float sharpenKernel[9] = float[](
//        -1, -1, -1,
//        -1,  9, -1,
//        -1, -1, -1
//    );
//
//    // blurry kernel
//    float blurryKernel[9] = float[](
//        1.0 / 16, 2.0 / 16, 1.0 / 16,
//        2.0 / 16, 4.0 / 16, 2.0 / 16,
//        1.0 / 16, 2.0 / 16, 1.0 / 16
//    );
//
//    // edge detection kernel
//    float edgeKernel[9] = float[](
//        1, 1, 1,
//        1, -8, 1,
//        1, 1, 1
//    );
//
//
//    vec3 sampleTex[9];
//    for (int i = 0; i < 9; i++) {
//        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
//    }
//
//    vec3 color = vec3(0.0);
//    for (int i = 0; i < 9; i++) {
//        color += edgeKernel[i] * sampleTex[i];
//    }
//
//    FragColor = vec4(color, 1.0);
}