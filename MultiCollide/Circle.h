#pragma once

#include "Shape2D.h"

class Circle : public Shape2D {

public:

	float radius = .75f;

	int numSegments = 26;

	Circle() {

		float offset = (MathUtils::twoPI) / numSegments;
		glm::vec3 norm(0.0f, 0.0f, 1.0f);
		glm::vec3 origin(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < numSegments; i++) {
			float rads = offset * i;
			glm::vec3 a = radius * glm::vec3(MathUtils::cos(rads), MathUtils::sin(rads), 0.0f);

			rads += offset;
			glm::vec3 b = radius * glm::vec3(MathUtils::cos(rads), MathUtils::sin(rads), 0.0f);

			corners.push_back(b);

			normals.push_back(ShapeUtils::rightNormalOfEdge(a, b));

			ShapeUtils::addTriangleToVector(origin, a, b, norm, vertices);
		}

		boundingSphereRadius = radius + BoundingSphereBuffer;
	}

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "6" << std::endl;
		return os.str();
	}

};