// Invoke with glDrawArrays(GL_TRIANGLES, 0, 12) and an empty VAO bound

uniform mat4 view;
uniform mat4 proj;

const vec4 verts[5] = vec4[5](
    vec4( 0, 0, 0, 1),
    vec4( 1, 0, 0, 0),
    vec4( 0, 0, 1, 0),
    vec4(-1, 0, 0, 0),
    vec4( 0, 0,-1, 0)
);
const int indices[12] = int[12](
    0,1,2,
    0,2,3,
    0,3,4,
    0,4,1
);

void main(void)
{
    vec4 pos = verts[indices[gl_VertexID]];
    gl_Position = proj * view * pos;
}