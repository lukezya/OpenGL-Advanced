#version 330 core

in vec3 VertexPosition; //local space
in vec3 VertexNormal; //local space
in vec2 VertexTex;
in vec3 VertexTangent;
in vec3 VertexBitangent;

out vec3 EyePosition;
out vec3 Normal;
out vec2 TexCoord;
out mat3 TBN;

uniform mat4 MVP;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

void main()
{
    Normal = NormalMatrix*VertexNormal;
    EyePosition = vec3(ModelViewMatrix*vec4(VertexPosition,1.0f));
	gl_Position = MVP*vec4(VertexPosition,1.0f);
	TexCoord = VertexTex;

	vec3 T = normalize(vec3(ModelMatrix * vec4(VertexTangent, 0.0)));
	vec3 B = normalize(vec3(ModelMatrix * vec4(VertexBitangent, 0.0)));
	vec3 N = normalize(vec3(ModelMatrix * vec4(VertexNormal, 0.0)));
	TBN = mat3(T, B, N);
}

