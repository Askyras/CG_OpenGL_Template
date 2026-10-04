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
uniform int debug_mode = 0; // 0: none, 1: normals, 2: texcoords.

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
    if (base_color.a < 0.001) {
        discard;
    }
    fcolor = base_color;
}