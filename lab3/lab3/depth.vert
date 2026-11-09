#version 330 core

// build the light-space depth map
layout(location = 0) in vec3 vertexPosition;

uniform mat4 MVP;

void main()
{
	// OpenGL writes depth to the depth attachment
	gl_Position = MVP * vec4(vertexPosition, 1.0);
}
