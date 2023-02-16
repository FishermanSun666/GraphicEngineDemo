#version 330 core
uniform sampler2D sceneTex;
uniform int isVertical;
uniform int distJudge;

in Vertex {
	vec3 worldPos;
	vec2 texCoord;
} IN;

out vec4 fragColor;

const float scaleFactors [7] = float [](0.006 , 0.061 , 0.242 , 0.383 , 0.242 , 0.061 , 0.006);

void main ( void ) {
	if (distJudge > IN.worldPos.y) {
		fragColor = texture2D(sceneTex, IN.texCoord.xy);
		return;
	}
	fragColor = vec4(0 ,0 ,0 ,1);
	vec2 delta = vec2(0 ,0);
	if(isVertical == 1) {
		delta = dFdy(IN.texCoord);
	} else {
		delta = dFdx(IN.texCoord);
	}
	for(int i = 0; i < 7; i ++ ) {
		vec2 offset = delta * ( i - 3);
		vec4 tmp = texture2D(sceneTex, IN.texCoord.xy + offset);
		fragColor += tmp * scaleFactors[i];
	}
}