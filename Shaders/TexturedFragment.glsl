#version 330 core
uniform sampler2D diffuseTex;

in Vertex {
	vec3 worldPos;
	vec2 texCoord;
} IN;

out vec4 fragColour;
void main(void) {
	fragColour = texture(diffuseTex, IN.texCoord);
}