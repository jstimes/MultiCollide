#pragma once

#include "Shape2D.h"

class Triangle : public Shape2D {

public:

	Triangle() {
		glm::vec3 a(-.7f, -.4041f, 0.0f);
		glm::vec3 b(.7f, -.4041f, 0.0f);
		glm::vec3 c(0.0f, .8083f, 0.0f);
		glm::vec3 norm(0.0f, 0.0f, 1.0f);

		ShapeUtils::addTriangleToVector(a, b, c, norm, vertices);

		corners.push_back(a);
		corners.push_back(b);
		corners.push_back(c);

		normals.push_back(ShapeUtils::rightNormalOfEdge(a, b));
		normals.push_back(ShapeUtils::rightNormalOfEdge(b, c));
		normals.push_back(ShapeUtils::rightNormalOfEdge(c, a));

		boundingSphereRadius = sqrtf(.8083f*.8083f) + BoundingSphereBuffer;
	}

};