#pragma once

#include "Shape2D.h"

class Circle : public Shape2D {

public:

	float radius = .75f;

	int numSegments = 36;

	

	Circle() {

		float offset = (MathUtils::twoPI) / numSegments;
		glm::vec3 norm(0.0f, 0.0f, 1.0f);
		glm::vec3 origin(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < numSegments; i++) {
			float rads = offset * i;
			glm::vec3 a(MathUtils::cos(rads), MathUtils::sin(rads), 0.0f);

			rads += offset;
			glm::vec3 b(MathUtils::cos(rads), MathUtils::sin(rads), 0.0f);

			corners.push_back(b);

			normals.push_back(ShapeUtils::rightNormalOfEdge(a, b));

			ShapeUtils::addTriangleToVector(origin, a, b, norm, vertices);
		}

		boundingSphereRadius = radius + BoundingSphereBuffer;
	}

};