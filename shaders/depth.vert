
#version 120

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

void main(void)
{
	gl_Position = projection * view * model * gl_Vertex;
	//gl_Position = projection * view * gl_Vertex;
	//gl_Position = vec4(0,0,0,1);
	//gl_Position.w = 1;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
