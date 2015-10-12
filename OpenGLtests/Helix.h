#ifndef HELIX
#define HELIX

#include <vector>
#ifndef GLEW_STATIC
#endif
#include <GL/glew.h>
#include <glm/common.hpp>
#include <glm/gtc/constants.hpp>

class Helix {

public:

	std::vector<GLfloat> vertices;

	Helix(float radius, float coilOffset, float height){
		GLfloat a = radius;
		GLfloat b = coilOffset;
		//GLfloat tMax = glm::pi<float>() * 8.0;
		GLfloat t;
		GLfloat tInc = glm::pi<float>() / 32.0f;
		GLfloat tMax = height / tInc;

		for (t = 4.0 * tMax / -4.0f; t < tMax; t += tInc){
			vertices.push_back(a*cos(t));
			vertices.push_back(b*t);
			vertices.push_back(a*sin(t));
			vertices.push_back(0.0f); //normal for now TODO
			vertices.push_back(0.0f); //normal for now
			vertices.push_back(0.0f); //normal for now
		}
	}

};

#endif