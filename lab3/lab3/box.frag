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

	vec3 irradiance = vec3(1.0e6) * cosine / distanceSquared;
	finalColor = color * irradiance / PI;
}
