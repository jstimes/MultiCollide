#pragma once

#include <string>
#include <vector>

#include "ShapeUtils.h" // includes MathUtils, GL calls, and functions for shapes
#include "CollisionDetector.h"
#include "Arrow.h"

class CollisionDetector;
class Superquadric;
class ShapeSeparatingAxis;
struct ParamPoint;


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

	glm::mat3 angularInertia;

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

		/*std::cout << "Rotation Update:" << std::endl;
		MathUtils::printMat(newRot);*/

		glm::mat4 current = this->getRotationMatrix();
		current = newRot * current;
		glm::quat curQuat = glm::quat_cast(current);
		this->rotationAngle = glm::angle(curQuat);
		this->rotationAxis = glm::axis(curQuat);
	}

	glm::vec3 angularVelocityAxis = glm::vec3(1.0f, 0.0f, 0.0f);   //TODO quaternions

	float angularVelocity = 0.0f;

	glm::vec3 curVelocity;

	//glm::vec3 scaling = glm::vec3(1.0f, 1.0f, 1.0f);
	float scaling = 1.0f;

	float time;

	float mass = 1.0f;

	float frictionCoefficient = .25f;    // mu

	float restitutionCoefficient = 1.0f; // e

	
	//Collision Detection:

	bool is2D = false;

	bool UsingBoundingSphere = true;

	float boundingBox[6];

	//Used to give padding to bounding sphere so collision is not
	// detected to0 late
	const float BoundingSphereBuffer = .0075f;

	//Every subclass should set this by examining the furthest point from the 
	// shape's centroid during vertex creation/iteration
	//As of now includes the buffer ^
	float boundingSphereRadius;


	//Styling:
	bool useCustomColors = false;

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



	virtual void DrawInitialVelocity(Shader &shader) {
		Arrow::Draw(shader, glm::length(curVelocity), 2.0f, curVelocity, centroid);
	}

	virtual void DrawInitialAngularVelocity(Shader &shader) {
		Arrow::Draw(shader, 1.3f * boundingSphereRadius, 2.0f, angularVelocityAxis, translation);
	}

	virtual void DrawRotationAxis(Shader &shader) {
		glm::quat q = glm::angleAxis(rotationAngle, rotationAxis);
		glm::vec3 axis = glm::vec3(q.x, q.y, q.z);
		
		if (!ShapeUtils::isZeroVec(axis)) {
			Arrow::Draw(shader, 1.5f * boundingSphereRadius, 1.0f, axis, translation);
		}
	}

	virtual bool DispatchCollisionDetection(Shape *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "DD 1" << std::endl;
		return other->DispatchCollisionDetection(this, closestPt1, closestPt2, pp1, pp2);
	}

	virtual bool DispatchCollisionDetection(Superquadric *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "DD 2" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
		//other.DispatchCollisionDetection(*this);
	}

	virtual bool DispatchCollisionDetection(ShapeSeparatingAxis *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "DD 3" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
		//other.DispatchCollisionDetection(*this);
	}
};