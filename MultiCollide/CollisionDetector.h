#pragma once

#include <math.h>
#include <vector>
#include <map>
#include <glm/glm.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "Shape.h"
#include "ShapeUtils.h"
#include "Superquadric.h"
#include "Cube.h"

//Visitor pattern
class CollisionDetector {

	//Maybe?
	std::map<Shape, std::string> map;
	 
};