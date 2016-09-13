#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>


class ArrowHead {

private:
	glm::mat4 model;

	GLuint VAO, VBO;

	std::vector<GLfloat> vertices;

	void InitVertices() {
		glm::vec3 base1, base2, base3, tip;

		glm::mat4 rotation;
		glm::vec3 axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
		rotation = glm::rotate(glm::mat4(), acos(glm::dot(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)), glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)))), axis);

		//Orient tetrahedron so that 'tip' has just an x component:
		base1 = glm::vec3(rotation * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f));
		base2 = glm::vec3(rotation * glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f));
		base3 = glm::vec3(rotation * glm::vec4(1.0f, -1.0f, -1.0f, 1.0f));
		tip = glm::vec3(rotation * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

		glm::vec3 base123norm = glm::normalize(glm::cross(base3 - base1, base2 - base1));
		glm::vec3 base12tipnorm = glm::normalize(glm::cross(tip - base1, base2 - base1));
		glm::vec3 base13tipnorm = glm::normalize(glm::cross(base3 - base1, tip - base1));
		glm::vec3 base23tipnorm = glm::normalize(glm::cross(base3 - base1, base2 - base1));


		vertices.push_back(base1.x);
		vertices.push_back(base1.y);
		vertices.push_back(base1.z);
		vertices.push_back(base123norm.x);
		vertices.push_back(base123norm.y);
		vertices.push_back(base123norm.z);

		vertices.push_back(base2.x);
		vertices.push_back(base2.y);
		vertices.push_back(base2.z);
		vertices.push_back(base123norm.x);
		vertices.push_back(base123norm.y);
		vertices.push_back(base123norm.z);

		vertices.push_back(base3.x);
		vertices.push_back(base3.y);
		vertices.push_back(base3.z);
		vertices.push_back(base123norm.x);
		vertices.push_back(base123norm.y);
		vertices.push_back(base123norm.z);



		vertices.push_back(base1.x);
		vertices.push_back(base1.y);
		vertices.push_back(base1.z);
		vertices.push_back(base12tipnorm.x);
		vertices.push_back(base12tipnorm.y);
		vertices.push_back(base12tipnorm.z);

		vertices.push_back(base2.x);
		vertices.push_back(base2.y);
		vertices.push_back(base2.z);
		vertices.push_back(base12tipnorm.x);
		vertices.push_back(base12tipnorm.y);
		vertices.push_back(base12tipnorm.z);

		vertices.push_back(tip.x);
		vertices.push_back(tip.y);
		vertices.push_back(tip.z);
		vertices.push_back(base12tipnorm.x);
		vertices.push_back(base12tipnorm.y);
		vertices.push_back(base12tipnorm.z);



		vertices.push_back(base1.x);
		vertices.push_back(base1.y);
		vertices.push_back(base1.z);
		vertices.push_back(base13tipnorm.x);
		vertices.push_back(base13tipnorm.y);
		vertices.push_back(base13tipnorm.z);

		vertices.push_back(base3.x);
		vertices.push_back(base3.y);
		vertices.push_back(base3.z);
		vertices.push_back(base13tipnorm.x);
		vertices.push_back(base13tipnorm.y);
		vertices.push_back(base13tipnorm.z);

		vertices.push_back(tip.x);
		vertices.push_back(tip.y);
		vertices.push_back(tip.z);
		vertices.push_back(base13tipnorm.x);
		vertices.push_back(base13tipnorm.y);
		vertices.push_back(base13tipnorm.z);



		vertices.push_back(base2.x);
		vertices.push_back(base2.y);
		vertices.push_back(base2.z);
		vertices.push_back(base23tipnorm.x);
		vertices.push_back(base23tipnorm.y);
		vertices.push_back(base23tipnorm.z);

		vertices.push_back(base3.x);
		vertices.push_back(base3.y);
		vertices.push_back(base3.z);
		vertices.push_back(base23tipnorm.x);
		vertices.push_back(base23tipnorm.y);
		vertices.push_back(base23tipnorm.z);

		vertices.push_back(tip.x);
		vertices.push_back(tip.y);
		vertices.push_back(tip.z);
		vertices.push_back(base23tipnorm.x);
		vertices.push_back(base23tipnorm.y);
		vertices.push_back(base23tipnorm.z);
	}

public:

	ArrowHead() {}

	ArrowHead(glm::vec3 base, glm::vec3 tip) {

		InitVertices();

		InitBuffers();

		UpdateModel(base, tip);
	}

	void UpdateModel(glm::vec3 base, glm::vec3 tip) {
		glm::vec3 direction = tip - base;

		if (abs(direction.x) < .0001f && abs(direction.y) < .0001f && abs(direction.z) < .0001f) {
			model = glm::mat4(0.0f);
			return;
		}

		//direction = glm::normalize(direction);

		//glm::vec3 rotAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), direction);
		//float rotAngle = acos(glm::dot(direction, glm::vec3(1.0f, 0.0f, 0.0f)));

		////In case direction is parallel with (1, 0, 0) :
		//if (abs(rotAxis.x) < .0001f && abs(rotAxis.y) < .0001f && abs(rotAxis.z) < .0001f) {
		//	rotAxis = glm::vec3(.0f, 1.0f, 0.0f);
		//	
		//	if (direction.x > 0.0f) {
		//		rotAngle = .0f;
		//	}
		//	else {
		//		rotAngle = glm::pi<float>();
		//	}
		//}

		//glm::mat4 rotation = glm::rotate(glm::mat4(), rotAngle, rotAxis);

		glm::mat4 translation = glm::translate(glm::mat4(), tip);

		glm::mat4 rotation = ShapeUtils::rotationFromAtoB(glm::vec3(1.0f, 0.0f, 0.0f), direction);

		glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(.1f, .1f, .1f));

		this->model = translation * rotation * scale;
	}

	void InitBuffers() {

		glGenVertexArrays(1, &this->VAO);
		glGenBuffers(1, &this->VBO);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(GLfloat), &this->vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	void Draw(Shader &shader) {
		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);
	}

	~ArrowHead() {
		glDeleteBuffers(1, &this->VAO);
		glDeleteBuffers(1, &this->VBO);
	}
};