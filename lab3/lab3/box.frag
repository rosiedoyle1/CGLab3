#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 lightSpacePosition;

out vec3 finalColor;

// point-light params and depth map from light
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 ambientIntensity;
uniform sampler2D shadowMap;

// return reduced visibility when fragment is behind stored light depth
float visibility()
{
	// behind light or outside frustum means not shadowed
	if (lightSpacePosition.w <= 0.0) return 1.0;
	vec3 p = lightSpacePosition.xyz / lightSpacePosition.w * 0.5 + 0.5;
	if (any(lessThan(p, vec3(0.0))) || any(greaterThan(p, vec3(1.0))))
		return 1.0;

	float closest = texture(shadowMap, p.xy).r;
	return p.z >= closest + 1e-3 ? 0.2 : 1.0;
}

void main()
{
	const float PI = 3.14159265359;
	vec3 normal = normalize(worldNormal);

	// evaluate Lambertian point-light illumination with inverse-square falloff
	vec3 lightOffset = lightPosition - worldPosition;
	float distanceSquared = max(dot(lightOffset, lightOffset), 0.0001);
	vec3 lightDirection = lightOffset * inversesqrt(distanceSquared);
	float cosine = max(dot(normal, lightDirection), 0.0);

	// apply shadow to direct light and add unshadowed ambient fill
	vec3 irradiance = lightIntensity * cosine / distanceSquared * visibility();
	vec3 linearColor = color * (irradiance / PI + ambientIntensity);

	// map high-dynamic-range linear color
	const float exposure = 3.0;
	vec3 c = linearColor * exposure;
	c = c / (1.0 + c);
	finalColor = pow(c, vec3(1.0 / 2.2));
}
