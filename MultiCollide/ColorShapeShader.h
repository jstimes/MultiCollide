#pragma once

#include "ShapeUtils.h"

//Wrapper object for main shader
class ColorShapeShader {

public:

	static ColorShapeShader getInstance();

	Shader shader;

	//Shader uniform locations:
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint lightPosLoc;
	GLint objectColorLoc;
	GLint sceneTransformLoc;


	ColorShapeShader() {
		//shader = Shader::ShaderFromCode("#version 100 \n attribute vec3 position;\n attribute vec3 normal;\n uniform mat4 model;\n uniform mat4 view;\n uniform mat4 projection;\n uniform mat4 sceneTransform;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n void main()\n {\n Normal = mat3(model) * normal;\n FragPos = vec3(model * vec4(position, 1.0));\n	if(position.y == 0.0){\n		Line = vec3(1.0, 1.0, 1.0);\n	}\n	else {\n		Line = vec3(-1.0, -1.0, -1.0);\n	}\n   gl_Position = sceneTransform * projection * view * model * vec4(position, 1.0);\n }\n",
		//	"#version 100\n precision mediump float;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n  uniform vec3 lightPos;\n uniform vec3 objectColor;\n void main()\n {\n vec3 lightColor = vec3(1.0, 1.0, 1.0);\n float ambientStrength = 0.5;\n  vec3 ambient = ambientStrength * lightColor;\n     vec3 norm = normalize(Normal);\n    vec3 lightDir = normalize(lightPos - FragPos);\n    float diff = max(dot(norm, lightDir), 0.0);\n    vec3 diffuse = diff * lightColor;\n    vec3 result = (ambient + diffuse) * objectColor;\n    gl_FragColor = vec4(result, 1.0);\n if(objectColor.x + objectColor.y + objectColor.z < .00001) {\n 		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n 	}\n }\n");

		shader = Shader::ShaderFromCode("#version 100 \n attribute vec3 position;\n attribute vec3 normal;\n attribute vec4 color;\n uniform mat4 model;\n uniform mat4 sceneTransform;\n uniform mat4 view;\n uniform mat4 projection;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec4 Color;\n void main()\n {\n //gl_PointSize = 10.0;\n FragPos = vec3(model * vec4(position, 1.0));\n Normal = mat3(model) * normal;\n Color = color;\n gl_Position = sceneTransform * projection * view * model * vec4(position, 1.0);\n }\n",
			"#version 100\n precision mediump float;\n varying vec3 FragPos;\n varying vec3 Normal;\n varying vec4 Color;\n uniform vec3 lightPos;\n void main()\n {\n  vec4 lightColor = vec4(1.0, 1.0, 1.0, 1.0);\n float ambientStrength = 0.5;\n vec4 ambient = ambientStrength * lightColor;\n vec3 norm = normalize(Normal);\n vec3 lightDir = normalize(lightPos - FragPos);\n float diff = max(dot(norm, lightDir), 0.0);\n vec4 diffuse = diff * lightColor;\n vec4 result = (ambient + diffuse) * Color;\n gl_FragColor = vec4(result.x, result.y, result.z, Color.w);\n  }\n");

		//shader = Shader("vertexShader.glsl", "fragmentShader.glsl");

		modelLoc = glGetUniformLocation(shader.Program, "model");
		viewLoc = glGetUniformLocation(shader.Program, "view");
		projLoc = glGetUniformLocation(shader.Program, "projection");
		lightPosLoc = glGetUniformLocation(shader.Program, "lightPos");
		//objectColorLoc = glGetUniformLocation(shader.Program, "objectColor");
		sceneTransformLoc = glGetUniformLocation(shader.Program, "sceneTransform");
	}

};