#ifndef SHAPE_UTILS_HG
#define SHAPE_UTILS_HG

#include <vector>
#ifndef GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>


class ShapeUtils {

public:

	static void getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset = 128.0f);

	static glm::vec3 getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);

	static void addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec);

	//Tangent and binormal vectors of n will be 2nd and 3rd columns of H, respectively
	static void HouseholderOrthogonalization(glm::dvec3 n, glm::mat3 &H);

	static float squaredDistance(glm::vec3 pt1, glm::vec3 pt2) {
		float xDiff, yDiff, zDiff;
		xDiff = pt1.x - pt2.x;
		yDiff = pt1.y - pt2.y;
		zDiff = pt1.z - pt2.z;
		return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
	}

	static glm::mat4 rotationFromAtoB(glm::vec3 a, glm::vec3 b) {
		a = glm::normalize(a);
		b = glm::normalize(b);
		glm::vec3 rotAxis = glm::cross(a, b);
		float rotAngle = acos(glm::dot(b, a));

		//In case b is parallel with a :
		if (absf(a.x - b.x) < .0001f && absf(a.y - b.y) < .0001f && absf(a.z - b.z) < .0001f) {

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

	static float absf(float x) {
		if (x < 0.0f) {
			return -1.0f * x;
		}
		return x;
	}

	static bool isZeroVec(glm::vec3 vec) {
		return (absf(vec.x) < 0.0001f && absf(vec.y) < 0.0001f && absf(vec.z) < 0.0001f);
	}

};

#endif