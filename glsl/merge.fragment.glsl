#version 330 core

out vec4 color;

uniform sampler2D Color1;
uniform sampler2D Depth1;
uniform sampler2D Color2;
uniform sampler2D Depth2;

void main(){
	ivec2 texcoord = ivec2(floor(gl_FragCoord.xy));

	vec4 color1 = texelFetch(Color1, texcoord, 0);
	float depth1 = texelFetch(Depth1, texcoord, 0).r;
	vec4 color2 = texelFetch(Color2, texcoord, 0);
	float depth2 = texelFetch(Depth2, texcoord, 0).r;

    if (depth1 < depth2) {
        //color = (color1 + vec4(vec3(depth1), 1.0))/2.0;
        color = color1;
        gl_FragDepth = depth1;
    } else {
        //color = (color2 + vec4(vec3(depth2), 1.0))/2.0;
        color = color2;
        gl_FragDepth = depth2;
    }
}