// VERTEX SHADER
#version 330

// Matrices
uniform mat4 matrixProjection;
uniform mat4 matrixView;
uniform mat4 matrixModelView;

// Materials
uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float shininess;

in vec3 aVertex;
in vec3 aNormal;
in vec2 aTexCoord;
//in vec3 aTangent;
//in vec3 aBiTangent;

out vec4 color;
out vec4 position;
out vec3 normal;
out vec2 texCoord0;

// Light declarations

//Ambient
struct AMBIENT
{	
	vec3 color;
};
uniform AMBIENT lightAmbient1, lightAmbient2;

vec4 AmbientLight(AMBIENT light)
{
// Calculate Ambient Light
	return vec4(materialAmbient * light.color, 1);
}

//Directional
struct DIRECTIONAL
{	
	vec3 direction;
	vec3 diffuse;
};
uniform DIRECTIONAL lightDir;

vec4 DirectionalLight(DIRECTIONAL light)
{
	// Calculate Directional Light
	vec4 color = vec4(0, 0, 0, 0);
	vec3 L = normalize(mat3(matrixView) * light.direction);
	float NdotL = dot(normal, L);
	if (NdotL > 0)
		color += vec4(materialDiffuse * light.diffuse, 1) * NdotL;
	return color;
}

//Point
struct POINT
{
	vec3 position;
	vec3 diffuse;
	vec3 specular;
};
uniform POINT lightPoint1, lightPoint2;


void main(void) 
{
	// calculate position
	position = matrixModelView * vec4(aVertex, 1.0);
	gl_Position = matrixProjection * position;
	normal = normalize(mat3(matrixModelView) * aNormal);

	texCoord0 = aTexCoord;

	// calculate light
	color = vec4(0, 0, 0, 1);
	color += AmbientLight(lightAmbient1);
	color += AmbientLight(lightAmbient2);
	color += DirectionalLight(lightDir);
	//color += PointLight(lightPoint1);
	//color += PointLight(lightPoint2);
}




