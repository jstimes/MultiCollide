#pragma once

#include <math.h>
#include "HashMap.h"

#include <Eigen/Dense>

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
	static const float PI_OVER_2;
	static const float PI_OVER_4;

	static const glm::mat4 I4;

	static float cos(float x);

	static float sin(float x);

	static float abs(float x);

	/* Returns the sign of x */
	static float sgnf(float x);

	/* Returns a rotation matrix that rotates vector a to vector b */
	static glm::mat4 rotationFromAtoB(glm::vec3 &a, glm::vec3 &b);

	/* Returns length of vector */
	static float magnitude(glm::vec3 &a);

	static Eigen::Vector3d glmToEigenVector(glm::vec3 &glmVec);

	static glm::vec3 eigenToGlmVector(Eigen::Vector3d &eigenVec);

	static Eigen::Matrix3d glmToEigenMatrix(glm::mat3 &glmMat);

	static void printMat(glm::mat3 &mat);

	static void printMat(glm::mat4 &mat);

	static bool checkOppositeSigns(float a, float b);

	static glm::vec2 solveQuadratic(float a, float b, float c);

	static glm::vec3 projectionOfAontoB(glm::vec3 &a, glm::vec3 &b);

	static bool isZeroVec(glm::vec3 &vec);

	static float clamp(float value, float min, float max);

};