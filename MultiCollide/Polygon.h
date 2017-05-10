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

	Polygon() {
		this->is2D = true;
	}

	void AddVertex(float x, float y) {
		glm::vec3 pt(x, y, 0.0f);
		corners.push_back(pt);
	}

	void DoneAddingVertices() {

		//Compute centroid:
		glm::vec3 centroid(0.0f);
		float signedArea = 0.0f;
		float y0, y1, x0, x1, a;
		a = y1 = y0 = x1 = x0 = 0.0f;

		int numPts = corners.size();

		for (int i = 0; i < numPts; i++) {
			x0 = corners[i].x;
			y0 = corners[i].y;
			x1 = corners[(i + 1) % numPts].x;
			y1 = corners[(i + 1) % numPts].y;

			a = x0*y1 - x1*y0;
			signedArea += a;
			centroid.x += (x0 + x1)*a;
			centroid.y += (y0 + y1)*a;
		}
		signedArea *= 0.5;
		centroid.x /= (6.0*signedArea);
		centroid.y /= (6.0*signedArea);

		//Now shift all pts so shape is centered at (0, 0)
		//Also find max pts for scaling
		float maxDist = 0.0f;
		for (int i = 0; i < numPts; i++) {

			//Adjust for centroid:
			corners[i] -= centroid;

			//Check max:
			float dist = MathUtils::magnitude(corners[i]);
			if (dist > maxDist) {
				maxDist = dist;
			}
		}

		this->radius = 1.0f;
		this->scaling = 1.0f / maxDist;
		boundingSphereRadius = 1.0f + BoundingSphereBuffer;

		//Now that all pts are adjusted,  create triangles for rendering
		glm::vec3 center(0.0f);
		glm::vec3 shapeNorm(0.0f, 0.0f, 1.0f);
		for (int i = 0; i < numPts; i++) {
			//Create triangle:
			ShapeUtils::addTriangleToVector(center, corners[i], corners[(i + 1) % numPts], shapeNorm, vertices);
			normals.push_back(ShapeUtils::rightNormalOfEdge(corners[i], corners[(i + 1) % numPts]));
		}
	}

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "9";
		for (int i = 0; i < corners.size(); i++) {
			os << "," << corners[i].x << "," << corners[i].y;
		}
		os << std::endl;
		return os.str();
	}

};