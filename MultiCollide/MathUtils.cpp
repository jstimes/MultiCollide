#include "MathUtils.h"
#include <glm/gtc/constants.hpp>

HashMap MathUtils::cosMap = HashMap();
HashMap MathUtils::sinMap = HashMap();

const float MathUtils::PI = glm::pi<float>();
const float MathUtils::twoPI = glm::pi<float>() * 2.0f;

float MathUtils::cos(float x) {
	x = MathUtils::convertTrigInputToStandardRange(x);
	if (!cosMap.ContainsKey(x)) {
		//Hasn't been computed before:
		float cos = cosf(x);
		cosMap.Put(x, cos);
		return cos;
	}
	return cosMap.Get(x);
}

float MathUtils::sin(float x) {
	x = MathUtils::convertTrigInputToStandardRange(x);
	if (!sinMap.ContainsKey(x)) {
		//Hasn't been computed before:
		float sin = sinf(x);
		sinMap.Put(x, sin);
		return sin;
	}
	return sinMap.Get(x);
}

float MathUtils::abs(float x) {
	if (x < 0.0f) {
		return -x;
	}
	return x;
}

/* Returns the sign of x */
float MathUtils::sgnf(float x) {
	if (x < 0)
		return -1;
	if (x > 0)
		return 1;
	return 0;
}

//given a trignometric input in radians, 
//converts it to it's equivalent value within the range [0, 2pi]
float MathUtils::convertTrigInputToStandardRange(float x) {
	if (x > MathUtils::twoPI) {
		x = fmodf(x, MathUtils::twoPI);
	}
	else if (x < 0.0f) {
		x = fmodf(x, -1.0f * MathUtils::twoPI); // gives value in [-2pi, 0]

		x += MathUtils::twoPI;
	}
	return x;
}