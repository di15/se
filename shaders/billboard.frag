
#version 120

uniform vec4 color;

//varying vec3 normalOut;
varying vec2 texCoordOut0;
uniform sampler2D texture0;

void main(void)
{
	gl_FragColor = color * texture2D(texture0, texCoordOut0);
}
