#include "ShapeUtils.h"
#include "Shape.h"


//void ShapeUtils::BackwardsIntegrate(Shape &shape, float seconds) {
//	float deltaT = .05f;
//
//	int iterations = ((float) seconds) / deltaT;
//
//	for (int i = 0; i < iterations; i++) {
//		shape.applyRotation(shape.angularVelocityAxis, shape.angularVelocity * deltaT);
//		shape.translation += -shape.curVelocity * deltaT;
//	}
//}

void ShapeUtils::AddVecToStringstream(glm::vec3 &vec, std::ostringstream &stream, std::string lastChar) {
	stream << vec.x << "," << vec.y << "," << vec.z << lastChar;
}

void ShapeUtils::AddPoint(std::vector<GLfloat> &vector, glm::vec3 &pt, glm::vec3 &normal) {
	vector.push_back(pt.x);
	vector.push_back(pt.y);
	vector.push_back(pt.z);
	vector.push_back(normal.x);
	vector.push_back(normal.y);
	vector.push_back(normal.z);
}

void ShapeUtils::getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset) {
	const GLdouble TWO_PI = glm::pi<double>() * 2.0f;
	GLdouble radInc = glm::pi<double>() / ptOffset;

	for (GLdouble rad = 0; rad <= TWO_PI; rad += radInc) {
		circleXpts.push_back(radius*cos(rad));
		circleZpts.push_back(radius*sin(rad));
	}
}

glm::vec3 ShapeUtils::getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3) {
	glm::vec3 normal;
	glm::vec3 V = p2 - p1;
	glm::vec3 W = p3 - p1;
	normal.x = (V.y * W.z) - (V.z * W.y);
	normal.y = (V.z * W.z) - (V.x * W.z);
	normal.z = (V.x * W.y) - (V.y * W.x);
	glm::vec3 normalizedNormal = glm::normalize(normal);
	return normalizedNormal;
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

void ShapeUtils::addTriangleToVector(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &normal, std::vector<GLfloat> &vec) {
	AddPoint(vec, p1, normal);
	AddPoint(vec, p2, normal);
	AddPoint(vec, p3, normal);
}

void ShapeUtils::addTriangleToVectorWithColor(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &normal, glm::vec4 &color, std::vector<GLfloat> &vec) {
	AddPoint(vec, p1, normal);
	AddVec4(vec, color);
	AddPoint(vec, p2, normal);
	AddVec4(vec, color);
	AddPoint(vec, p3, normal);
	AddVec4(vec, color);
}

void ShapeUtils::AddVec4(std::vector<GLfloat> &arr, glm::vec4 &vec) {
	arr.push_back(vec.x);
	arr.push_back(vec.y);
	arr.push_back(vec.z);
	arr.push_back(vec.w);
}

