#pragma once

#include <math.h>
#include "HashMap.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>


class MathUtils {

private:

	static HashMap cosMap;
	static HashMap sinMap;

	//given a trignometric input in radians, 
	//converts it to it's equivalent value within the range [0, 2pi]
	static float convertTrigInputToStandardRange(float x);

public:

	static const float PI;
	static const float twoPI;

	static const glm::mat4 I4;

	static float cos(float x);

	static float sin(float x);

	static float abs(float x);

	/* Returns the sign of x */
	static float sgnf(float x);

	/* Returns a rotation matrix that rotates vector a to vector b */
	static glm::mat4 rotationFromAtoB(glm::vec3 a, glm::vec3 b);

};