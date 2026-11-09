#version 330 core

// interpolated mesh attributes, texture coordinates, and wood flag
in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;
in vec4 lightSpacePosition;
in vec3 textureCoordinateAndFlag;

out vec3 finalColor;

// point light, ambient fill, shadow map, & wood texture
uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform vec3 ambientIntensity;
uniform sampler2D shadowMap;
uniform sampler2D woodTexture;

// less visibility when frag depth is behind light depth
float visibility()
{
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

	// diffuse point-light illum
	vec3 lightOffset = lightPosition - worldPosition;
	float distanceSquared = max(dot(lightOffset, lightOffset), 0.0001);
	vec3 lightDirection = lightOffset * inversesqrt(distanceSquared);
	float cosine = max(dot(normal, lightDirection), 0.0);

	//apply the wood texture
	vec3 woodColor = texture(woodTexture, textureCoordinateAndFlag.xy).rgb;
	vec3 albedo = mix(color, color * woodColor, clamp(textureCoordinateAndFlag.z, 0.0, 1.0));

	// shadow direct illumination only; ambient fill remains visible
	vec3 irradiance = lightIntensity * cosine / distanceSquared * visibility();
	vec3 linearColor = albedo * (irradiance / PI + ambientIntensity);

	//exposure, tone mapping & sRGB encoding
	const float exposure = 3.0;
	vec3 mappedColor = linearColor * exposure;
	mappedColor = mappedColor / (vec3(1.0) + mappedColor);
	finalColor = pow(mappedColor, vec3(1.0 / 2.2));
}