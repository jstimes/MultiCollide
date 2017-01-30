#pragma once

#include "ShapeSeparatingAxis.h"

class Cube : public ShapeSeparatingAxis {

public:

	virtual void InitVAOandVBO(Shader &shader) override {

		glm::vec3 c1(.5f, -.5f, .5f);   //bottom right fwd
		glm::vec3 c2(.5f, -.5f, -.5f);  //bottom right back
		glm::vec3 c3(-.5f, -.5f, -.5f); //bottom left back
		glm::vec3 c4(-.5f, -.5f, .5f);  //bototm left fwd

		glm::vec3 c5(.5f, .5f, .5f);   //top right fwd
		glm::vec3 c6(.5f, .5f, -.5f);  //top right back
		glm::vec3 c7(-.5f, .5f, -.5f); //top left back
		glm::vec3 c8(-.5f, .5f, .5f);  //top left fwd

		corners.push_back(c1);
		corners.push_back(c2);
		corners.push_back(c3);
		corners.push_back(c4);
		corners.push_back(c5);
		corners.push_back(c6);
		corners.push_back(c7);
		corners.push_back(c8);

		glm::vec3 n1(1.0f, 0.0f, 0.0f);
		glm::vec3 n2(0.0f, 1.0f, 0.0f);
		glm::vec3 n3(0.0f, 0.0f, 1.0f); 

		normals.push_back(n1);
		normals.push_back(n2);
		normals.push_back(n3);

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