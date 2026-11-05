#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal; 

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;

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
	finalColor = pow(toneMappedColor, vec3(1.0 / 2.2));
}
