#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 lightSpacePosition;

out vec3 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform sampler2D shadowMap;

// 1.0 = lit, 0.0 = in shadow
float visibility()
{
	if (lightSpacePosition.w <= 0.0) return 1.0;            // behind the light camera
	vec3 p = lightSpacePosition.xyz / lightSpacePosition.w * 0.5 + 0.5;
	if (any(lessThan(p, vec3(0.0))) || any(greaterThan(p, vec3(1.0))))
		return 1.0;                                          // outside the shadow map
	float closest = texture(shadowMap, p.xy).r;
	return (p.z - 1e-5 > closest) ? 0.0 : 1.0;               // tiny bias (depthNear = 10)
}

void main()
{
	const float PI = 3.14159265359;
	vec3 normal = normalize(worldNormal);
	vec3 lightOffset = lightPosition - worldPosition;
	float distanceSquared = max(dot(lightOffset, lightOffset), 0.0001);
	vec3 lightDirection = lightOffset * inversesqrt(distanceSquared);
	float cosine = max(dot(normal, lightDirection), 0.0);

	// Shadow blocks the light, so it removes irradiance BEFORE tone mapping
	vec3 irradiance = lightIntensity * cosine / distanceSquared * visibility();
	vec3 linearColor = color * irradiance / PI;

	const float exposure = 3.0;
	vec3 c = linearColor * exposure;
	c = c / (1.0 + c);                     // Reinhard
	finalColor = pow(c, vec3(1.0 / 2.2));  // gamma
}
