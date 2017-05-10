#pragma once

#include <vector>
#include "Shape.h"

//Shapes that will utilize the Separating Axis Theorem for Collision Detection
class ShapeSeparatingAxis : public Shape {

public:

	//For Collision Detection
	std::vector<glm::vec3> corners;
	std::vector<glm::vec3> normals;

	virtual void ComputeInertia() override = 0;

	virtual void InitVAOandVBO(Shader &shader) override = 0;

	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) override {
		glm::mat4 rot = getRotationMatrix();
		glm::vec3 localPt = ShapeUtils::getLocalCoordinates(pt.pt, translation, rot, scaling);

		return localPt; //TODO very bad approximation. Only used for meshes right now tho
	}

	virtual bool DispatchCollisionDetection(Shape *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SAT DD 1" << std::endl;
		return other->DispatchCollisionDetection(this, closestPt2, closestPt1, pp2, pp1);
	}

	virtual bool DispatchCollisionDetection(Superquadric *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SAT DD 2" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

	virtual bool DispatchCollisionDetection(ShapeSeparatingAxis *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SAT DD 3" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

};