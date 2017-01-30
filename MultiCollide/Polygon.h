#pragma once

#include "Shape2D.h"

class Polygon : public Shape2D {

public:

	int numSides;

	float radius;

	Polygon(int numSides, float radius = 1.0f) : numSides(numSides), radius(radius) {

		this->is2D = true;

		//Just use client side validation (in JS)... Emscripten doesn't support Exceptions
		//if numSides < 3 throw exception...

		glm::vec3 norm(0.0f, 0.0f, 1.0f);

		float startAngle = MathUtils::PI / 2.0f;
		float centerAngle = 2.0f * MathUtils::PI / numSides;

		if (numSides % 2 == 0) {
			startAngle -= (centerAngle / 2.0f);
		}

		glm::vec3 center(0.0f);
		for (int i = 0; i < numSides; i++) {
			float angle = startAngle + (i * centerAngle);
			glm::vec3 a(radius * MathUtils::cos(angle), radius * MathUtils::sin(angle), 0.0f);

			angle += centerAngle;
			glm::vec3 b(radius * MathUtils::cos(angle), radius * MathUtils::sin(angle), 0.0f);

			ShapeUtils::addTriangleToVector(center, a, b, norm, vertices);

			corners.push_back(b);
			normals.push_back(ShapeUtils::rightNormalOfEdge(a, b));
		}

		boundingSphereRadius = radius + BoundingSphereBuffer;
	}

};