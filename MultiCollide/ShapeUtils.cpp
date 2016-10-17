#include "ShapeUtils.h"

void ShapeUtils::getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset) {
	const GLdouble TWO_PI = glm::pi<double>() * 2.0f;
	GLdouble radInc = glm::pi<double>() / ptOffset;

	for (GLdouble rad = 0; rad <= TWO_PI; rad += radInc) {
		circleXpts.push_back(radius*cos(rad));
		circleZpts.push_back(radius*sin(rad));
	}
}

glm::vec3 ShapeUtils::getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3) {
	glm::dvec3 normal;
	glm::dvec3 V = p2 - p1;
	glm::dvec3 W = p3 - p1;
	normal.x = (V.y * W.z) - (V.z * W.y);
	normal.y = (V.z * W.z) - (V.x * W.z);
	normal.z = (V.x * W.y) - (V.y * W.x);
	return normal;
}

//TODO max sure glm::length is same as Euclidean norm
//Tangent and binormal vectors of n will be 2nd and 3rd columns of H, respectively
void ShapeUtils::HouseholderOrthogonalization(glm::dvec3 n, glm::mat3 &H) {
	//Determine vector h:
	double normN = glm::length(n);
	glm::dvec3 h;
	h.x = absf(n.x - normN) > absf(n.x + normN) ? absf(n.x - normN) : absf(n.x + normN);
	h.y = n.y;
	h.z = n.z;

	//Determine matrix H: 
	double normH = glm::length(h);

	//H = I3 - 2( h*h^t / h^t*h )  -> h times h transpose divided by dot product of h with itself
	glm::dmat3 identity;
	glm::dmat3 hTimesTranspose = glm::outerProduct(h, h);
	double hTransposeTimesH = glm::dot(h, h);
	hTimesTranspose = hTimesTranspose / hTransposeTimesH;
	hTimesTranspose = hTimesTranspose * 2.0;

	H = identity - hTimesTranspose;
}

void ShapeUtils::addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec) {
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