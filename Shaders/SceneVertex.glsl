#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec4 nodeColour;
uniform bool animate;
uniform bool transparent;

in vec3 position;
in vec2 texCoord;
in vec3 normal;
in vec4 jointWeights;
in ivec4 jointIndices;

uniform mat4 joints[128];

out Vertex {
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
	vec3 worldPos;
} OUT;

void main(void) {
	vec3 finalPos = position;
	if (animate) {
		//animate mesh need calculate position
		vec4 localPos = vec4(position, 1.0f);
		vec4 skelPos = vec4(0 ,0 ,0 ,0);

		for (int i = 0; i < 4; ++ i) {
			int jointIndex = jointIndices[i];
			float jointWeight = jointWeights[i];
			skelPos += joints[jointIndex] * localPos * jointWeight;
		}
		finalPos = skelPos.xyz;
	}
	gl_Position = (projMatrix * viewMatrix * modelMatrix) * vec4(finalPos, 1.0);
	OUT.texCoord = texCoord;
	OUT.colour = nodeColour;
	//light
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	OUT.normal = normalize(normalMatrix * normalize(normal));
	vec4 worldPos = modelMatrix * vec4(finalPos, 1);
	OUT.worldPos = worldPos.xyz;
}