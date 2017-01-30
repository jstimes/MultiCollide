#pragma once

#include "ShapeUtils.h"
#include "ShapeShader.h"
#include "Impact.h"

class ImpulseRenderer {

public:

	static ImpulseRenderer getInstance();

	//Shader impulseShader;

	//GLint icolorLoc;
	//GLint imodelLoc;
	//GLint iviewLoc;
	//GLint iprojLoc;
	//GLint isceneTransformLoc;

	std::vector<GLfloat> ptVertices;
	GLuint VAOpt, VBOpt;

	std::vector<GLfloat> planeVertices;
	GLuint planeVAO, planeVBO;


	ImpulseRenderer() {
		/*impulseShader = Shader::ShaderFromCode("#version 100 \n attribute vec3 position;\n uniform mat4 model;\n uniform mat4 sceneTransform;\n uniform mat4 view;\n uniform mat4 projection;\n varying vec3 FragPos;\n void main()\n {\n FragPos = vec3(model * vec4(position, 1.0));\n  gl_Position = sceneTransform * projection * view * model * vec4(position, 1.0);\n }\n",
		"#version 100\n precision mediump float;\n varying vec3 FragPos;\n uniform vec3 color;\n void main()\n {\n  gl_FragColor = vec4(color, 1.0);\n }\n");

		impulseShader.Use();
		icolorLoc = glGetUniformLocation(impulseShader.Program, "color");
		imodelLoc = glGetUniformLocation(impulseShader.Program, "model");
		iviewLoc = glGetUniformLocation(impulseShader.Program, "view");
		iprojLoc = glGetUniformLocation(impulseShader.Program, "projection");
		isceneTransformLoc = glGetUniformLocation(impulseShader.Program, "sceneTransform");*/

		//Single pt:
		ptVertices.push_back(0.0f);
		ptVertices.push_back(0.0f);
		ptVertices.push_back(0.0f);

		glGenVertexArrays(1, &VAOpt);
		glGenBuffers(1, &VBOpt);
		glBindVertexArray(VAOpt);
		glBindBuffer(GL_ARRAY_BUFFER, VBOpt);
		glBufferData(GL_ARRAY_BUFFER, ptVertices.size() * sizeof(GLfloat), &ptVertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);


		ShapeShader::getInstance().shader.Use();

		float value = 1.50f;
		float yValue = .0;

		glm::vec3 norm(0.0f, 1.0f, 0.0f);

		glm::vec3 a(value, 0.0f, value);
		glm::vec3 b(-0.0f, 0.0f, -0.0f);
		glm::vec3 c(-0.0f, 0.0f, value);
		glm::vec3 d(value, 0.0f, -0.0f);
		ShapeUtils::AddPoint(planeVertices, a, norm);
		ShapeUtils::AddPoint(planeVertices, b, norm);
		ShapeUtils::AddPoint(planeVertices, c, norm);

		ShapeUtils::AddPoint(planeVertices, b, norm);
		ShapeUtils::AddPoint(planeVertices, a, norm);
		ShapeUtils::AddPoint(planeVertices, d, norm);

		glGenVertexArrays(1, &planeVAO);
		glGenBuffers(1, &planeVBO);
		glBindVertexArray(planeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
		glBufferData(GL_ARRAY_BUFFER, planeVertices.size() * sizeof(GLfloat), &planeVertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		//Normal
		glVertexAttribPointer(ShapeShader::getInstance().shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

	void cleanup() {
		glDeleteBuffers(1, &planeVAO);
		glDeleteBuffers(1, &planeVBO);

		glDeleteBuffers(1, &VAOpt);
		glDeleteBuffers(1, &VBOpt);
	}

};
