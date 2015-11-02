#ifndef SHAPE_UTILS_HG
#define SHAPE_UTILS_HG

#include <vector>
#ifndef GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>


class ShapeUtils {

public:

	static void getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset = 128.0f);

	static glm::dvec3 getNormalOfTriangle(glm::dvec3 &p1, glm::dvec3 &p2, glm::dvec3 &p3);

	static void addTriangleToVector(glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3, glm::dvec3 normal, std::vector<GLdouble> &vec);

};

#endif