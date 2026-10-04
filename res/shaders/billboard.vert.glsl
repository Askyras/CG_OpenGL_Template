layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
#ifdef VERTEX_COLOR
layout(location = 3) in vec3 color;
#endif

smooth out vec3 wpos;
smooth out vec3 wnormal;
smooth out vec2 uv;
#ifdef VERTEX_COLOR
smooth out vec3 vcolor;
#endif

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void sphericalBillboard()
{
    // Camera's world-space basis vectors
    mat3 cam_world = transpose(mat3(view));

    //mat3 linear = cam_world * mat3(scale)
    //wpos = translation + cam_world * linear;
    //wnormal = normalize(inverse(transpose(linear)) * normal);

    vec3 world_scale = vec3(length(model[0].xyz), length(model[1].xyz), length(model[2].xyz));
    wpos = model[3].xyz + cam_world * (pos * world_scale);
    wnormal = normalize(cam_world * (normal / world_scale));
    uv = texcoord;
	#ifdef VERTEX_COLOR
	vcolor = color;
	#endif

    gl_Position = proj * view * vec4(wpos, 1);
}

void main(void)
{
    sphericalBillboard();
}