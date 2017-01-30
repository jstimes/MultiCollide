#include "CollisionDetector.h"

#include "Shape.h"
#include "ShapeUtils.h"
#include "Superquadric.h"
#include "ShapeSeparatingAxis.h"

bool CollisionDetector::Detect(Shape &s1, Shape &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-S CD" << std::endl;

	return false;
}

bool CollisionDetector::Detect(Shape &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-SQ CD" << std::endl;

	return false;
}

bool CollisionDetector::Detect(Shape &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-SAT CD" << std::endl;

	return false;
}

bool CollisionDetector::Detect(Superquadric &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "SQ-SAT CD" << std::endl;

	return false;
}

bool CollisionDetector::Detect(ShapeSeparatingAxis &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "SAT-SQ CD" << std::endl;

	return false;
}

bool CollisionDetector::Detect(Superquadric &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "SQ-SQ CD" << std::endl;

	float contactAccuracy = 0.001f;
	Superquadric::ClosestPointFramework(s1, s2, closestPt1, closestPt2, pp1, pp2);

	//TODO  use squared distance instead
	float distance = glm::distance(closestPt1, closestPt2);

	//check for collision:
	if (distance < contactAccuracy || glm::dot(s1.translation - s2.translation, closestPt1 - closestPt2) < 0.0) {
		return true;
	}

	return false;
}

//Returns true if collision detected
bool CollisionDetector::Detect(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {

	std::cout << "SAT-SAT CD" << std::endl;

	glm::mat4 s1Rot, s2Rot;
	s1Rot = s1.getRotationMatrix();
	s2Rot = s2.getRotationMatrix();

	std::vector<glm::vec3> allNormals;
	for (int i1 = 0; i1 < s1.normals.size(); i1++) {
		glm::vec3 normal = ShapeUtils::applyRotation(s1.normals[i1], s1Rot);
		allNormals.push_back(normal);
	}
	for (int i2 = 0; i2 < s2.normals.size(); i2++) {
		glm::vec3 normal = ShapeUtils::applyRotation(s2.normals[i2], s2Rot);
		allNormals.push_back(normal);
	}

	for (int n = 0; n < allNormals.size(); n++) {
		glm::vec3 normal = allNormals[n];

		if (checkForSeparatingAxisAlongNormal(s1, s2, normal, s1Rot, s2Rot)) {
			return false;
		}
	}

	//Couldn't find a single separating axis betweenshapes, must be in contact
	return true;
}

	//TODO could cache global coords of corners in a vector
bool CollisionDetector::checkForSeparatingAxisAlongNormal(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &normal, glm::mat4 &s1Rot, glm::mat4 &s2Rot) {
	float max1, min1, max2, min2;
	max1 = max2 = -100000.0f;
	min1 = min2 = 100000.0f;

	for (int v1 = 0; v1 < s1.corners.size(); v1++) {
		glm::vec3 pt = ShapeUtils::getGlobalCoordinates(s1.corners[v1], s1.translation, s1Rot, s1.scaling);

		float proj = glm::dot(pt, normal);

		if (proj > max1) {
			max1 = proj;
		}
		if (proj < min1) {
			min1 = proj;
		}
	}

	for (int v2 = 0; v2 < s2.corners.size(); v2++) {
		glm::vec3 pt = ShapeUtils::getGlobalCoordinates(s2.corners[v2], s2.translation, s2Rot, s2.scaling);

		float proj = glm::dot(pt, normal);

		if (proj > max2) {
			max2 = proj;
		}
		if (proj < min2) {
			min2 = proj;
		}
	}

	if (max1 < min2 || max2 < min1) {
		return true; //Found separating axis
	}

	return false; //Couldn't find one
}