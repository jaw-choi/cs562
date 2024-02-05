/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

in vec4 vertex;
in vec2 vertexTexture;
in vec3 vertexNormal;
in vec3 vertexTangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;
void main()
{
    TexCoords = vertexTexture;
    gl_Position = projection * view * model * vertex;
}