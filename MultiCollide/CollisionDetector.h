#pragma once

#include <math.h>
#include <vector>
#include <map>
#include <glm/glm.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

class Shape;
class Superquadric;
struct ParamPoint;
class ShapeSeparatingAxis;

class CollisionDetector {

public:

	//Maybe?
	//std::map<Shape, std::string> map;

	static bool Detect(Shape &s1, Shape &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static bool Detect(Shape &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static bool Detect(Shape &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static bool Detect(Superquadric &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static bool Detect(ShapeSeparatingAxis &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static bool Detect(Superquadric &s1, Superquadric &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	//Returns true if collision detected
	static bool Detect(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2);

	static void SlideS2onS1(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &closestS1, glm::vec3 &closestS2, glm::vec3 &s2Trans);

private:

	//TODO could cache global coords of corners in a vector
	static bool checkForSeparatingAxisAlongNormal(ShapeSeparatingAxis &s1, ShapeSeparatingAxis &s2, glm::vec3 &normal, glm::mat4 &s1Rot, glm::mat4 &s2Rot, float &minOverlap);
	 
};