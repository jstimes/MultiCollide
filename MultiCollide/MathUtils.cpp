#include "MathUtils.h"


HashMap MathUtils::cosMap = HashMap();
HashMap MathUtils::sinMap = HashMap();

const glm::mat4 MathUtils::I4 = glm::mat4();
const float MathUtils::PI = glm::pi<float>();
const float MathUtils::twoPI = glm::pi<float>() * 2.0f;
const float MathUtils::PI_OVER_2 = glm::pi<float>() / 2.0f;
const float MathUtils::PI_OVER_4 = glm::pi<float>() / 4.0f;

float MathUtils::cos(float x) {
	return cosf(x);
	/*
	x = MathUtils::convertTrigInputToStandardRange(x);
	if (!cosMap.ContainsKey(x)) {
		//Hasn't been computed before:
		float cos = cosf(x);
		cosMap.Put(x, cos);
		return cos;
	}
	return cosMap.Get(x);
	*/
}

float MathUtils::sin(float x) {
	return sinf(x);
	/*
	x = MathUtils::convertTrigInputToStandardRange(x);
	if (!sinMap.ContainsKey(x)) {
		//Hasn't been computed before:
		float sin = sinf(x);
		sinMap.Put(x, sin);
		return sin;
	}
	return sinMap.Get(x);*/
}

float MathUtils::abs(float x) {
	if (x < 0.0f) {
		return -x;
	}
	return x;
}

/* Returns the sign of x */
float MathUtils::sgnf(float x) {
	if (x < 0.0f)
		return -1;
	if (x >= 0.0f)
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

float MathUtils::magnitude(glm::vec3 a) {
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

Eigen::Vector3d MathUtils::glmToEigenVector(glm::vec3 &glmVec)
{
	return Eigen::Vector3d(glmVec.x, glmVec.y, glmVec.z);
}

glm::vec3 MathUtils::eigenToGlmVector(Eigen::Vector3d &eigenVec)
{
	return glm::vec3(eigenVec(0), eigenVec(1), eigenVec(2));
}

Eigen::Matrix3d MathUtils::glmToEigenMatrix(glm::mat3 &glmMat)
{
	Eigen::Matrix3d eigenMat;

	//Both eigen and glm matrices are stored in column-major order:
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			eigenMat(j, i) = glmMat[i][j];
		}
	}

	return eigenMat;
}

void MathUtils::printMat(glm::mat3 mat) {
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

void MathUtils::printMat(glm::mat4 mat) {
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

bool MathUtils::checkOppositeSigns(float a, float b) {
	return (a > 0.0f && b < 0.0f) || (a < 0.0f && b > 0.0f);
}

glm::vec2 MathUtils::solveQuadratic(float a, float b, float c) {
	float squareRoot = sqrt(b*b - 4.0f * a * c);
	float root1 = (-b + squareRoot) / 2.0f * a;
	float root2 = (-b - squareRoot) / 2.0f * a;
	return glm::vec2(root1, root2);
}

glm::vec3 MathUtils::projectionOfAontoB(glm::vec3 &a, glm::vec3 &b) {
	glm::vec3 bHat = glm::normalize(b);

	return glm::dot(a, bHat) * b;
}