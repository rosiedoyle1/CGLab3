#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTextureCoordinate;

out vec3 color;
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 lightSpacePosition;
out vec3 textureCoordinateAndFlag;

uniform mat4 MVP;
uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = MVP * vec4(vertexPosition, 1.0);
	color = vertexColor;
	worldPosition = vertexPosition;
	worldNormal = normalize(vertexNormal);
	lightSpacePosition = lightSpaceMatrix * vec4(vertexPosition, 1.0);
	textureCoordinateAndFlag = vertexTextureCoordinate;
}
