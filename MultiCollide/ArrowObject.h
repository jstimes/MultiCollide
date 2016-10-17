#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ShapeUtils.h"

//Represents an arrow starting at origin that goes to (1, 0, 0)
//Changes to shape & size are controlled by matrices passed into Draw method
//Used so only one arrow buffer is allocated, and so that one buffer can be transformed to 
// draw any arrow
class ArrowObject {

private:

	GLuint headVAO, headVBO;
	GLuint lineVAO, lineVBO;

	std::vector<GLfloat> headVertices;
	std::vector<GLfloat> lineVertices;

	void InitHeadVertices() {
		glm::vec3 base1, base2, base3, tip;

		glm::mat4 rotation;
		glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		float angle = acos(glm::dot(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))));
		rotation = glm::rotate(glm::mat4(), angle, axis);

		//Orient tetrahedron so that 'tip' has just an x component:
		base1 = glm::vec3(rotation * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
		base2 = glm::vec3(rotation * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f));
		base3 = glm::vec3(rotation * glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
		tip = glm::vec3(rotation * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		glm::vec3 base123norm = glm::normalize(glm::cross(base3 - base1, base2 - base1));
		glm::vec3 base12tipnorm = glm::normalize(glm::cross(base2 - base1, tip - base1));
		glm::vec3 base13tipnorm = glm::normalize(glm::cross(base1 - base3, tip - base3));
		glm::vec3 base23tipnorm = glm::normalize(glm::cross(base3 - base2, tip - base2));


		headVertices.push_back(base1.x);
		headVertices.push_back(base1.y);
		headVertices.push_back(base1.z);
		headVertices.push_back(base123norm.x);
		headVertices.push_back(base123norm.y);
		headVertices.push_back(base123norm.z);

		headVertices.push_back(base2.x);
		headVertices.push_back(base2.y);
		headVertices.push_back(base2.z);
		headVertices.push_back(base123norm.x);
		headVertices.push_back(base123norm.y);
		headVertices.push_back(base123norm.z);

		headVertices.push_back(base3.x);
		headVertices.push_back(base3.y);
		headVertices.push_back(base3.z);
		headVertices.push_back(base123norm.x);
		headVertices.push_back(base123norm.y);
		headVertices.push_back(base123norm.z);



		headVertices.push_back(base1.x);
		headVertices.push_back(base1.y);
		headVertices.push_back(base1.z);
		headVertices.push_back(base12tipnorm.x);
		headVertices.push_back(base12tipnorm.y);
		headVertices.push_back(base12tipnorm.z);

		headVertices.push_back(base2.x);
		headVertices.push_back(base2.y);
		headVertices.push_back(base2.z);
		headVertices.push_back(base12tipnorm.x);
		headVertices.push_back(base12tipnorm.y);
		headVertices.push_back(base12tipnorm.z);

		headVertices.push_back(tip.x);
		headVertices.push_back(tip.y);
		headVertices.push_back(tip.z);
		headVertices.push_back(base12tipnorm.x);
		headVertices.push_back(base12tipnorm.y);
		headVertices.push_back(base12tipnorm.z);



		headVertices.push_back(base1.x);
		headVertices.push_back(base1.y);
		headVertices.push_back(base1.z);
		headVertices.push_back(base13tipnorm.x);
		headVertices.push_back(base13tipnorm.y);
		headVertices.push_back(base13tipnorm.z);

		headVertices.push_back(base3.x);
		headVertices.push_back(base3.y);
		headVertices.push_back(base3.z);
		headVertices.push_back(base13tipnorm.x);
		headVertices.push_back(base13tipnorm.y);
		headVertices.push_back(base13tipnorm.z);

		headVertices.push_back(tip.x);
		headVertices.push_back(tip.y);
		headVertices.push_back(tip.z);
		headVertices.push_back(base13tipnorm.x);
		headVertices.push_back(base13tipnorm.y);
		headVertices.push_back(base13tipnorm.z);



		headVertices.push_back(base2.x);
		headVertices.push_back(base2.y);
		headVertices.push_back(base2.z);
		headVertices.push_back(base23tipnorm.x);
		headVertices.push_back(base23tipnorm.y);
		headVertices.push_back(base23tipnorm.z);

		headVertices.push_back(base3.x);
		headVertices.push_back(base3.y);
		headVertices.push_back(base3.z);
		headVertices.push_back(base23tipnorm.x);
		headVertices.push_back(base23tipnorm.y);
		headVertices.push_back(base23tipnorm.z);

		headVertices.push_back(tip.x);
		headVertices.push_back(tip.y);
		headVertices.push_back(tip.z);
		headVertices.push_back(base23tipnorm.x);
		headVertices.push_back(base23tipnorm.y);
		headVertices.push_back(base23tipnorm.z);
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
		InitHeadVertices();
		InitBuffers();
	}

	void InitBuffers() {

		glGenVertexArrays(1, &this->headVAO);
		glGenBuffers(1, &this->headVBO);
		glBindVertexArray(this->headVAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->headVBO);
		glBufferData(GL_ARRAY_BUFFER, this->headVertices.size() * sizeof(GLfloat), &this->headVertices[0], GL_STATIC_DRAW);

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
		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(headModel));
		glBindVertexArray(this->headVAO);
		glDrawArrays(GL_TRIANGLES, 0, headVertices.size() / 6);
		glBindVertexArray(0);

		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(lineModel));
		glBindVertexArray(this->lineVAO);
		glDrawArrays(GL_LINES, 0, lineVertices.size() / 6);
		glBindVertexArray(0);
	}

	~ArrowObject() {
		std::cout << "ArrowObject deleted" << std::endl;
		glDeleteBuffers(1, &this->headVAO);
		glDeleteBuffers(1, &this->headVBO);

		glDeleteBuffers(1, &this->lineVAO);
		glDeleteBuffers(1, &this->lineVBO);
	}
};