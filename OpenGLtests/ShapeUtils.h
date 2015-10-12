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

	static void getCirclePoints(float radius, std::vector<GLfloat> &circleXpts, std::vector<GLfloat> &circleZpts, float ptOffset = 128.0f);

	static glm::vec3 getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);

	static void addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec);

};

#endif