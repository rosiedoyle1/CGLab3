#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal; 
in vec4 lightSpacePosition;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D shadowMap;

void main()
{
	const float PI = 3.14159265359;
	vec3 normal = normalize(worldNormal);
	vec3 lightOffset = lightPosition - worldPosition;
	float distanceSquared = max(dot(lightOffset, lightOffset), 0.0001);
	vec3 lightDirection = lightOffset * inversesqrt(distanceSquared);
	float cosine = max(dot(normal, lightDirection), 0.0);

	vec3 irradiance = lightIntensity * cosine / distanceSquared;
	vec3 linearColor = color * irradiance / PI;

	const float exposure = 1.0;
	vec3 exposedColor = linearColor * exposure;
	vec3 toneMappedColor = exposedColor / (vec3(1.0) + exposedColor);

	vec3 projectedPosition = lightSpacePosition.xyz / lightSpacePosition.w;
	vec3 shadowCoordinates = projectedPosition * 0.5 + 0.5;
	float shadow = 1.0;
	if (all(greaterThanEqual(shadowCoordinates, vec3(0.0))) &&
		all(lessThanEqual(shadowCoordinates, vec3(1.0)))) {
		float existingDepth = texture(shadowMap, shadowCoordinates.xy).r;
		shadow = shadowCoordinates.z >= existingDepth + 1e-3 ? 0.2 : 1.0;
	}

	finalColor = pow(toneMappedColor, vec3(1.0 / 2.2)) * shadow;
}
