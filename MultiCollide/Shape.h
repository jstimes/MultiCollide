#pragma once

#include <string>
#include <sstream>
#include <vector>

#include "ShapeUtils.h" // includes MathUtils, GL calls, and functions for shapes
#include "CollisionDetector.h"
#include "Arrow.h"

class CollisionDetector;
class Superquadric;
class ShapeSeparatingAxis;

struct ParamPoint {
	float u;
	float v;
	glm::vec3 pt;
};


//An abstract class that defines common properties and behaviors among all
// objects to be used in collision simulation
class Shape {

private:
	glm::mat3 angularInertia;

public:

	//OpenGL:

	GLuint VBO, VAO;

	glm::mat4 model;

	
	//Orientation:

	//The initial position. Should only equal translation at time=0
	glm::vec3 centroid;  

	//2D shapes just use the [0][0] entry
	
	virtual void ComputeInertia() = 0;

	glm::mat3 getAngularInertia() {
		return cbrtf(mass) * angularInertia;
	}

	void setAngularInertia(glm::mat3 &angular_inertia) {
		this->angularInertia = angular_inertia;
	}

	glm::vec3 translation;

	glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	float rotationAngle = 0.0f;

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

	//Magnitude gives speed:
	glm::vec3 angularVelocityAxis = glm::vec3(0.0f, 0.0f, 0.0f); 

	glm::vec3 curVelocity;

	//Used for x, y, and z components (uniform scaling)
	float scaling = 1.0f;

	float time;

	float mass = 1.0f;

	
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

	glm::vec3 contactColor2 = glm::vec3(1.0f, 1.0f, 0.0f);

	glm::vec3 objectColor = defaultColor;

	std::string name;

	void BackwardsIntegrate(float seconds = 2.5f) {
		float deltaT = .05f;

		int iterations = ((float)seconds) / deltaT;

		for (int i = 0; i < iterations; i++) {
			if (!MathUtils::isZeroVec(this->angularVelocityAxis)) {
				float speed = MathUtils::magnitude(this->angularVelocityAxis);
				glm::vec3 axis = glm::normalize(this->angularVelocityAxis);
				this->applyRotation(axis, -speed * deltaT);
			}
			this->translation += -this->curVelocity * deltaT;
		}

		this->time = -1.0f * seconds;
	}


	//Abstract functionality:

	//Should return normal in local space
	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) = 0;

	//Generate buffers and configure shader in-attributes (position, normal)
	virtual void InitVAOandVBO(Shader &shader) = 0;
	
	//Ensure VAO is bound before the draw call
	virtual void Draw(Shader &shader) = 0;

	//Destructor is responsible for deallocating the VAO and VBO
	virtual ~Shape() { }



	virtual void DrawInitialVelocity(Shader &shader) {
		Arrow::Draw(shader, glm::length(this->curVelocity), 2.0f, this->curVelocity, this->centroid);
	}

	virtual void DrawInitialAngularVelocity(Shader &shader) {
		Arrow::Draw(shader, glm::length(this->angularVelocityAxis), 2.0f, this->angularVelocityAxis, translation);
	}

	virtual void DrawRotationAxis(Shader &shader) {
		//glm::quat q = glm::angleAxis(rotationAngle, rotationAxis);
		//glm::vec3 axis = glm::vec3(q.x, q.y, q.z);
		
		//if (!ShapeUtils::isZeroVec(axis)) {
			Arrow::Draw(shader, 1.5f * this->boundingSphereRadius, 1.0f, this->rotationAxis, this->translation);
		//}
	}

	//These methods are required so that the correct collision detection method
	// can be called without explicitly checking the types of objects and casting them
	virtual bool DispatchCollisionDetection(Shape *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		//std::cout << "DD 1" << std::endl;
		return other->DispatchCollisionDetection(this, closestPt2, closestPt1, pp2, pp1);
	}

	virtual bool DispatchCollisionDetection(Superquadric *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		//std::cout << "DD 2" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

	virtual bool DispatchCollisionDetection(ShapeSeparatingAxis *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		//std::cout << "DD 3" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

	/*
	0 - sphere
	1 - cube 
	2 - ellipsoid
	3 - superquadric
	4 - custom superquad, a1, a2, a3, e1, e2
	5 - mesh - all data?
	6 - circle  
	7 - triangle
	8 - square
	9 - custom poly - all vertices, x1, y1, x2, y2,...
	10 - icosahedron
	11 - tetrahedron
	*/
	virtual std::string getShapeCSVline1() = 0;

	virtual std::string getShapeCSVline2() {
		std::ostringstream os;
		ShapeUtils::AddVecToStringstream(this->translation, os, ",");

		ShapeUtils::AddVecToStringstream(this->curVelocity, os, ",");

		ShapeUtils::AddVecToStringstream(this->rotationAxis, os, ",");

		os << rotationAngle << ",";

		ShapeUtils::AddVecToStringstream(this->angularVelocityAxis, os, ",");

		os << scaling << ",";

		os << mass << "\n";

		return os.str();
	}

	/*virtual Shape* duplicate() {

	}*/
};