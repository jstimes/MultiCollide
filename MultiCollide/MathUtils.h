#pragma once

#include <math.h>
//#include <map>
#include "HashMap.h"


class MathUtils {

private:

	static HashMap cosMap;
	static HashMap sinMap;

public:

	static float cos(float x);

	static float sin(float x);

	static float abs(float x);

	/* Returns the sign of x */
	static float sgnf(float x);

};