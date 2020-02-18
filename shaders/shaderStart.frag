#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec4 fragPosLightSpace;
in vec2 fragTexCoords;

out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform	vec3 lightDir;
uniform vec3 lightPoint0;
uniform vec3 lightPoint1;
uniform vec3 lightPoint2;
uniform vec3 lightPoint3;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform vec3 lightposition;
uniform vec3 camDir;
uniform bool ok1;
uniform bool ok2;
uniform bool ok3;

float ambientStrength = 0.2f;



float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 1.0f;
float linear = 0.7f;
float quadratic = 1.8f;

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

vec3 computeLightComponents()
{		
vec3 ambient;
vec3 diffuse;
vec3 specular;
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
		ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	specular *= vec3(texture(specularTexture, fragTexCoords));
	float shadow = computeShadow();

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    return color;
}

vec3 CalcPointLight(vec3 lightPoint,vec3 LColor)
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightPointN = normalize(lightPoint-fragPosEye.xyz);
	
	//compute ambient light
	//compute distance to light
	float dist = length(lightPoint-fragPosEye.xyz);
	//compute attenuation
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
	ambient = att*ambientStrength * LColor;
	
	//compute diffuse light
	diffuse = att*max(dot(normalEye, lightPointN), 0.0f) * LColor;
	
	//compute specular light
	vec3 reflection = reflect(-lightPointN, normalEye);
	float specCoeff = pow(max(dot(normalEye, reflection), 0.0f), shininess);
	specular = att*specularStrength * specCoeff * LColor;
	
	ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	specular *= vec3(texture(specularTexture, fragTexCoords));
	
	//float shadow = 0.0f;
    vec3 color = min(ambient + diffuse + specular, 1.0f);
    return color;
}



float cutOff = 0.5f;
float outerCutOff = 0.2f;
vec3 CalcSpotLight( vec3 light,vec3 LColor )
{
	vec3 ambient = ambientStrength * vec3(texture(diffuseTexture, fragTexCoords));
    // Diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(light - fragPosEye.xyz);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse =  diff * vec3(texture(diffuseTexture, fragTexCoords));
    
    // Specular
    vec3 viewDir = normalize(camDir - fragPosEye.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(texture(specularTexture, fragTexCoords));
    
    // Spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light));
    float epsilon = (cutOff - outerCutOff);
    float intensity = clamp((theta - outerCutOff) / epsilon, 0.0, 1.0);
    diffuse  *= intensity;
    specular *= intensity;
    
    // Attenuation
    float distance    = length(light - fragPosEye.xyz);
    float attenuation = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    ambient  *= attenuation*LColor;
    diffuse  *= attenuation*LColor;
    specular *= attenuation*LColor;
    
    vec3 color = min(ambient + diffuse + specular, 1.0f);

	return color;
}

float computeFog()
{
 float fogDensity = 0.05f;
 float fragmentDistance = length(fragPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	//computeLightComponents();
	//CalcPointLight();
	//float shadow = computeShadow();
	
	//modulate with diffuse map
	//ambient *= vec3(texture(diffuseTexture, fragTexCoords));
	//diffuse *= vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	//specular *= vec3(texture(specularTexture, fragTexCoords));
	

	//modulate with shadow
	//vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
    vec3 result;
	vec3 culoare = vec3(1.0f,1.0f,0.0f);
	vec3 culoare2 = vec3(0.0f,1.0f,1.0f);
	vec3 alb = vec3(1.0f,1.0f,1.0f);
	vec3 roz = vec3(1.0f,0.0f,1.0f);
	vec3 culoare4 = vec3(1.0f,0.0f,1.0f);
	
	//vec3 lightSpot0 = vec3(1.33f, 2.6f, 1.1f);
	vec3 lightSpot0 = vec3(0.0f, 0.0f, 0.0f);

	if (ok1 == true){
		result += computeLightComponents();
	}
	if (ok2 == true){
		result+=CalcPointLight(lightPoint0,culoare); 
	}
	if (ok3 == true){
		result+=CalcSpotLight(lightSpot0,roz);
	}
	//result+=CalcPointLight(lightPoint1,culoare2);
	//result+=CalcPointLight(lightPoint2,roz);
	//result+=CalcPointLight(lightPoint3,culoare4);
    fColor = vec4(result, 1.0f);
vec3 color = result;
    //fColor = vec4(o, 1.0f);
float fogFactor = computeFog();
vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
fColor = fogColor*(1-fogFactor) + vec4(color*fogFactor,0.0f);


}
