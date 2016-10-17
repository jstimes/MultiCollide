#include "MathUtils.h"


HashMap MathUtils::cosMap = HashMap();
HashMap MathUtils::sinMap = HashMap();

const glm::mat4 MathUtils::I4 = glm::mat4();
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

glm::mat4 MathUtils::rotationFromAtoB(glm::vec3 a, glm::vec3 b) {
	a = glm::normalize(a);
	b = glm::normalize(b);
	glm::vec3 rotAxis = glm::cross(a, b);
	float rotAngle = acos(glm::dot(b, a));

	//In case b is parallel with a :
	if (MathUtils::abs(a.x - b.x) < .0001f && MathUtils::abs(a.y - b.y) < .0001f && MathUtils::abs(a.z - b.z) < .0001f) {

		//Same vector:
		if (glm::dot(a, b) > 0.0f) {
			return glm::mat4();
		}
		// Opposite vectors
		else {

			rotAngle = glm::pi<float>();
		}
	}

	return glm::rotate(glm::mat4(), rotAngle, rotAxis);
}