#version 330 core

out vec4 color;

uniform sampler2D Color;
uniform sampler2D Depth;

void main(){
	ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));

    color = texelFetch(Color, texcoord, 0);
    gl_FragDepth = gl_FragCoord.z;
}