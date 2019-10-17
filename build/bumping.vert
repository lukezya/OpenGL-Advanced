#version 330 core

in vec3 VertexPosition; //local space
in vec3 VertexNormal; //local space
in vec2 VertexTex;
in vec3 VertexTangent;
in vec3 VertexBitangent;

out vec3 EyePosition;
out vec2 TexCoord;
out vec3 TangentLightPos1;
out vec3 TangentLightPos2;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

uniform mat4 MVP;
uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
//uniform vec3 viewPos;

void main()
{
    //Normal = NormalMatrix*VertexNormal;
    EyePosition = vec3(ModelViewMatrix*vec4(VertexPosition,1.0f));
	gl_Position = MVP*vec4(VertexPosition,1.0f);
	TexCoord = VertexTex;

	vec3 T = normalize(NormalMatrix * VertexTangent);
    vec3 N = normalize(NormalMatrix * VertexNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));    
    TangentLightPos1 = TBN * lightPos1;
	TangentLightPos2 = TBN * lightPos2;
    TangentViewPos  = TBN*vec3(1.0f);
    TangentFragPos  = TBN * EyePosition;
}
