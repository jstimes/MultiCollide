#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Arrow.h"
#include "Shader.h"

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

//Represents a right-handed set of axes that can be oriented to render as an object's principal frame
class Frame {

	Arrow xAxis;
	Arrow yAxis;
	Arrow zAxis;

	glm::vec3 xColor = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 yColor = glm::vec3(0.2f, .5f, 0.2f);
	glm::vec3 zColor = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);

public:

	Frame() { }

	void init() {
		xAxis = Arrow(origin, xColor);
		xAxis.SetArrowHeadScaling(.05f);
		yAxis = Arrow(origin, glm::vec3(0.0, 1.0f, 0.0f));
		yAxis.SetArrowHeadScaling(.05f);
		zAxis = Arrow(origin, zColor);
		zAxis.SetArrowHeadScaling(.05f);
	};

	//Ensure VAO is bound before the draw call
	void Draw(Shader &shader, glm::vec3 &translation, glm::mat4 &rotation, glm::mat4 &scaling) {
	
		glm::mat4 model = glm::translate(glm::mat4(), translation) * rotation * scaling;
		xAxis.ApplyModel(model);
		yAxis.ApplyModel(model);
		zAxis.ApplyModel(model);
		
		shader.Use();
		GLuint objectColorLoc = shader.getUniform("objectColor");
		//glUniform3fv(objectColorLoc, 3, &xColor[0]);
		glUniform3f(objectColorLoc, xColor[0], xColor[1], xColor[2]);
		xAxis.Draw(shader);

		//glUniform3fv(objectColorLoc, 3, &yColor[0]);
		glUniform3f(objectColorLoc, yColor[0], yColor[1], yColor[2]);
		yAxis.Draw(shader);

		//glUniform3fv(objectColorLoc, 3, &zColor[0]);
		glUniform3f(objectColorLoc, zColor[0], zColor[1], zColor[2]);
		zAxis.Draw(shader);
	};

};