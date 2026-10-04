out vec4 fcolor;

uniform vec4 color; // not pre-multiplied

void main(void)
{
    fcolor = vec4(color.rgb * color.a, color.a); // pre-multiplied
}