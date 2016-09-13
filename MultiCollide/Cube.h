#pragma once

#include "Shape.h"

class Cube : public Shape {

public:

	virtual void InitVAOandVBO(Shader &shader) override {

		GLfloat vertices[] = {
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
			0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,
			0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,

			0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,
			-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,


			-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

			0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,


			-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  -1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  -1.0f, 0.0f, 0.0f,


			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,

			0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,


			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f, 0.0f,
			0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,
			0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,

			0.5f, -0.5f,  0.5f,  0.0f, -1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,


			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,

			0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(shader.getAttribute("position"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		boundingSphereRadius = sqrtf(.5f*.5f + .5f*.5f + .5f*.5f) + Shape::BoundingSphereBuffer;
	}

	//Ensure VAO is bound before the draw call
	virtual void Draw(Shader &shader) override {
		shader.Use();
		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	//Destructor is responsible for deallocating the VAO and VBO
	virtual ~Cube() { 
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

};