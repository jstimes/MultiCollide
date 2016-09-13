#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "Arrow.h"
#include "Shader.h"


//An abstract class that defines common properties and behaviors among all
// objects to be used in collision simulation
class Shape {

public:

	//OpenGL:

	GLuint VBO, VAO;

	glm::mat4 model;

	
	//Orientation:

	//The initial position. Should only equal translation at time=0
	glm::vec3 centroid;  

	glm::vec3 translation;

	glm::mat4 rotation;

	glm::vec3 rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);   //TODO quaternions

	float angularVelocity = 0.0f;

	glm::vec3 curVelocity;

	glm::vec3 scaling = glm::vec3(1.0f, 1.0f, 1.0f);

	float time;

	float mass = 1.0f;

	float frictionCoefficient = .25f;    // mu

	float restitutionCoefficient = 1.0f; // e

	
	//Collision Detection:

	//Used to give padding to bounding sphere in so collision is not
	// detected to late
	const float BoundingSphereBuffer = .0075f;

	//Every subclass should set this by examining the furthest point from the 
	// shape's centroid during vertex creation/iteration
	//As of now includes the buffer ^
	float boundingSphereRadius;


	//Styling:

	glm::vec3 defaultColor = glm::vec3(1.0f, 0.5f, 0.31f);

	glm::vec3 objectColor = defaultColor;

	std::string name;


	//Abstract functionality:

	//Generate buffers and configure shader in-attributes (position, normal)
	virtual void InitVAOandVBO(Shader &shader) = 0;

	//Ensure VAO is bound before the draw call
	virtual void Draw(Shader &shader) = 0;

	//Destructor is responsible for deallocating the VAO and VBO
	virtual ~Shape() = 0 { }


	Arrow velocityArrow;

	virtual void DrawInitialVelocity(Shader &shader) {
		shader.Use();

		this->velocityArrow.updatePoints(centroid, centroid + curVelocity);
		this->velocityArrow.Draw(shader);
	}

	Arrow angularVelocityArrow;

	virtual void DrawInitialAngularVelocity(Shader &shader) {
		shader.Use();

		this->angularVelocityArrow.updatePoints(centroid, centroid + rotationAxis);
		this->angularVelocityArrow.Draw(shader);
	}

};