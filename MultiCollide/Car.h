#pragma once

#include <math.h>
#include <vector>
#include <map>
#include <glm/glm.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "Shape.h"
#include "Superquadric.h"
#include "Shader.h"
#include "Arrow.h"

//Assuming Car is facing positive x-axis to begin
class Car : public Shape {

private:

	bool boxCar = true;
	float boxCarLengthAdjust = 1.5f;
	Shape* body;
	//  !boxCar: Superquadric body
	//   boxCar: Cube body

	Superquadric *frontLeftTire, *frontRightTire, *backLeftTire, *backRightTire;
	std::vector<Superquadric*> wheels;

	float bodyLength = 1.5f;
	float bodyRadius = .5f;

	float wheelRadius = .4f;
	float wheelWidth = .15f;
	float wheelFrontBackOffset = .6f;
	float wheelLeftRightOffset = .4f;
	float wheelUpDownOffset = -.3f;
	glm::vec3 wheelColor = glm::vec3(1.0f, .9f, .9f);

	float handling = .01f;
	float frontWheelRotation = 0.0f;
	float maxRotation = glm::pi<float>() / 4.0f;

	//Car orientation:
	glm::vec3 carNormal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 carDirection = glm::vec3(1.0f, 0.0f, 0.0f);

	float topSpeed = .65f;
	float acceleration = .025f;
	float minSpeed = -.2f;
	float brake = -.075;
	float deceleration = -.02f;

	

	float lastUpdate = -1.0f;

	float friction = .001f;

public:

	Car() {

		this->model = glm::mat4();
		
		if (boxCar) {
			body = new Cube();
			bodyLength *= boxCarLengthAdjust;
			body->scaling = glm::vec3(bodyLength , bodyRadius, bodyRadius);
		}
		else {
			Superquadric *bodySq = new Superquadric();
			bodySq->CreateSphere();
			bodySq->a1 = bodyLength;
			bodySq->a2 = bodyRadius;
			bodySq->a3 = bodyRadius;
			SuperEllipsoid::sqSolidEllipsoid(*bodySq);
			Superquadric::InitializeClosestPoints(*bodySq);
			body = bodySq;
		}

		this->boundingSphereRadius = bodyLength + BoundingSphereBuffer;

		frontLeftTire = new Superquadric();
		frontLeftTire->CreateSphere();
		frontRightTire = new Superquadric();
		frontRightTire->CreateSphere();
		backLeftTire = new Superquadric();
		backLeftTire->CreateSphere();
		backRightTire = new Superquadric();
		backRightTire->CreateSphere();

		wheels.push_back(frontLeftTire);
		wheels.push_back(frontRightTire);
		wheels.push_back(backLeftTire);
		wheels.push_back(backRightTire);

		for (int i = 0; i < wheels.size(); i++) {
			wheels[i]->a1 = wheels[i]->a2 = wheelRadius;
			wheels[i]->a3 = wheelWidth;
			wheels[i]->objectColor = wheelColor;
			SuperEllipsoid::sqSolidEllipsoid(*wheels[i]);
			Superquadric::InitializeClosestPoints(*wheels[i]);
		}
	}

	virtual void InitVAOandVBO(Shader &shader) override {

		this->velocityArrow = Arrow(this->centroid, centroid + curVelocity);
		this->angularVelocityArrow = Arrow(this->centroid, centroid + rotationAxis);

		this->body->InitVAOandVBO(shader);
		for (int i = 0; i < wheels.size(); i++) {
			wheels[i]->InitVAOandVBO(shader);
		}

	}

	void accelerate() {
		
		float curSpeed = sqrtf(curVelocity.x * curVelocity.x + curVelocity.y * curVelocity.y + curVelocity.z * curVelocity.z);
		
		if (curSpeed < topSpeed) {
			std::cout << "Car - accelerate" << std::endl;
			curVelocity += carDirection * acceleration;
		}

	}

	void deccelerate() {
		float curSpeed = sqrtf(curVelocity.x * curVelocity.x + curVelocity.y * curVelocity.y + curVelocity.z * curVelocity.z);
		if (glm::dot(carDirection, curVelocity) < 0.0f) {
			curSpeed *= -1.0f;
		}
		if (curSpeed > 0.0f) {
			curVelocity += carDirection * brake;
		}
		else if (curSpeed > minSpeed) {
			curVelocity += carDirection * deceleration;
		}
	}

	void turnLeft() {
		if (frontWheelRotation < maxRotation) {
			frontWheelRotation += handling;
		}
	}

	void turnRight() {
		if (abs(frontWheelRotation) < maxRotation) {
			frontWheelRotation -= handling;
		}
	}

	virtual void update() {
		float time = glfwGetTime();

		if (lastUpdate < 0.0f) {
			lastUpdate = time;
			return;
		}

		float elapsed = time - lastUpdate;

		lastUpdate = time;

		//Friction:
		glm::vec3 frictionResistance = glm::normalize(carDirection) * friction * (glm::dot(carDirection, curVelocity) < 0.0f ? -1.0f : 1.0f);
		if (glm::dot(curVelocity, curVelocity - frictionResistance) < 0.0f) {
			curVelocity = glm::vec3(0.0f);
		}
		else if(glm::dot(curVelocity, curVelocity) > 0.0f) {
			curVelocity -= frictionResistance;
		}
		
		

		glm::vec3 wheelDir = glm::vec3(glm::rotate(glm::mat4(), frontWheelRotation, carNormal) * glm::vec4(carDirection.x, carDirection.y, carDirection.z, 1.0f));
		wheelDir = glm::normalize(wheelDir);

		//Project 
		glm::vec3 transUpdate = elapsed * (glm::dot(this->curVelocity,wheelDir) / glm::dot(wheelDir, wheelDir)) * wheelDir;

		this->translation += transUpdate;

		float velocityWheelDot = glm::dot(glm::normalize(this->curVelocity), wheelDir);

		if (glm::dot(curVelocity, curVelocity) > 0.001f && !(velocityWheelDot < .99f)) {
			float carDirRotate = acos(velocityWheelDot) / 256.0f;
			this->rotation = glm::rotate(glm::mat4(), carDirRotate, carNormal) * this->rotation;
			carDirection = glm::normalize(glm::vec3(this->rotation * glm::vec4(carDirection.x, carDirection.y, carDirection.z, 1.0f)));
		}

	}

	virtual void Draw(Shader &shader) override {
		shader.Use();

		update();

		body->translation = this->translation;

		glm::mat4 carTrans = glm::translate(glm::mat4(), this->translation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(), this->scaling);

		this->model = carTrans * this->rotation;
		body->model = this->model * glm::scale(glm::mat4(), body->scaling) * scaleMat;

		body->Draw(shader);
		
		for (int i = 0; i < wheels.size(); i++) {
			glm::mat4 wheelPos = glm::translate(glm::mat4(), getWheelTranslation(i));

			//Front wheels may be turning
			if (i < 2) {
				wheelPos = glm::rotate(glm::mat4(), frontWheelRotation, carNormal)  * wheelPos;;
			}

			wheels[i]->model = this->model * scaleMat * wheelPos;

			wheels[i]->Draw(shader);
		}

	}

	virtual ~Car() override {
		delete body;
		for (int i = 0; i < wheels.size(); i++) {
			delete wheels[i];
		}
	}

private:

	glm::vec3 getWheelTranslation(int i) {
		if (i == 0)
		{
			return glm::vec3(wheelFrontBackOffset, wheelUpDownOffset, wheelLeftRightOffset);
		}
		else if (i == 1) {
			return glm::vec3(wheelFrontBackOffset, wheelUpDownOffset, -wheelLeftRightOffset);
		}
		else if (i == 2) {
			return glm::vec3(-wheelFrontBackOffset, wheelUpDownOffset, -wheelLeftRightOffset);
		}
		else if (i == 3) {
			return glm::vec3(-wheelFrontBackOffset, wheelUpDownOffset, wheelLeftRightOffset);
		}
		else {
			std::cout << "Jacob what" << std::endl;
		}
		
	}

};