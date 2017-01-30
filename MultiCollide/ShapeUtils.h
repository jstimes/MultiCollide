#pragma once

#include <vector>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "MathUtils.h"
#include "Shader.h"

//#include "Shape.h"


class ShapeUtils {

public:

	static void AddPoint(std::vector<GLfloat> &vector, glm::vec3 &pt, glm::vec3 &normal);

	static void AddVec4(std::vector<GLfloat> &arr, glm::vec4 &vec);

	static void getCirclePoints(double radius, std::vector<GLdouble> &circleXpts, std::vector<GLdouble> &circleZpts, double ptOffset = 128.0f);

	static glm::vec3 getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3);

	static void addTriangleToVector(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &normal, std::vector<GLfloat> &vec);

	static void addTriangleToVectorWithColor(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &normal, glm::vec4 &color, std::vector<GLfloat> &vec);

	//Tangent and binormal vectors of n will be 2nd and 3rd columns of H, respectively
	static void HouseholderOrthogonalization(glm::dvec3 n, glm::mat3 &H);

	static float squaredDistance(glm::vec3 pt1, glm::vec3 pt2) {
		float xDiff, yDiff, zDiff;
		xDiff = pt1.x - pt2.x;
		yDiff = pt1.y - pt2.y;
		zDiff = pt1.z - pt2.z;
		return xDiff * xDiff + yDiff * yDiff + zDiff * zDiff;
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

	//static glm::vec3 getGlobalCoordinates(glm::vec3 localCoords, glm::vec3 &translation, glm::mat4 &rot) {
	//	//  Rotate pt then translate it:
	//	glm::vec3 global = applyRotation(localCoords, rot);
	//	return global + translation;
	//}

	//static glm::vec3 getLocalCoordinates(glm::vec3 &globalPt, glm::vec3 &translation, glm::mat4 &rot) {
	//	//  Translate back to origin, then un-rotate to standard orientation
	//	glm::vec3 local = (-1.0f * translation) + globalPt;
	//	glm::mat4 transpose = glm::transpose(rot);
	//	return applyRotation(local, transpose);   //Transpose of rotation matrix is its inverse since orthogonal
	//}

	static glm::vec3 getGlobalCoordinates(glm::vec3 localCoords, glm::vec3 &translation, glm::mat4 &rot, float scale) {
		//  Rotate pt then translate it:
		localCoords *= scale;
		glm::vec3 global = applyRotation(localCoords, rot);
		return global + translation;
	}

	static glm::vec3 getLocalCoordinates(glm::vec3 &globalPt, glm::vec3 &translation, glm::mat4 &rot, float scale) {
		//  Translate back to origin, then un-rotate to standard orientation
		glm::vec3 local = (-1.0f * translation) + globalPt;
		glm::mat4 transpose = glm::transpose(rot);
		return (1.0f / scale) * applyRotation(local, transpose);   //Transpose of rotation matrix is its inverse since orthogonal
	}

	static glm::vec3 applyRotation(glm::vec3 &pt, glm::mat4 &rotation) {
		return glm::vec3(rotation * glm::vec4(pt.x, pt.y, pt.z, 1.0f));
	}

	static glm::vec3 rightNormalOfEdge(glm::vec3 &a, glm::vec3 &b) {
		glm::vec3 edgeVec = b - a;

		return glm::vec3(edgeVec.y, -edgeVec.x, 0.0f);
	}

};