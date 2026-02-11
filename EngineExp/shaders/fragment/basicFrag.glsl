#version 330

out vec4 FragColor;

in vec3 colour;
in vec2 texCoord;

flat in int face;
flat in int block;

uniform sampler2D ourTexture;


void main(void) {
    // 1. Configuration for Bottom-Left of a 2x2 grid
    vec2 subRegionOrigin = vec2(0, 0);
    vec2 subRegionSize   = vec2(0.5, 0.5);

    if (block == 1) { subRegionOrigin = vec2(0.5, 0); }
    if (block == 2) { subRegionOrigin = vec2(0, 0.5); }
    if (block == 3) { subRegionOrigin = vec2(0.5, 0.5); }

    // 2. Apply wrapping logic
    // We use fract() to ensure the UVs loop within the subregion
     vec2 wrappedUV = fract(texCoord);

    // 3. Map the wrapped 0-1 coordinates into the bottom-left quadrant
     vec2 finalUV = subRegionOrigin + (wrappedUV * subRegionSize);

    // 4. Sample using textureGrad
    // Standard 'texture()' will create a visible seam line where fract() resets.
    // textureGrad tells the GPU to ignore the 'jump' in coordinates for mipmapping.
     vec4 texColor = textureGrad(ourTexture, finalUV, dFdx(texCoord), dFdy(texCoord));

    // 5. Apply your face-based shading
     float shade = float(face) * 0.167 + .2;
     FragColor = texColor * vec4(shade, shade, shade, 1.0);

}