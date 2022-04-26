#type vertex
#version 450

layout(binding = 0) uniform Matrices {
	mat4 View;
	mat4 Projection;
	mat4 Transform;
} u_Matrices;

layout(location = 1) out vec3 NearPoint;
layout(location = 2) out vec3 FarPoint;
layout(location = 3) out mat4 FragProj;
layout(location = 7) out mat4 FragView;

// Grid position are in clipped space
vec3 gridPlane[6] = vec3[] (
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
);

vec3 UnprojectPoint(float x, float y, float z, mat4 view, mat4 projection) 
{
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(projection);
    vec4 unprojectedPoint =  viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() 
{
    FragProj = u_Matrices.Projection;
    FragView = u_Matrices.View;

    vec3 p = gridPlane[ gl_VertexIndex ].xyz;

    NearPoint = UnprojectPoint(p.x, p.y, 0.0, u_Matrices.View, u_Matrices.Projection).xyz; // unprojecting on the near plane
    FarPoint = UnprojectPoint(p.x, p.y, 1.0, u_Matrices.View, u_Matrices.Projection).xyz; // unprojecting on the far plane

    gl_Position = vec4( p, 1.0 );
    //gl_Position.y = -gl_Position.y; 

   // gl_Position = vec4( p, 1.0 );
}

#type fragment
#version 450

layout(location = 1) in vec3 NearPoint;
layout(location = 2) in vec3 FarPoint;
layout(location = 3) in mat4 FragProj;
layout(location = 7) in mat4 FragView;

layout(location = 0) out vec4 FinalColor;

float computeDepth(vec3 pos) {
    vec4 clip_space_pos = FragProj * FragView * vec4(pos.xyz, 1.0);
    return (clip_space_pos.z / clip_space_pos.w);
}

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;
    return color;
}

void main() 
{
    float t = -NearPoint.y / (FarPoint.y - NearPoint.y);

    vec3 fragPos3D = NearPoint + t * (FarPoint - NearPoint);

     gl_FragDepth = computeDepth( fragPos3D );

    FinalColor = grid(fragPos3D, 10) * float(t > 0);
}