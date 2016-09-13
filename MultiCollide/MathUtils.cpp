#include "MathUtils.h"

HashMap MathUtils::cosMap = HashMap();
HashMap MathUtils::sinMap = HashMap();

float MathUtils::cos(float x) {
	if (!cosMap.ContainsKey(x)) {
		//Hasn't been computed before:
		float cos = cosf(x);
		cosMap.Put(x, cos);
		return cos;
	}
	return cosMap.Get(x);
}

float MathUtils::sin(float x) {
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