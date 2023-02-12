#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 shadowMatrix;

uniform vec3 lightPos;
uniform vec4 nodeColour;
uniform bool animate;
uniform bool transparent;
uniform bool map;

in vec3 position;
in vec4 colour;
in vec3 normal;
in vec4 tangent;
in vec2 texCoord;
in vec4 jointWeights;
in ivec4 jointIndices;

uniform mat4 joints[128];

out Vertex {
	vec4 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
	vec4 shadowProj;
	//extra param
	float height;
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

	vec4 worldPos = (modelMatrix * vec4 (finalPos, 1));
	OUT.worldPos = worldPos.xyz;
	gl_Position = (projMatrix * viewMatrix) * worldPos;
	OUT.height = finalPos.y;
	OUT.colour = nodeColour;
	OUT.texCoord = texCoord;
	//light
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	vec3 wNormal = normalize(normalMatrix * normalize(normal));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));
	OUT.normal = wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wNormal, wTangent) * tangent.w;
	//shadow
	vec3 viewDir = normalize(lightPos - worldPos.xyz);
	vec4 pushVal = vec4(OUT.normal , 0) * dot(viewDir, OUT.normal);
	OUT.shadowProj = shadowMatrix * (worldPos + pushVal);
}