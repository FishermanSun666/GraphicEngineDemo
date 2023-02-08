#version 330 core
uniform sampler2D roleTex;
uniform sampler2D treeTex;
uniform sampler2D cloudTex;
uniform int shadeType;

in Vertex {
	vec2 texCoord;
	vec4 colour;
} IN;

out vec4 fragColour;
void main(void) {
	fragColour = IN.colour;
	if (0 == shadeType) {
		fragColour = texture(roleTex, IN.texCoord);
	} 
	else if(1 == shadeType) {
		fragColour = texture(treeTex, IN.texCoord);
	} 
	else {
		fragColour *= texture(cloudTex, IN.texCoord);
	}
}