#version 330 core

//vert attributes from the Cornell Box mesh
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

// values interpolated across all triangles for frag shader
out vec3 color;
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 lightSpacePosition;

// cam & light-space transforms
uniform mat4 MVP;
uniform mat4 lightSpaceMatrix;

void main() {
    //project vertex for cam view
    gl_Position =  MVP * vec4(vertexPosition, 1);
    
    //pass surface data to frag shader
    color = vertexColor;
    worldPosition = vertexPosition;
    worldNormal = normalize(vertexNormal);

    // save light-space pos for shadow-map
    lightSpacePosition = lightSpaceMatrix * vec4(vertexPosition, 1.0);
}
