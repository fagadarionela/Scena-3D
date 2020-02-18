#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec2 fragTexCoords;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;

uniform	vec3 lightDir;
uniform	vec3 lightColor;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDir- fragPosEye.xyz);
	
	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
		
vec3 halfVector = normalize(lightDirN + viewDirN);
	//compute ambient light

//compute distance to light
float dist = length(lightDir- fragPosEye.xyz);
//compute attenuation
float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	ambient = att*ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = att*max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular = att*specularStrength * specCoeff * lightColor;
}

void main() 
{
	computeLightComponents();
	
	vec3 baseColor = vec3(1.0f, 0.55f, 0.0f);//orange
	
	ambient *=  texture(diffuseTexture, fragTexCoords);
	diffuse *=  texture(diffuseTexture, fragTexCoords);
	specular *=  texture(specularTexture, fragTexCoords);;
	
	vec3 color = min((ambient + diffuse) + specular, 1.0f);
    
    fColor = vec4(color, 1.0f);
}
