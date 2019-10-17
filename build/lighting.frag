#version 330 core

struct LightInfo
{
	vec3 LPos;			//light position
	vec3 LCol;			//light color
	vec3 La;			//Ambient light intensity
	vec3 Ld;			//Diffuse light intensity
	vec3 Ls;			//Specular light intensity
	float SpecShine;	//Specular shininess factor
};

in vec3 EyePosition;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TBN;

out vec4 FragColor;

uniform LightInfo Light[2];
uniform sampler2D Tex;
uniform sampler2D Norm;

void light(int lightNo, vec3 position, vec3 normal, out vec3 ambient, out vec3 diffuse, out vec3 specular)
{
	vec3 n = normalize(normal);
    vec3 l = normalize(Light[lightNo].LPos - position);
    vec3 e = normalize(-position);
    vec3 r = reflect(-l, n);
 
	//ambient
    ambient = Light[lightNo].La *Light[lightNo].LCol; //prob will change to texture

    //diffuse
    float diff = max(dot(l,n),0.0f);
    diffuse = Light[lightNo].Ld * diff * Light[lightNo].LCol;

	//specular
	float spec = pow(max(dot(r,e),0.0f),Light[lightNo].SpecShine);
	specular = Light[lightNo].Ls * spec * Light[lightNo].LCol;
}

void main()
{
	vec3 ambientFinal = vec3(0.0f);
	vec3 diffuseFinal= vec3(0.0f);
	vec3 specularFinal = vec3(0.0f);
	vec3 ambientCal, diffuseCal, specularCal;

	//vec3 Normal = texture(Norm, TexCoord).rgb;
	//Normal = normalize(Normal*2.0-1.0);
	//Normal = normalize(TBN * Normal); 

	for( int i=0; i<2; ++i )
	{
		light(i, EyePosition, Normal, ambientCal, diffuseCal, specularCal);
		ambientFinal += ambientCal;
		diffuseFinal += diffuseCal;
		specularFinal += specularCal;
	}

	ambientFinal /= 2;

	vec4 texColor = texture(Tex, TexCoord);
	FragColor = vec4(ambientFinal + diffuseFinal, 1.0f) * texColor + vec4(specularFinal, 1.0f);
}

