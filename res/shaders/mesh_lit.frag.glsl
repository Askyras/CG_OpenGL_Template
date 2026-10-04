#include "shaders/lights.glsl"

out vec4 fcolor;

smooth in vec3 wpos;
smooth in vec3 wnormal;
smooth in vec2 uv;
#ifdef VERTEX_COLOR
smooth in vec3 vcolor;
#endif

uniform sampler2D tex; // texture not pre-multiplied
uniform int has_texture = 0;
uniform vec4 color = vec4(0); // tint, not pre-multiplied
uniform int debug_mode = 0; // 0: none, 1: normals, 2: texcoords, 3: unlit
uniform vec3 cam_pos;
uniform vec3 ka = vec3(0.1);
uniform vec3 kd = vec3(0.9);
uniform vec3 ks = vec3(1.0);

vec3 phongLighting(vec3 base_color, vec3 ka, vec3 kd, vec3 ks)
{
    vec3 normal = normalize(wnormal);
    vec3 view_dir = normalize(wpos - cam_pos);
    vec3 rv = reflect(view_dir, normal);
    vec3 ambient = vec3(0.0);
    vec3 diffuse = vec3(0.0);
    vec3 specular = vec3(0.0);
    for (int i = 0; i < n_lights; i++) {
        vec3 ld = normalize(lights[i].pos - wpos);
        vec3 h = normalize(ld - view_dir);
        float d = lights[i].falloff * length(lights[i].pos - wpos);
        float falloff = 1.0 / (1.0 + d * d);
        if (lights[i].type == 1) { // Directional
            ld = -lights[i].dir;
            falloff = 1;
        }
        else if (lights[i].type == 2) { // Spotlight
            falloff *= smoothstep(lights[i].phi - 1.0, 1.0 - lights[i].phi, dot(ld, -lights[i].dir) - lights[i].phi);
        }
        vec3 radiance = lights[i].col * falloff;
        ambient += radiance;
        diffuse += max(dot(normal, ld), 0.0) * radiance;
        specular += pow(max(dot(rv, ld), 0.0), 32) * radiance;
    }
    return base_color * (ka * ambient + kd * diffuse) + ks * specular;
}

void main(void)
{
#ifdef DEBUG
    if (debug_mode == 1) {
        fcolor = vec4(wnormal, 1);
        return;
    } else if (debug_mode == 2) {
        fcolor = vec4(uv, 0, 1);
        return;
    }
#endif
    vec4 base_color;
#ifdef VERTEX_COLOR
    base_color = mix(vcolor, color.rgb, color.a); // use normal blending for the tint
#else
    if (has_texture == 1) {
        base_color = texture(tex, uv);
        base_color.rgb = mix(base_color.rgb, color.rgb, color.a); // use normal blending for the tint
    }
    else {
        base_color = color;
    }
#endif
    base_color.rgb *= base_color.a; // pre-multiply result
#ifdef DEBUG
    if (debug_mode == 3) {
        fcolor = base_color;
        return;
    }
#endif
    if (base_color.a < 0.001) {
        discard;
    }
    base_color.rgb = phongLighting(base_color.rgb, ka, kd, ks);
    fcolor = base_color;
}