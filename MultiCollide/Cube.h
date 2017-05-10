#pragma once

#include "ShapeSeparatingAxis.h"

class Cube : public ShapeSeparatingAxis {

public:

	//From https://en.wikipedia.org/wiki/List_of_moments_of_inertia
	virtual void ComputeInertia() override {
		//h = w = d

		angularInertia = glm::mat3();

		float val = (1.0f / 12.0f) * mass * 2.0f;
		angularInertia[0][0] = val;
		angularInertia[1][1] = val;
		angularInertia[2][2] = val;
	}

	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) override {
		glm::mat4 rot = getRotationMatrix();
		glm::vec3 localPt = ShapeUtils::getLocalCoordinates(pt.pt, translation, rot, scaling);

		glm::mat4 i4;
		glm::mat4 axisRot = glm::rotate(i4, MathUtils::PI_OVER_4, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec4 local4 = glm::vec4(localPt.x, localPt.y, localPt.z, 1.0f);
		glm::vec3 localRotatedPt = glm::vec3(axisRot * local4);

		//First check if on top or bottom of cube
		// If so, y component will be greater than z and x
		if (MathUtils::abs(localRotatedPt.y) > localRotatedPt.x
			&& MathUtils::abs(localRotatedPt.y) > localRotatedPt.z) {
			//localPt is on top or bottom of cube
			if (localRotatedPt.y > 0.0f) {
				return glm::vec3(0.0f, 1.0f, 0.0f);
			}
			else {
				return glm::vec3(0.0f, -1.0f, 0.0f);
			}
		}

		//Now localPt must lie on one of the sides of the cube.
		// The rotation makes it so the edges of the sides 
		// lie on the x and z axes, for easy checks:
		if (localRotatedPt.x > 0.0f) {
			if (localRotatedPt.z > 0.0f) {
				return glm::vec3(0.0f, 0.0f, 1.0f);
			}
			else {
				return glm::vec3(1.0f, 0.0f, 0.0f);
			}
		}
		else {
			if (localRotatedPt.z > 0.0f) {
				return glm::vec3(-1.0f, 0.0f, 0.0f);
			}
			else {
				return glm::vec3(0.0f, 0.0f, -1.0f);
			}
		}
	}

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

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "1" << std::endl;
		return os.str();
	}

};