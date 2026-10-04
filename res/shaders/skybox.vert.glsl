layout(location = 0) in vec3 pos;

smooth out vec3 texcoords;

uniform mat4 view;
uniform mat4 proj;

void main(void)
{
    texcoords = pos;
    mat4 rotation = view;
    rotation[3] = vec4(0, 0, 0, 1);
    vec4 clip = proj * rotation * vec4(pos, 1);
    gl_Position = vec4(clip.xy, -clip.w, clip.w);
}