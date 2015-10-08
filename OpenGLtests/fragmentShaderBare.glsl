#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 color;

uniform vec3 lightPos;
uniform vec3 objectColor;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);

	// Ambient
    float ambientStrength = 0.3f;
    vec3 ambient = ambientStrength * lightColor;

	// Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(abs(dot(norm, lightDir)), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    color = vec4(result, 1.0f);

}