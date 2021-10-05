#type vertex

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 u_Transform;

void main()
{
	gl_Position = u_Transform * vec4( aPos, 1.0 );
	TexCoord = aTexCoord;
}

#type fragment

#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
	FragColor = texture(ourTexture, TexCoord);
}