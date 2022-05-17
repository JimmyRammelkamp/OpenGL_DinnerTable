// FRAGMENT SHADER

#version 330

in vec4 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord0;
in mat3 matrixTangent;

out vec4 outColor;

// View Matrix
uniform mat4 matrixView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

uniform sampler2D texture0;
uniform sampler2D textureNormal;
//uniform sampler2D textureAO;

uniform bool useNormalMap = false;

vec3 normalNew;


//Point
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;

vec4 PointLight(POINT light)
{
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = (normalize((matrixView * vec4(light.position, 1)) - position)).xyz;
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
		color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	float dist = length(matrixView * vec4(light.position, 1) - position);
	float att = 1 / (0.03 * dist * dist); //att_quadratic hard coded to 0.03 would be good to make uniform float variable


	return color * att;

}

//Spot Light
struct SPOT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
	vec3 direction;
	float cutoff;
	float attenuation;
};
uniform SPOT spotLight1;

vec4 SpotLight(SPOT light)
{
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = (normalize((matrixView * vec4(light.position, 1)) - position)).xyz;
	float NdotL = dot(normalNew, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;

	vec3 V = normalize(-position.xyz);
	vec3 R = reflect(-L, normalNew);
	float RdotV = dot(R, V);
	if (NdotL > 0 && RdotV > 0)
		color += vec4(materialSpecular * light.specular * pow(RdotV, shininess), 1);

	vec3 D = normalize(mat3(matrixView) * light.direction);
	float s = dot(-L, D);
	float a = acos(s);

	float spotFactor;

	if (a <= light.cutoff) spotFactor = pow(s, light.attenuation);
	else spotFactor = 0;

	return spotFactor * color;

}

void main(void) 
{
	if (useNormalMap)
	{
		normalNew = 2.0 * texture(textureNormal, texCoord0).xyz - vec3(1.0, 1.0, 1.0);
		normalNew = normalize(matrixTangent * normalNew);
	}
	else
		normalNew = normal;

	outColor = color;
	outColor += PointLight(lightPoint1);
	outColor += PointLight(lightPoint2);
	outColor += SpotLight(spotLight1);
	outColor *= texture(texture0, texCoord0);
}
