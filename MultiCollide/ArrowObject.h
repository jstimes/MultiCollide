#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShapeUtils.h"
#include "ShapeShader.h"

//Represents an arrow starting at origin that goes to (1, 0, 0)
//Changes to shape & size are controlled by matrices passed into Draw method
//Used so only one arrow buffer is allocated, and so that one buffer can be transformed to 
// draw any arrow
class ArrowObject {

private:

	GLuint coneVAO, coneVBO;
	GLuint lineVAO, lineVBO;

	std::vector<GLfloat> coneVertices;
	std::vector<GLfloat> lineVertices;


	void InitConeVertices() {
		int numSegments = 36;

		float radius = .3f;
		float height = 1.0f;

		float offset = (MathUtils::twoPI) / numSegments;
		float baseHeight = -(height / 2.0f);

		glm::vec3 circleOrigin(0.0f, 0.0f, 0.0f);
		glm::vec3 tip(height, 0.0f, 0.0f);
		glm::vec3 circleNorm(-1.0f, 0.0f, 0.0f);

		for (int i = 0; i < numSegments; i++) {

			float rads = offset * i;
			glm::vec3 a(0.0f, MathUtils::sin(rads), MathUtils::cos(rads));

			rads += offset;
			glm::vec3 b(0.0f, MathUtils::sin(rads), MathUtils::cos(rads));

			glm::vec3 norm = ShapeUtils::getNormalOfTriangle(a, tip, b);

			a *= radius;
			b *= radius;

			ShapeUtils::addTriangleToVector(a, tip, b, norm, coneVertices);
			ShapeUtils::addTriangleToVector(circleOrigin, a, b, circleNorm, coneVertices);
		}
	}

	//In model space, line should just be from origin to (1, 0, 0)
	void InitLineVertices() {
		glm::vec3 norm(1.0f, 1.0f, 1.0f);

		lineVertices.push_back(0.0f);
		lineVertices.push_back(0.0f);
		lineVertices.push_back(0.0f);
		lineVertices.push_back(norm.x);
		lineVertices.push_back(norm.y);
		lineVertices.push_back(norm.z);

		lineVertices.push_back(1.0f);
		lineVertices.push_back(0.0f);
		lineVertices.push_back(0.0f);
		lineVertices.push_back(norm.x);
		lineVertices.push_back(norm.y);
		lineVertices.push_back(norm.z);
	}

public:

	ArrowObject() {
		InitLineVertices();
		InitConeVertices();
		InitBuffers();
	}

	void InitBuffers() {
		
		glGenVertexArrays(1, &this->coneVAO);
		glGenBuffers(1, &this->coneVBO);
		glBindVertexArray(this->coneVAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->coneVBO);
		glBufferData(GL_ARRAY_BUFFER, this->coneVertices.size() * sizeof(GLfloat), &this->coneVertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
		

		glGenVertexArrays(1, &this->lineVAO);
		glGenBuffers(1, &this->lineVBO);
		glBindVertexArray(this->lineVAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->lineVBO);
		glBufferData(GL_ARRAY_BUFFER, this->lineVertices.size() * sizeof(GLfloat), &this->lineVertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	void Draw(Shader &shader, glm::mat4 headModel, glm::mat4 lineModel) {
		glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(headModel));
		
		glBindVertexArray(this->coneVAO);
		glDrawArrays(GL_TRIANGLES, 0, coneVertices.size() / 6);
		glBindVertexArray(0);

		glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
		glBindVertexArray(this->lineVAO);
		glDrawArrays(GL_LINES, 0, lineVertices.size() / 6);
		glBindVertexArray(0);
	}

	~ArrowObject() {
		glDeleteBuffers(1, &this->coneVAO);
		glDeleteBuffers(1, &this->coneVBO);

		glDeleteBuffers(1, &this->lineVAO);
		glDeleteBuffers(1, &this->lineVBO);
	}
};