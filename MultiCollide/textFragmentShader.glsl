#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

uniform vec3 backgroundColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
	if(sampled.a < 0.1)
		if(backgroundColor.x > 0)
			color = vec4(backgroundColor, 1.0);
        else
			discard;
    else
		color = vec4(textColor, 1.0) * sampled;
} 