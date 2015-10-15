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
	std::vector<GLfloat> vertices;

	Cylinder(float radius, float height)  {
		GLfloat TWO_PI = glm::pi<float>() * 2.0f;
		GLfloat diffBetweenCircles = .01f;
		int numCircles = (int) height / diffBetweenCircles;
		GLfloat radInc = glm::pi<float>() / 128.0f;

		std::vector<GLfloat> circleXpts;
		std::vector<GLfloat> circleZpts;
		ShapeUtils::getCirclePoints(radius, circleXpts, circleZpts);
		int circlePts = circleXpts.size();

		int heightOffset = numCircles / 2; //TODO an input param?
	
		for (int h = 0 - heightOffset; h < numCircles - heightOffset; h++){
			for (int circ = 0; circ < circlePts; circ++){
				GLfloat x1 = circleXpts[circ];
				GLfloat y1 = diffBetweenCircles * h;
				GLfloat z1 = circleZpts[circ];

				GLfloat x2 = x1;
				GLfloat y2 = y1 + diffBetweenCircles;
				GLfloat z2 = z1;

				GLfloat x3, z3;
				if (circ == circlePts - 1){
					x3 = circleXpts[0];
					z3 = circleZpts[0];
				}
				else{
					x3 = circleXpts[circ + 1];
					z3 = circleZpts[circ + 1];
				}
				GLfloat y3 = y1;

				GLfloat x4 = x3;
				GLfloat y4 = y2;
				GLfloat z4 = z3;

				glm::vec3 p1(x1, y1, z1);
				glm::vec3 p2(x2, y2, z2);
				glm::vec3 p3(x3, y3, z3);
				glm::vec3 p4(x4, y4, z4);

				// Triangles look like:
				///  p2 -- p4
				///  |  \  |
				///  p1 -- p3

				glm::vec3 normal1 = ShapeUtils::getNormalOfTriangle(p1, p2, p3);
				glm::vec3 normal2 = ShapeUtils::getNormalOfTriangle(p3, p2, p4);

				ShapeUtils::addTriangleToVector(p1, p2, p3, normal1, vertices);
				ShapeUtils::addTriangleToVector(p3, p2, p4, normal2, vertices);
			}
		}
	}

	static TetrahedralMesh MeshCylinder(float radius, float height){
		TetrahedralMesh mesh;
		GLfloat TWO_PI = glm::pi<float>() * 2.0f;
		GLfloat diffBetweenCircles = .01f;
		int numCircles = (int)height / diffBetweenCircles;
		GLfloat radInc = glm::pi<float>() / 128.0f;

		std::vector<GLfloat> circleXpts;
		std::vector<GLfloat> circleZpts;
		ShapeUtils::getCirclePoints(radius, circleXpts, circleZpts);
		int circlePts = circleXpts.size();

		int heightOffset = numCircles / 2; //TODO an input param?

		for (int h = 0 - heightOffset; h < numCircles - heightOffset; h++){
			for (int circ = 0; circ < circlePts; circ++){
				GLfloat x1 = circleXpts[circ];
				GLfloat y1 = diffBetweenCircles * h;
				GLfloat z1 = circleZpts[circ];

				GLfloat x2 = x1;
				GLfloat y2 = y1 + diffBetweenCircles;
				GLfloat z2 = z1;

				GLfloat x3, z3;
				if (circ == circlePts - 1){
					x3 = circleXpts[0];
					z3 = circleZpts[0];
				}
				else{
					x3 = circleXpts[circ + 1];
					z3 = circleZpts[circ + 1];
				}
				GLfloat y3 = y1;

				GLfloat x4 = x3;
				GLfloat y4 = y2;
				GLfloat z4 = z3;

				glm::vec3 p1(x1, y1, z1);
				glm::vec3 p2(x2, y2, z2);
				glm::vec3 p3(x3, y3, z3);
				glm::vec3 p4(x4, y4, z4);

				//TETRAHEDRON:
				glm::vec3 d1(0.0f, y1, 0.0f);
				glm::vec3 d2(0.0f, y2, 0.0f);

				// Triangles look like:
				///  p2 -- p4
				///  |  \  |
				///  p1 -- p3

				glm::vec3 normal1 = ShapeUtils::getNormalOfTriangle(p1, p2, p3);
				glm::vec3 normal2 = ShapeUtils::getNormalOfTriangle(p3, p2, p4);

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