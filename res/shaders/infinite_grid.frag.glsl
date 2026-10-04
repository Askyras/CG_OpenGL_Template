out vec4 fcolor;

uniform mat4 view;
uniform mat4 proj;
uniform vec2 viewport; // glViewport size
uniform vec3 cam_pos; // world-space camera position
uniform vec3 cam_offset;

const float grid_res[3] = float[3](0.1, 1.0, 10.0);
const float grid_line_widths[3] = float[3](0.003, 0.01, 0.02);
const vec4 grid_colors[3] = vec4[3](vec4(1.,1.,1.,.5), vec4(1.0), vec4(1.0));

// Pristine grid from The Best Darn Grid Shader (yet)
// https://bgolus.medium.com/the-best-darn-grid-shader-yet-727f9278b9d8
float pristineGrid(vec2 uv, float lineWidth)
{
    vec2 ddx = dFdx(uv);
    vec2 ddy = dFdy(uv);
    vec2 uvDeriv = vec2(length(vec2(ddx.x, ddy.x)), length(vec2(ddx.y, ddy.y)));
    bvec2 invertLine = bvec2(lineWidth > 0.5, lineWidth > 0.5);
    vec2 targetWidth = vec2(
      invertLine.x ? 1.0 - lineWidth : lineWidth,
      invertLine.y ? 1.0 - lineWidth : lineWidth
      );
    vec2 drawWidth = clamp(targetWidth, uvDeriv, vec2(0.5));
    vec2 lineAA = uvDeriv * 1.5;
    vec2 gridUV = abs(fract(uv) * 2.0 - 1.0);
    gridUV.x = invertLine.x ? gridUV.x : 1.0 - gridUV.x;
    gridUV.y = invertLine.y ? gridUV.y : 1.0 - gridUV.y;
    vec2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);

    grid2 *= clamp(targetWidth / drawWidth, 0.0, 1.0);
    grid2 = mix(grid2, targetWidth, clamp(uvDeriv * 2.0 - 1.0, 0.0, 1.0));
    grid2.x = invertLine.x ? 1.0 - grid2.x : grid2.x;
    grid2.y = invertLine.y ? 1.0 - grid2.y : grid2.y;
    return mix(grid2.x, 1.0, grid2.y);
}

// Single line snippet from the same blog post
float pristineLine(float u, float lineWidth)
{
    float uvDeriv = fwidth(u);
    float drawWidth = max(lineWidth, uvDeriv);
    float lineAA = uvDeriv * 1.5;
    float lineUV = abs(u * 2.0);
    float line = smoothstep(drawWidth + lineAA, drawWidth - lineAA, lineUV);
    line *= clamp(lineWidth / drawWidth, 0.0, 1.0);
    return line;
}

vec3 wposInfiniteReverseZ(vec2 ndc)
{
    vec3 vrd = normalize(vec3(
        ndc.x / proj[0][0],
        ndc.y / proj[1][1],
        -1.0
    ));
    vec3 rd = normalize(inverse(mat3(view)) * vrd);
    float denom = rd.y;
    if (abs(denom) < 1e-9)
        discard;
    float t = -cam_pos.y / denom;
    if (t < 0.0)
        discard;
    return cam_pos + t * rd;
}

void main(void)
{
    // Reconstruct world position of fragment by raycasting against the infinite plane
    // The construction is tied to the type of camera projection used
    vec2 ndc = (gl_FragCoord.xy / viewport) * 2.0 - 1.0;
    vec3 wpos = wposInfiniteReverseZ(ndc);
    vec2 uv = wpos.xz;

    fcolor = vec4(fract(uv / 10), 0, 1);
    //return;

    // Composite the grid levels with premultiplied alpha, finer levels under coarser ones
    float grid_0 = pristineGrid(uv / grid_res[0], grid_line_widths[0] / grid_res[0]);
    float grid_1 = pristineGrid(uv / grid_res[1], grid_line_widths[1] / grid_res[1]);
    float grid_2 = pristineGrid(uv / grid_res[2], grid_line_widths[2] / grid_res[2]);
    vec4 color_0 = vec4(grid_colors[0].rgb, 1.0) * (grid_0 * grid_colors[0].a);
    vec4 color_1 = vec4(grid_colors[1].rgb, 1.0) * (grid_1 * grid_colors[1].a);
    vec4 color_2 = vec4(grid_colors[2].rgb, 1.0) * (grid_2 * grid_colors[2].a);
    vec4 color = color_0;
    color = color_1 + (color * (1.0 - color_1.a));
    color = color_2 + (color * (1.0 - color_2.a));

    #if 1
    // Add coloured X and Z axes
    float x_axis = pristineLine(uv.y / grid_res[2], grid_line_widths[2] / grid_res[2]);
    float z_axis = pristineLine(uv.x / grid_res[2], grid_line_widths[2] / grid_res[2]);
    vec4 color_x = vec4(1.0, 0.0, 0.0, 1.0) * x_axis;
    vec4 color_z = vec4(0.0, 0.0, 1.0, 1.0) * z_axis;
    color = color_z + (color * (1.0 - color_z.a));
    color = color_x + (color * (1.0 - color_x.a));
    #endif

    #if 1
    // Fade out the grid close to the camera
    color *= 0.05 + 0.95 * smoothstep(0.1, 3.0, length(wpos - cam_pos));
    #endif

    fcolor = color; // premultiplied

    if (fcolor.a < 0.001) {
        discard;
    }
}