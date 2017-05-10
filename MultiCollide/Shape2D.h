#pragma once


#include "ShapeSeparatingAxis.h"

class Shape2D : public ShapeSeparatingAxis {

public: 

	//For OpenGL
	std::vector<GLfloat> vertices;

	Shape2D() {
		this->is2D = true;
		this->rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
		this->angularVelocityAxis = glm::vec3(0.0f, 0.0f, 0.0f);
	}

	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) override {
		//pt.pt is in global space

		glm::mat4 myRot = getRotationMatrix();
		glm::vec3 localPt = ShapeUtils::getLocalCoordinates(pt.pt, this->translation, myRot, this->scaling);

		//compare all corner vertices to localPt, track the closest 2 corners
		//Then find the normal of the edge between those two pts (should be adjacent)

		glm::vec3 closest1(0.0f);
		glm::vec3 closest2(0.0f);
		float dist1, dist2;
		dist1 = dist2 = 100000.0f;

		for (int i = 0; i < corners.size(); i++) {
			glm::vec3 corner = corners[i];
			float dist = ShapeUtils::squaredDistance(localPt, corner);

			if (dist < dist1) {
				//Move 1st point to second slot
				closest2 = closest1;
				dist2 = dist1;

				closest1 = corner;
				dist1 = dist;
			}
			else if (dist < dist2) {
				closest2 = corner;
				dist2 = dist;
			}
		}

		glm::vec3 line = closest1 - closest2;
		glm::vec3 normal = glm::vec3(-line.y, line.x, 0.0f);

		
		//Might be facing wrong direction, check that it's not pointing to centroid
		glm::vec3 zero(0.0f);
		glm::vec3 ptPlusNorm = localPt + normal;
		glm::vec3 ptSubNorm = localPt - normal;
		if (ShapeUtils::squaredDistance(ptPlusNorm, zero) < ShapeUtils::squaredDistance(ptSubNorm, zero)) {
			//If the pt plus the normal is closer to origin, normal is facing inward, so invert it:
			normal *= -1.0f;
		}

		return normal;
	}

	//From http://mathoverflow.net/questions/73556/calculating-moment-of-inertia-in-2d-planar-polygon
	virtual void ComputeInertia() override {
		float numerator = 0.0f;
		float denominator = 0.0f;

		for (int i = 0; i < corners.size() - 1; i++) {
			glm::vec3 pi = corners[i];
			glm::vec3 piPlus1 = corners[i + 1];

			float xi = pi.x;
			float yi = pi.y;
			float xj = piPlus1.x;
			float yj = piPlus1.y;

			float tail = (xi*yj - xj * yi);
			numerator += (xi*xi + yi*yi + xi*xj + yi*yj + xj*xj + yj + yj)*tail;
			denominator += tail;
		}

		this->angularInertia[0][0] = (this->mass / 6.0f) * (numerator / denominator);
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

};