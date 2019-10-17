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
//in vec3 Normal;
in vec2 TexCoord;
in vec3 TangentLightPos1;
in vec3 TangentLightPos2;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

out vec4 FragColor;

uniform LightInfo Light[2];
uniform sampler2D Tex;
uniform sampler2D Norm;

/*void light(int lightNo, vec3 position, vec3 normal, out vec3 ambient, out vec3 diffuse, out vec3 specular)
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
}*/

void main()
{
	vec3 ambientFinal = vec3(0.0f);
	vec3 diffuseFinal= vec3(0.0f);
	vec3 specularFinal = vec3(0.0f);
	vec3 ambientCal, diffuseCal, specularCal;

	vec3 Normal = texture(Norm, TexCoord).rgb;
	Normal = normalize(Normal*2.0-1.0);

	vec3 n = normalize(Normal);
    vec3 l = normalize(TangentLightPos1 - TangentFragPos);
    vec3 e = normalize(-TangentFragPos);
    vec3 r = reflect(-l, n);

	//ambient
    ambientCal = Light[0].La *Light[0].LCol; //prob will change to texture

    //diffuse
    float diff = max(dot(l,n),0.0f);
    diffuseCal = Light[0].Ld * diff * Light[0].LCol;

	//specular
	float spec = pow(max(dot(r,e),0.0f),Light[0].SpecShine);
	specularCal = Light[0].Ls * spec * Light[0].LCol;

	ambientFinal += ambientCal;
	diffuseFinal += diffuseCal;
	specularFinal += specularCal;

	vec3 ambientCal2, diffuseCal2, specularCal2;
	vec3 n2 = normalize(Normal);
    vec3 l2 = normalize(TangentLightPos2 - TangentFragPos);
    vec3 e2 = normalize(-TangentFragPos);
    vec3 r2 = reflect(-l2, n2);

	//ambient
    ambientCal2 = Light[1].La *Light[1].LCol; //prob will change to texture

    //diffuse
    float diff2 = max(dot(l2,n2),0.0f);
    diffuseCal2 = Light[1].Ld * diff2 * Light[1].LCol;

	//specular
	float spec2 = pow(max(dot(r2,e2),0.0f),Light[1].SpecShine);
	specularCal2 = Light[1].Ls * spec2 * Light[1].LCol;

	ambientFinal += ambientCal2;
	diffuseFinal += diffuseCal2;
	specularFinal += specularCal2;

	ambientFinal /= 2;

	vec4 texColor = texture(Tex, TexCoord);
	FragColor = vec4(ambientFinal + diffuseFinal, 1.0f) * texColor + vec4(specularFinal, 1.0f);
}
