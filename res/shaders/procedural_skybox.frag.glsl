out vec4 fcolor;

smooth in vec3 texcoords;

vec3 skybox1(vec3 ro, vec3 rd)
{
    return mix(vec3(0.2), vec3(0., 0.2, 1.), smoothstep(-0.05, 0.05, rd.y));
}

void main(void)
{
    vec3 rd = normalize(texcoords);
    vec3 c = mix(vec3(0.2), vec3(0., 0.2, 1.), smoothstep(-0.05, 0.05, rd.y));
    fcolor = vec4(c, 1);
}