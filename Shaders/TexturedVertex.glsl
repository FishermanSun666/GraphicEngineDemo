#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;

out Vertex {
	vec3 worldPos;
	vec2 texCoord;
} OUT;
void main(void) {
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;
	vec4 worldPos = mvp * vec4(position, 1.0);
	gl_Position = worldPos;
	OUT.worldPos = worldPos.xyz;
	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
}