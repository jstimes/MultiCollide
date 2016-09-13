#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


#include "ArrowHead.h"


class Arrow {

public: 

	GLuint VAO, VBO;

	std::vector<GLfloat> vertices;

	glm::vec3 basePt;

	glm::vec3 tipPt;

	ArrowHead arrowHead;

	//Identity
	glm::mat4 model = glm::mat4();

	Arrow() {}

	Arrow(glm::vec3 base, glm::vec3 tip) {
		this->basePt = base;
		this->tipPt = tip;

		InitVertices();
		InitBuffers();

		arrowHead = ArrowHead(base, tip);
	}

	void updatePoints(glm::vec3 base, glm::vec3 tip) {
		this->basePt = base;
		this->tipPt = tip;

		this->vertices.clear();
		InitVertices();

		UpdateBuffer();

		arrowHead.UpdateModel(base, tip);
	}

	void Draw(Shader &shader) {

		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_LINES, 0, vertices.size() / 6);
		glBindVertexArray(0);

		this->arrowHead.Draw(shader);
	}


private:

	void InitVertices() {
		glm::vec3 norm(1.0f, 1.0f, 1.0f);

		vertices.push_back(basePt.x);
		vertices.push_back(basePt.y);
		vertices.push_back(basePt.z);
		vertices.push_back(norm.x);
		vertices.push_back(norm.y);
		vertices.push_back(norm.z);

		vertices.push_back(tipPt.x);
		vertices.push_back(tipPt.y);
		vertices.push_back(tipPt.z);
		vertices.push_back(norm.x);
		vertices.push_back(norm.y);
		vertices.push_back(norm.z);
	}

	void InitBuffers() {
		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(GLfloat), &this->vertices[0], GL_DYNAMIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	void UpdateBuffer() {
		glBindVertexArray(this->VAO);
		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, this->vertices.size() * sizeof(GLfloat), &this->vertices[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}


};
