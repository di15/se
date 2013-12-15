attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

void main(void)
{
	gl_Position = projection * (view * (model * position));
	texCoordOut0 = texCoordIn0;
}
