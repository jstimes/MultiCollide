#pragma once

#include <string>
#include <vector>
#include "ShapeUtils.h"
#include "Shader.h"


#include "ArrowObject.h"


class Arrow {

	

	static const float defaultArrowHeadScaling;

public:

	///Draws an arrow based on the parameters:
	/// arrowLength - distance from arrow start to arrow head
	/// headScaling - scales the size of the arrow head by this fraction (1.0 is default, .5 is half that size, 2.0 is double default)
	/// ptToRotateTo - the point to which the arrow should point in its local space
	/// translation - gives the position of where the arrow should start at
	static void Draw(Shader &shader, float arrowLength, float headScaling, glm::vec3 ptToRotateArrowTo, glm::vec3 translation);

};
