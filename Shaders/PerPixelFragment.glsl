#version 330 core

uniform sampler2D diffuseTex;
uniform sampler2D diffuseTex1;

uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;

in Vertex {
	vec3 colour;
	vec2 texCoord;
	vec3 normal;
	vec3 worldPos;

	float height;
} IN;

out vec4 fragColour;
void main(void) {
	vec3 incident = normalize(lightPos - IN.worldPos);
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	vec4 diffuse = texture(diffuseTex, IN.texCoord);

	if (IN.height <= 120.0f) {
		diffuse = texture(diffuseTex1, IN.texCoord);
		float lambert = max(dot(incident, IN.normal), 0.0f);
		float dist = length(lightPos - IN.worldPos);
		float attenuation = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
		
		float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
		specFactor = pow(specFactor, 60.0);
		
		vec3 surface = (diffuse.rgb * lightColour.rgb);
		fragColour.rgb = surface * lambert * attenuation;
		fragColour.rgb += (lightColour.rgb * specFactor) * attenuation * 0.33;
		fragColour.rgb += surface * 0.1f;
		fragColour.a = diffuse.a;
	}
	else {
		float lambert = max(dot(incident, IN.normal), 0.0f);
		float distance = length(lightPos - IN.worldPos);
		float attenuation = 1.0 - clamp(distance/ lightRadius, 0.0, 1.0);
		float specFactor = clamp(dot(halfDir, IN.normal), 0.0, 1.0);
		specFactor = pow(specFactor, 60.0);
		vec3 surface = (diffuse.rgb * lightColour.rgb);
		fragColour.rgb = surface * lambert * attenuation;
		fragColour.rgb += (lightColour.rgb * specFactor) * attenuation*0.8;
		fragColour.rgb += surface * 0.1f;
		fragColour.a = diffuse.a;
	}
}