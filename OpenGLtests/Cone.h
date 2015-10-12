#ifndef CONE_HG
#define CONE_HG
#include <vector>
#ifndef GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include "ShapeUtils.h"

class Cone {

public:

	std::vector<GLfloat> vertices;

	Cone(float baseRadius, float height, bool ptUp = true){
		

		const int heightOffset = 0;

		if(!ptUp){
			height = -height;
		}
		glm::vec3 conePoint(0.0f, height, 0.0f);

		std::vector<GLfloat> circleXpts;
		std::vector<GLfloat> circleZpts;
		ShapeUtils::getCirclePoints(baseRadius, circleXpts, circleZpts);

		int circlePts = circleXpts.size();
		for (int circ = 0; circ < circlePts; circ++){
			GLfloat x1 = circleXpts[circ];
			GLfloat y1 = 0.0f;
			GLfloat z1 = circleZpts[circ];
			GLfloat x2, z2;
			if (circ == circlePts - 1){
				x2 = circleXpts[0];
				z2 = circleZpts[0];
			}
			else{
				x2 = circleXpts[circ + 1];
				z2 = circleZpts[circ + 1];
			}
			glm::vec3 p1(x1, y1, z1);
			glm::vec3 p2(x2, y1, z2);
			ShapeUtils::addTriangleToVector(p1, p2, conePoint, ShapeUtils::getNormalOfTriangle(p1, p2, conePoint), vertices);
		}
	}

};

#endif