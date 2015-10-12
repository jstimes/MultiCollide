#include "ShapeUtils.h"

void ShapeUtils::getCirclePoints(float radius, std::vector<GLfloat> &circleXpts, std::vector<GLfloat> &circleZpts, float ptOffset){
	const GLfloat TWO_PI = glm::pi<float>() * 2.0f;
	GLfloat radInc = glm::pi<float>() / ptOffset;

	for (GLfloat rad = 0; rad <= TWO_PI; rad += radInc){
		circleXpts.push_back(radius*cos(rad));
		circleZpts.push_back(radius*sin(rad));
	}
}

glm::vec3 ShapeUtils::getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3){
	glm::vec3 normal;
	glm::vec3 V = p2 - p1;
	glm::vec3 W = p3 - p1;
	normal.x = (V.y * W.z) - (V.z * W.y);
	normal.y = (V.z * W.z) - (V.x * W.z);
	normal.z = (V.x * W.y) - (V.y * W.x);
	return normal;
}

void ShapeUtils::addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec){
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