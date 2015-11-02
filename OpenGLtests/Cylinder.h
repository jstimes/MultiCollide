#ifndef CYLINDER_HG
#define CYLINDER_HG

#include <vector>
#ifndef GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>
#include "ShapeUtils.h"
#include "TetrahedralMesh.h"


class Cylinder {

public:
	std::vector<GLdouble> vertices;

	Cylinder(double radius, double height)  {
		GLdouble TWO_PI = glm::pi<double>() * 2.0f;
		GLdouble diffBetweenCircles = .01f;
		int numCircles = (int) height / diffBetweenCircles;
		GLdouble radInc = glm::pi<double>() / 128.0f;

		std::vector<GLdouble> circleXpts;
		std::vector<GLdouble> circleZpts;
		ShapeUtils::getCirclePoints(radius, circleXpts, circleZpts);
		int circlePts = circleXpts.size();

		int heightOffset = numCircles / 2; //TODO an input param?
	
		for (int h = 0 - heightOffset; h < numCircles - heightOffset; h++){
			for (int circ = 0; circ < circlePts; circ++){
				GLdouble x1 = circleXpts[circ];
				GLdouble y1 = diffBetweenCircles * h;
				GLdouble z1 = circleZpts[circ];

				GLdouble x2 = x1;
				GLdouble y2 = y1 + diffBetweenCircles;
				GLdouble z2 = z1;

				GLdouble x3, z3;
				if (circ == circlePts - 1){
					x3 = circleXpts[0];
					z3 = circleZpts[0];
				}
				else{
					x3 = circleXpts[circ + 1];
					z3 = circleZpts[circ + 1];
				}
				GLdouble y3 = y1;

				GLdouble x4 = x3;
				GLdouble y4 = y2;
				GLdouble z4 = z3;

				glm::dvec3 p1(x1, y1, z1);
				glm::dvec3 p2(x2, y2, z2);
				glm::dvec3 p3(x3, y3, z3);
				glm::dvec3 p4(x4, y4, z4);

				// Triangles look like:
				///  p2 -- p4
				///  |  \  |
				///  p1 -- p3

				glm::dvec3 normal1 = ShapeUtils::getNormalOfTriangle(p1, p2, p3);
				glm::dvec3 normal2 = ShapeUtils::getNormalOfTriangle(p3, p2, p4);

				ShapeUtils::addTriangleToVector(p1, p2, p3, normal1, vertices);
				ShapeUtils::addTriangleToVector(p3, p2, p4, normal2, vertices);
			}
		}
	}

	static TetrahedralMesh MeshCylinder(double radius, double height){
		TetrahedralMesh mesh;
		GLdouble TWO_PI = glm::pi<double>() * 2.0f;
		GLdouble diffBetweenCircles = .01f;
		int numCircles = (int)height / diffBetweenCircles;
		GLdouble radInc = glm::pi<double>() / 128.0f;

		std::vector<GLdouble> circleXpts;
		std::vector<GLdouble> circleZpts;
		ShapeUtils::getCirclePoints(radius, circleXpts, circleZpts);
		int circlePts = circleXpts.size();

		int heightOffset = numCircles / 2; //TODO an input param?

		for (int h = 0 - heightOffset; h < numCircles - heightOffset; h++){
			for (int circ = 0; circ < circlePts; circ++){
				GLdouble x1 = circleXpts[circ];
				GLdouble y1 = diffBetweenCircles * h;
				GLdouble z1 = circleZpts[circ];

				GLdouble x2 = x1;
				GLdouble y2 = y1 + diffBetweenCircles;
				GLdouble z2 = z1;

				GLdouble x3, z3;
				if (circ == circlePts - 1){
					x3 = circleXpts[0];
					z3 = circleZpts[0];
				}
				else{
					x3 = circleXpts[circ + 1];
					z3 = circleZpts[circ + 1];
				}
				GLdouble y3 = y1;

				GLdouble x4 = x3;
				GLdouble y4 = y2;
				GLdouble z4 = z3;

				glm::dvec3 p1(x1, y1, z1);
				glm::dvec3 p2(x2, y2, z2);
				glm::dvec3 p3(x3, y3, z3);
				glm::dvec3 p4(x4, y4, z4);

				//TETRAHEDRON:
				glm::dvec3 d1(0.0f, y1, 0.0f);
				glm::dvec3 d2(0.0f, y2, 0.0f);

				// Triangles look like:
				///  p2 -- p4
				///  |  \  |
				///  p1 -- p3

				glm::dvec3 normal1 = ShapeUtils::getNormalOfTriangle(p1, p2, p3);
				glm::dvec3 normal2 = ShapeUtils::getNormalOfTriangle(p3, p2, p4);

				//ShapeUtils::addTriangleToVector(p1, p2, p3, normal1, vertices);
				//ShapeUtils::addTriangleToVector(p3, p2, p4, normal2, vertices);
				Tetrahedron t1(p1, p2, p3, d1, normal1, normal1, normal1, normal1);
				Tetrahedron t2(p4, p2, p3, d2, normal2, normal2, normal2, normal2);
				mesh.AddTetrahedron(t1);
				mesh.AddTetrahedron(t2);
			}
		}
		return mesh;
	}
};
#endif