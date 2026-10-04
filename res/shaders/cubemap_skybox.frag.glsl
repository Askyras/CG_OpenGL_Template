out vec4 fcolor;

smooth in vec3 texcoords;

uniform samplerCube tex;

void main(void)
{
    vec4 tex_col = texture(tex, texcoords);
    tex_col.rgb *= tex_col.a; // pre-multiply alpha
    fcolor = tex_col;
}