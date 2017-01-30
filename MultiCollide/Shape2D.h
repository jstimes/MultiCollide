#pragma once


#include "ShapeSeparatingAxis.h"

class Shape2D : public ShapeSeparatingAxis {

public: 

	//For OpenGL
	std::vector<GLfloat> vertices;

	Shape2D() {
		this->is2D = true;
		this->rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		this->angularVelocityAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	}


	virtual void InitVAOandVBO(Shader &shader) override {

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(shader.getAttribute("position"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	virtual void Draw(Shader &shader) override {

		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glUniform4f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z, 1.0f);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

	virtual ~Shape2D() override {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	//static bool CollisionDetection(Shape2D &s1, Shape2D &s2) {
	//	
	//	glm::mat4 s1Rot = s1.getRotationMatrix();
	//	glm::mat4 s2Rot = s2.getRotationMatrix();

	//	for (int i = 0; i < s1.normals.size(); i++) {
	//		
	//		glm::vec3 normal = ShapeUtils::applyRotation(s1.normals[i], s1Rot);

	//		float  min1, min2, max1, max2;
	//		min1 = min2 = 10000.0f;
	//		max1 = max2 = -min1;

	//		for (int c = 0; c < s1.corners.size(); c++) {
	//			glm::vec3 corner = ShapeUtils::getGlobalCoordinates(s1.corners[c], s1.translation, s1Rot);
	//			float proj = glm::dot(corner, normal);

	//			if (proj < min1) {
	//				min1 = proj;
	//			}
	//			if (proj > max1) {
	//				max1 = proj;
	//			}
	//		}

	//		for (int c = 0; c < s2.corners.size(); c++) {
	//			glm::vec3 corner = ShapeUtils::getGlobalCoordinates(s2.corners[c], s2.translation, s2Rot);
	//			float proj = glm::dot(corner, normal);

	//			if (proj < min2) {
	//				min2 = proj;
	//			}
	//			if (proj > max2) {
	//				max2 = proj;
	//			}
	//		}

	//		if (min1 > max2 || min2 > max2) {
	//			//std::cout << "Separating axis - no collision" << std::endl;
	//			return false;
	//		}
	//	}

	//	for (int i = 0; i < s2.normals.size(); i++) {

	//		glm::vec3 normal = ShapeUtils::applyRotation(s2.normals[i], s2Rot);

	//		float  min1, min2, max1, max2;
	//		min1 = min2 = 10000.0f;
	//		max1 = max2 = -min1;

	//		for (int c = 0; c < s1.corners.size(); c++) {
	//			glm::vec3 corner = ShapeUtils::getGlobalCoordinates(s1.corners[c], s1.translation, s1Rot);
	//			float proj = glm::dot(corner, normal);

	//			if (proj < min1) {
	//				min1 = proj;
	//			}
	//			if (proj > max1) {
	//				max1 = proj;
	//			}
	//		}

	//		for (int c = 0; c < s2.corners.size(); c++) {
	//			glm::vec3 corner = ShapeUtils::getGlobalCoordinates(s2.corners[c], s2.translation, s2Rot);
	//			float proj = glm::dot(corner, normal);

	//			if (proj < min2) {
	//				min2 = proj;
	//			}
	//			if (proj > max2) {
	//				max2 = proj;
	//			}
	//		}

	//		if (min1 > max2 || min2 > max2) {
	//			//std::cout << "Separating axis - no collision" << std::endl;
	//			return false;
	//		}
	//	}

	//	//std::cout << "Tried all axes, no separating axis, -> COllision" << std::endl;
	//	return true;
	//}
};