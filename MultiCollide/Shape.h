#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "ShapeUtils.h"
#include "Arrow.h"
#include "Shader.h"


//An abstract class that defines common properties and behaviors among all
// objects to be used in collision simulation
class Shape {

public:

	//OpenGL:

	GLuint VBO, VAO;

	glm::mat4 model;

	
	//Orientation:

	//The initial position. Should only equal translation at time=0
	glm::vec3 centroid;  

	glm::vec3 translation;

	glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	float rotationAngle = 0.0f;
	//glm::mat4 rotation;

	glm::mat4 getRotationMatrix() {
		//Converts to quaternion, then to rotation matrix.
		//Reasoning is that user can enter arbitrary rot-axis,
		// which makes it hard to store as just quaternion
		//(e.g. if user types rot-axis x = 1, then rot-axis y=1,
		//  updating the quatenion doesn't give an axis of (1, 1, 0) )
		return glm::mat4_cast(getRotationQuat());
	}

	glm::quat getRotationQuat() {
		return glm::angleAxis(rotationAngle, glm::normalize(rotationAxis));
	}

	void applyRotation(glm::vec3 axis, float angle) {
		glm::mat4 newRot = glm::rotate(glm::mat4(), angle, axis);
		glm::mat4 current = this->getRotationMatrix();
		current = newRot * current;
		glm::quat curQuat = glm::quat_cast(current);
		this->rotationAngle = glm::angle(curQuat);
		this->rotationAxis = glm::axis(curQuat);
	}

	glm::vec3 angularVelocityAxis = glm::vec3(1.0f, 0.0f, 0.0f);   //TODO quaternions

	float angularVelocity = 0.0f;

	glm::vec3 curVelocity;

	glm::vec3 scaling = glm::vec3(1.0f, 1.0f, 1.0f);

	float time;

	float mass = 1.0f;

	float frictionCoefficient = .25f;    // mu

	float restitutionCoefficient = 1.0f; // e

	
	//Collision Detection:

	bool UsingBoundingSphere = true;

	float boundingBox[6];

	//Used to give padding to bounding sphere in so collision is not
	// detected to late
	const float BoundingSphereBuffer = .0075f;

	//Every subclass should set this by examining the furthest point from the 
	// shape's centroid during vertex creation/iteration
	//As of now includes the buffer ^
	float boundingSphereRadius;


	//Styling:

	glm::vec3 defaultColor = glm::vec3(1.0f, 0.5f, 0.31f);

	glm::vec3 contactColor = glm::vec3(1.0f, 0.0f, 1.0f);

	glm::vec3 objectColor = defaultColor;

	std::string name;


	//Abstract functionality:

	//Generate buffers and configure shader in-attributes (position, normal)
	virtual void InitVAOandVBO(Shader &shader) = 0;

	//Ensure VAO is bound before the draw call
	virtual void Draw(Shader &shader) = 0;

	//Destructor is responsible for deallocating the VAO and VBO
	virtual ~Shape() = 0 { }


	Arrow velocityArrow;

	virtual void DrawInitialVelocity(Shader &shader) {
		this->velocityArrow.updatePoints(centroid, centroid + curVelocity);
		this->velocityArrow.Draw(shader);
	}

	Arrow angularVelocityArrow;

	virtual void DrawInitialAngularVelocity(Shader &shader) {
		glm::vec3 axis = centroid + glm::normalize(angularVelocityAxis) * 1.3f * boundingSphereRadius;
		this->angularVelocityArrow.updatePoints(centroid, axis);
		this->angularVelocityArrow.Draw(shader);
	}

	Arrow rotationAxisArrow = Arrow(centroid, angularVelocityAxis);

	virtual void DrawRotationAxis(Shader &shader) {
		glm::quat q = glm::angleAxis(rotationAngle, rotationAxis);//glm::quat_cast(rotation);
		glm::vec3 axis = glm::vec3(q.x, q.y, q.z);
		
		if (!ShapeUtils::isZeroVec(axis)) {
			axis = glm::normalize(axis) * 1.5f * boundingSphereRadius;
			this->rotationAxisArrow.updatePoints(centroid, centroid + axis);
			this->rotationAxisArrow.Draw(shader);
		}
	}
};