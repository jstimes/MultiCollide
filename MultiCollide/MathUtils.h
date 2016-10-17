#pragma once

#include <math.h>
//#include <map>
#include "HashMap.h"


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

	static float cos(float x);

	static float sin(float x);

	static float abs(float x);

	/* Returns the sign of x */
	static float sgnf(float x);

};