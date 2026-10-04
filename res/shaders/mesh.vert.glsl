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

void main(void)
{
	wpos = (model * vec4(pos, 1.0)).xyz;
	wnormal = normalize(inverse(transpose(mat3(model))) * normal);
	uv = texcoord;
	#ifdef VERTEX_COLOR
	vcolor = color;
	#endif
	gl_Position = proj * view * vec4(wpos, 1.0);
}