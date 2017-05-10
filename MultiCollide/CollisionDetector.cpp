#include "CollisionDetector.h"

#include "Shape.h"
#include "ShapeUtils.h"
#include "Superquadric.h"
#include "ShapeSeparatingAxis.h"

bool CollisionDetector::Detect(Shape &s1, Shape &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-S CD" << std::endl;
	std::cout << "No collision detection scheme identified for shapes in contact." << std::endl;
	std::cout << "Shapes: " << s1.name << ", " << s2.name << std::endl;
	return false;
}

bool CollisionDetector::Detect(Shape &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-SQ CD" << std::endl;
	std::cout << "Couldn't identify type of: " << s1.name;
	return false;
}

bool CollisionDetector::Detect(Shape &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	std::cout << "S-SAT CD" << std::endl;
	std::cout << "Couldn't identify type of: " << s1.name;
	return false;
}

bool CollisionDetector::Detect(Superquadric &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	return Detect(s2, s1, closestPt2, closestPt1, pp2, pp1);
}

bool CollisionDetector::Detect(ShapeSeparatingAxis &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	//std::cout << "SAT-SQ CD" << std::endl;
	//Will basically do bounding box intersection for the superquadric

	ShapeSeparatingAxis& ssa = static_cast<ShapeSeparatingAxis&>(s2);
	if (Detect(s1, ssa, closestPt1, closestPt2, pp1, pp2)) {
		//TODO probably want more exact checking
		return true;
	}
	return false;
}

bool CollisionDetector::Detect(Superquadric &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) {
	//std::cout << "SQ-SQ CD" << std::endl;

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

	//std::cout << "SAT-SAT CD" << std::endl;

	glm::mat4 s1Rot, s2Rot;
	s1Rot = s1.getRotationMatrix();
	s2Rot = s2.getRotationMatrix();

	float minOverlap = 10000.0f;
	glm::vec3 minOverlapNormal(0.0f);

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

		float overlap = 0.0f;
		if (checkForSeparatingAxisAlongNormal(s1, s2, normal, s1Rot, s2Rot, overlap)) {
			return false;
		}

		if (overlap < minOverlap) {
			minOverlap = overlap;
			minOverlapNormal = normal;
		}
	}
	
	//If the loop finished, the algorithm
	//Couldn't find a single separating axis between shapes, must be in contact

	//Find contact pt:
	glm::vec3 s1ToNorm = minOverlapNormal - s1.translation;
	if (glm::dot(s1ToNorm, s2.translation) < 0) {
		//minOverlapNormal is s2's normal. Negate for s1 
		minOverlapNormal *= -1.0f;
	}

	// Go over all corners of s1.
	// Project each onto the minOverlapNormal
	// Biggest value is the one closest to s2
	// Track two closest pts to s2. 
	// IF they are about equal, contact pt is along edge,
	// Otherwise, the greater value is closest pt
	float s1MaxDist1 = -100.0f;
	glm::vec3 s1MaxCorner1(0.0f);
	float s1DirMag1 = -100.0f;
	float s1DirMag2 = -100.0f;
	float s1MaxDist2 = -100.0f;
	glm::vec3 s1MaxCorner2(0.0f);

	glm::vec3 s1ToS2 = s2.translation - s1.translation;

	for (int i = 0; i < s1.corners.size(); i++) {
		glm::vec3 pt = ShapeUtils::getGlobalCoordinates(s1.corners[i], s1.translation, s1Rot, s1.scaling);
		//glm::vec3 s1ProjNormal = MathUtils::projectionOfAontoB(pt, minOverlapNormal);

		//float projMag = MathUtils::magnitude(s1ProjNormal);
		float projMag = glm::dot(pt, minOverlapNormal);
		float dirMag = glm::dot(pt, s1ToS2);

		if ((MathUtils::abs(projMag - s1MaxDist1) < .01f && dirMag > s1DirMag1) || projMag > s1MaxDist1) {
			s1MaxDist2 = s1MaxDist1;
			s1MaxCorner2 = s1MaxCorner1;
			s1DirMag2 = s1DirMag1;

			s1DirMag1 = dirMag;
			s1MaxDist1 = projMag;
			s1MaxCorner1 = pt;
		}
		else if ((MathUtils::abs(projMag - s1MaxDist2) < .01f && dirMag > s1DirMag2) || projMag > s1MaxDist2) {
			s1MaxDist2 = projMag;
			s1MaxCorner2 = pt;
			s1DirMag2 = dirMag;
		}
	}

	if (s1MaxDist1 - s1MaxDist2 < .01f) {
		s1MaxCorner1 = (s1MaxCorner1 + s1MaxCorner2) / 2.0f;
	}
	pp1.pt = closestPt1 = s1MaxCorner1; 

	//Repeat for s2, flip normal tho
	minOverlapNormal *= -1.0f;

	glm::vec3 s2ToS1 = s1.translation - s2.translation;

	float s2MaxDist1 = -100.0f;
	glm::vec3 s2MaxCorner1(0.0f);
	float s2dirMag1 = -100.0f;
	float s2MaxDist2 = -100.0f;
	float s2dirMag2 = -100.0f;
	glm::vec3 s2MaxCorner2(0.0f);

	for (int i = 0; i < s2.corners.size(); i++) {
		glm::vec3 pt = ShapeUtils::getGlobalCoordinates(s2.corners[i], s2.translation, s2Rot, s2.scaling);
		//glm::vec3 s2ProjNormal = MathUtils::projectionOfAontoB(pt, minOverlapNormal);

		//float projMag = MathUtils::magnitude(s2ProjNormal);
		float projMag = glm::dot(pt, minOverlapNormal);
		float dirMag = glm::dot(pt, s2ToS1);

		if ((MathUtils::abs(projMag - s2MaxDist1) < .01f && dirMag > s2dirMag1) || projMag > s2MaxDist1) {
			s2MaxDist2 = s2MaxDist1;
			s2MaxCorner2 = s2MaxCorner1;
			s2dirMag2 = s2dirMag1;

			s2dirMag1 = dirMag;
			s2MaxDist1 = projMag;
			s2MaxCorner1 = pt;
		}
		else if ((MathUtils::abs(projMag - s2MaxDist2) < .01f && dirMag > s2dirMag2) || projMag > s2MaxDist2) {
			s2MaxDist2 = projMag;
			s2MaxCorner2 = pt;
			s2dirMag2 = dirMag;
		}
	}

	if (s2MaxDist1 - s2MaxDist2 < .01f) {
		s2MaxCorner1 = (s2MaxCorner1 + s2MaxCorner2) / 2.0f;
	}
	pp2.pt = closestPt2 = s2MaxCorner1;

	//Find midpt between closest pts determined on both objects:
	glm::vec3 midPt = (closestPt1 + closestPt2) / 2.0f;

	pp2.pt = closestPt2 = pp1.pt = closestPt1 = midPt;

	return true;
}

void CollisionDetector::SlideS2onS1(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestS1, glm::vec3 &closestS2, glm::vec3 &s2Trans) {

	//std::cout << "SAT-SAT CD" << std::endl;

	glm::mat4 s1Rot, s2Rot;
	s1Rot = s1.getRotationMatrix();
	s2Rot = s2.getRotationMatrix();

	//Assuming a corner on s2 is in contact with an edge on s1
	// Find the edge and project s2's translation on to the edge
	float min1, min2;
	glm::vec3 minCorner1, minCorner2;
	min1 = min2 = 1000.0f;

	for (int i2 = 0; i2 < s1.corners.size(); i2++) {
		glm::vec3 globalCorner = ShapeUtils::getGlobalCoordinates(s1.corners[i2], s1.translation, s1Rot, s1.scaling);

		float dist = ShapeUtils::squaredDistance(closestS2, globalCorner);

		if (dist < min1) {
			min2 = min1;
			minCorner2 = minCorner1;

			min1 = dist;
			minCorner1 = globalCorner;
		}
		else if (dist < min2) {
			min2 = dist;
			minCorner2 = globalCorner;
		}
	}

	glm::vec3 edgeVec = minCorner1 - minCorner2;
	std::cout << "Edge vec: " << edgeVec.x << ", " << edgeVec.y << ", " << edgeVec.z << std::endl;
	glm::vec3 projTrans = MathUtils::projectionOfAontoB(s2Trans, edgeVec);
	std::cout << "Proj trans: " << projTrans.x << ", " << projTrans.y << ", " << projTrans.z << std::endl;
	//std::cout << "Here" << std::endl;
	s2.translation += projTrans;
	s2.centroid += projTrans;

	//float minOverlap = 10000.0f;
	//glm::vec3 minOverlapNormal(0.0f);
	//
	//std::vector<glm::vec3> allNormals;
	//for (int i1 = 0; i1 < s1.normals.size(); i1++) {
	//	glm::vec3 normal = ShapeUtils::applyRotation(s1.normals[i1], s1Rot);
	//	allNormals.push_back(normal);
	//}


	//for (int i2 = 0; i2 < s2.normals.size(); i2++) {
	//	glm::vec3 normal = ShapeUtils::applyRotation(s2.normals[i2], s2Rot);
	//	allNormals.push_back(normal);
	//}

	//
	//for (int n = 0; n < allNormals.size(); n++) {
	//	glm::vec3 normal = allNormals[n];

	//	float overlap = 0.0f;
	//	if (checkForSeparatingAxisAlongNormal(s1, s2, normal, s1Rot, s2Rot, overlap)) {
	//		//Should not happen, assuming this function only gets called when shapes in contact
	//		s2.translation += s2Trans;
	//		s2.centroid += s2Trans;
	//		std::cout << "SlideS2onS1 called when not in contact" << std::endl;
	//	}

	//	overlap = MathUtils::abs(overlap);
	//	if (overlap < minOverlap) {
	//		minOverlap = overlap;
	//		minOverlapNormal = normal;
	//	}
	//}

	////If the loop finished, the algorithm
	////Couldn't find a single separating axis between shapes, must be in contact

	////Find contact pt:
	//glm::vec3 s1ToNorm = minOverlapNormal - s1.translation;
	//if (glm::dot(s1ToNorm, s2.translation) < 0) {
	//	//If this condition is true, it should mean that the min overlap normal is 
	//	// coming from s2

	//	//Assuming a corner on s1 is in contact with an edge on s2
	//	// Find the edge and project s2's translation on to the edge
	//	float min1, min2;
	//	glm::vec3 minCorner1, minCorner2;
	//	min1 = min2 = 1000.0f;

	//	for (int i2 = 0; i2 < s2.corners.size(); i2++) {
	//		glm::vec3 globalCorner = ShapeUtils::getGlobalCoordinates(s2.corners[i2], s2.translation, s2Rot, s2.scaling);

	//		float dist = ShapeUtils::squaredDistance(closestS1, globalCorner);

	//		if (dist < min1) {
	//			min2 = min1;
	//			minCorner2 = minCorner1;

	//			min1 = dist;
	//			minCorner1 = globalCorner;
	//		}
	//		else if (dist < min2) {
	//			min2 = dist;
	//			minCorner2 = globalCorner;
	//		}
	//	}

	//	glm::vec3 edgeVec = minCorner1 - minCorner2;
	//	std::cout << "Edge vec: " << edgeVec.x << ", " << edgeVec.y << ", " << edgeVec.z << std::endl;
	//	glm::vec3 projTrans = MathUtils::projectionOfAontoB(s2Trans, edgeVec);
	//	std::cout << "Proj trans: " << projTrans.x << ", " << projTrans.y << ", " << projTrans.z << std::endl;
	//	//std::cout << "Here" << std::endl;
	//	s2.translation += projTrans;
	//	s2.centroid += projTrans;
	//}
	//else {
	//	
	//}

	/*
	// Go over all corners of s1.
	// Project each onto the minOverlapNormal
	// Biggest value is the one closest to s2
	// Track two closest pts to s2. 
	// IF they are about equal, contact pt is along edge,
	// Otherwise, the greater value is closest pt
	float s1MaxDist1 = -100.0f;
	glm::vec3 s1MaxCorner1(0.0f);
	float s1DirMag1 = -100.0f;
	float s1DirMag2 = -100.0f;
	float s1MaxDist2 = -100.0f;
	glm::vec3 s1MaxCorner2(0.0f);

	glm::vec3 s1ToS2 = s2.translation - s1.translation;

	for (int i = 0; i < s1.corners.size(); i++) {
		glm::vec3 pt = ShapeUtils::getGlobalCoordinates(s1.corners[i], s1.translation, s1Rot, s1.scaling);
		//glm::vec3 s1ProjNormal = MathUtils::projectionOfAontoB(pt, minOverlapNormal);

		//float projMag = MathUtils::magnitude(s1ProjNormal);
		float projMag = glm::dot(pt, minOverlapNormal);
		float dirMag = glm::dot(pt, s1ToS2);

		if ((MathUtils::abs(projMag - s1MaxDist1) < .01f && dirMag > s1DirMag1) || projMag > s1MaxDist1) {
			s1MaxDist2 = s1MaxDist1;
			s1MaxCorner2 = s1MaxCorner1;
			s1DirMag2 = s1DirMag1;

			s1DirMag1 = dirMag;
			s1MaxDist1 = projMag;
			s1MaxCorner1 = pt;
		}
		else if ((MathUtils::abs(projMag - s1MaxDist2) < .01f && dirMag > s1DirMag2) || projMag > s1MaxDist2) {
			s1MaxDist2 = projMag;
			s1MaxCorner2 = pt;
			s1DirMag2 = dirMag;
		}
	}

	if (s1MaxDist1 - s1MaxDist2 < .01f) {
		//S2 is in contact with an edge on s1, so it can slide along it
		glm::vec3 edgeVec = s1MaxCorner1 - s1MaxCorner2;
		glm::vec3 projTrans = MathUtils::projectionOfAontoB(s2Trans, edgeVec);
		std::cout << "Here" << std::endl;
		s2.translation += projTrans;
	}*/
}

//TODO calc minOverlap ***
	//TODO could cache global coords of corners in a vector
bool CollisionDetector::checkForSeparatingAxisAlongNormal(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &normal, glm::mat4 &s1Rot, glm::mat4 &s2Rot, float &minOverlap) {
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