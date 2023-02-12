#version 330 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform vec4 colour;
uniform bool animate;
uniform bool transparent;

in vec3 position;
in vec4 jointWeights;
in ivec4 jointIndices;
uniform mat4 joints[128];

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
	vec4 worldPos = (modelMatrix * vec4 (finalPos, 1));
	gl_Position = (projMatrix * viewMatrix) * worldPos;
}