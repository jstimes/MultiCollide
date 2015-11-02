#include "ShapeUtils.h"

void ShapeUtils::getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset){
	const GLdouble TWO_PI = glm::pi<double>() * 2.0f;
	GLdouble radInc = glm::pi<double>() / ptOffset;

	for (GLdouble rad = 0; rad <= TWO_PI; rad += radInc){
		circleXpts.push_back(radius*cos(rad));
		circleZpts.push_back(radius*sin(rad));
	}
}

glm::dvec3 ShapeUtils::getNormalOfTriangle(glm::dvec3 &p1, glm::dvec3 &p2, glm::dvec3 &p3){
	glm::dvec3 normal;
	glm::dvec3 V = p2 - p1;
	glm::dvec3 W = p3 - p1;
	normal.x = (V.y * W.z) - (V.z * W.y);
	normal.y = (V.z * W.z) - (V.x * W.z);
	normal.z = (V.x * W.y) - (V.y * W.x);
	return normal;
}

void ShapeUtils::addTriangleToVector(glm::dvec3 p1, glm::dvec3 p2, glm::dvec3 p3, glm::dvec3 normal, std::vector<GLdouble> &vec){
	vec.push_back(p1.x);
	vec.push_back(p1.y);
	vec.push_back(p1.z);
	vec.push_back(normal.x);
	vec.push_back(normal.y);
	vec.push_back(normal.z);

	vec.push_back(p2.x);
	vec.push_back(p2.y);
	vec.push_back(p2.z);
	vec.push_back(normal.x);
	vec.push_back(normal.y);
	vec.push_back(normal.z);

	vec.push_back(p3.x);
	vec.push_back(p3.y);
	vec.push_back(p3.z);
	vec.push_back(normal.x);
	vec.push_back(normal.y);
	vec.push_back(normal.z);
}