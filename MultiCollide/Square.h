#pragma once

#include "Shape2D.h"

class Square : public Shape2D {

public:

	Square() {

		glm::vec3 a(.5f, -.5f, 0.0f);
		glm::vec3 b(.5f, .5f, 0.0f);
		glm::vec3 c(-.5f, .5f, 0.0f);
		glm::vec3 d(-.5f, -.5f, 0.0f);
		glm::vec3 norm(0.0f, 0.0f, 1.0f);

		ShapeUtils::addTriangleToVector(a, b, c, norm, vertices);
		ShapeUtils::addTriangleToVector(c, d, a, norm, vertices);

		corners.push_back(a);
		corners.push_back(b);
		corners.push_back(c);
		corners.push_back(d);

		normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
		normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

		boundingSphereRadius = sqrtf(.8083f*.8083f) + BoundingSphereBuffer;
	}
};