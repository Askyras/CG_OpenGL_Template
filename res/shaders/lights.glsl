struct Light {
    vec3 pos;      // Worldspace light position. Point/spot only
    int type;      // Light type
    vec3 col;      // Light color (RGB) multiplied by intensity (so >1 is allowed).
    float falloff; // Attenuation factor for distance-based falloff. 0 = infinite range, 1 = normal inverse square falloff. Point/spot only.
    vec3 dir;      // Light direction (must be normalized). Directional/spot only.
    float phi;     // Cosine of spotlight cutoff angle (angle between direction and edge of light cone). Spot only.
};

layout(std140) uniform Lights {
    uint n_lights;
    // 12b padding
    Light lights[MAX_LIGHTS];
};