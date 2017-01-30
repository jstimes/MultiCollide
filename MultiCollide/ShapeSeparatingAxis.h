#pragma once

#include <vector>
#include "Shape.h"

//Shapes that will utilize the Separating Axis Theorem for Collision Detection
class ShapeSeparatingAxis : public Shape {

public:

	//For Collision Detection
	std::vector<glm::vec3> corners;
	std::vector<glm::vec3> normals;


	virtual bool DispatchCollisionDetection(Shape *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "SQ DD 1" << std::endl;
		return other->DispatchCollisionDetection(this, closestPt1, closestPt2, pp1, pp2);
	}

	virtual bool DispatchCollisionDetection(Superquadric *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "SQ DD 2" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
		//other.DispatchCollisionDetection(*this);
	}

	virtual bool DispatchCollisionDetection(ShapeSeparatingAxis *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
		std::cout << "SQ DD 3" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
		//other.DispatchCollisionDetection(*this);
	}

};