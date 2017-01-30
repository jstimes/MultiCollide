#pragma once

#include <vector>
#include "Shape.h"

//Shapes that will utilize the Separating Axis Theorem for Collision Detection
class ShapeSeparatingAxis : public Shape {

public:

	//For Collision Detection
	std::vector<glm::vec3> corners;
	std::vector<glm::vec3> normals;

};