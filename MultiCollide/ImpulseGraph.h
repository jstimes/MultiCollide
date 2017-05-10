#pragma once

#include "Impact.h"
#include "ShapeUtils.h"
#include "ShapeShader.h"
#include "Camera.h"
#include "GLViewIncludes.h"
#include "ImpulseRenderer.h"

class ImpulseGraph {

	bool graphShouldClose = false;

	bool jacobSays = true;
	bool done = false;
	int leftRight = 0;

	int endOfSlidingIndex;
	int endOfCompressionIndex;

	glm::vec3 endOfSlidingImpulsePt;
	glm::vec3 endOfCompressionImpulsePt;
	glm::vec3 endOfSlidingVelocityPt;
	glm::vec3 endOfCompressionVelocityPt;

public:

	//Shader &shapeShader;

	int numDataPts;
	int curDataPoint = 0;

	Camera impulseCamera;

	glm::mat4 imodel;
	//glm::mat4 view;
	glm::mat4 iproj;

	glm::mat3 &rtan;

	glm::mat4 sceneTransform;

	glm::vec3 slidingColor = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 stickingColor = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 velocityColor = glm::vec3(.7f, .2f, .1f);

	GLint colorLoc;

	GLuint VAOdist, VBOdist;
	GLuint VAOvel, VBOvel;

	std::vector<GLfloat> vertices, velVertices;

	void initImpulseViews();

	ImpulseGraph(ImpactOutput &output, glm::mat3 &RTAN) : rtan(RTAN) /*: shapeShader(ShapeShader::getInstance().shader)*/ {

		if (output.impulsePts.size() == 0) {
			std::cout << "ERROR - impact occured but no impulse points" << std::endl;
			return;
		}
		
#ifndef  EMSCRIPTEN_KEEPALIVE //TODO for some reason this still executes in Emscripten
		//this->initImpulseViews();
		//std::cout << "initting impulse views" << std::endl;
#endif

		numDataPts = output.impulsePts.size();
		endOfCompressionIndex = output.endOfCompression;
		endOfSlidingIndex = output.endOfSliding;
		endOfCompressionImpulsePt = output.impulsePts[endOfCompressionIndex];
		endOfCompressionVelocityPt = output.velocityPts[endOfCompressionIndex];

		if (endOfSlidingIndex < numDataPts) {
			endOfSlidingImpulsePt = output.impulsePts[endOfSlidingIndex];
			endOfSlidingVelocityPt = output.velocityPts[endOfSlidingIndex];
		}

		impulseCamera.cameraTarget = glm::vec3(0.0f);
		impulseCamera.cameraPos = glm::vec3(.5f, .5f, .5f);
		impulseCamera.up = glm::vec3(0.0f, 1.0f, 0.0f);
		impulseCamera.view = glm::lookAt(impulseCamera.cameraPos, impulseCamera.cameraTarget, impulseCamera.up);
		imodel = glm::mat4(RTAN);
		iproj = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, -4.0f, 4.0f);

		glm::vec3 norm(1.0f, -1.0f, -1.0f);

		for (int i = 0; i < output.impulsePts.size(); i++) {
			/*vertices.push_back(output.impulsePts[i].x);
			vertices.push_back(output.impulsePts[i].y);
			vertices.push_back(output.impulsePts[i].z);*/
			ShapeUtils::AddPoint(vertices, output.impulsePts[i], norm);
		}

		std::cout << "IG here" << std::endl;

		ShapeShader::getInstance().shader.Use(); // ImpulseRenderer::getInstance().impulseShader.Use();
		glGenVertexArrays(1, &VAOdist);
		glGenBuffers(1, &VBOdist);
		glBindVertexArray(VAOdist);
		glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		GLint posIndex = ShapeShader::getInstance().shader.getAttribute("position");
		glVertexAttribPointer(posIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(posIndex);

		// Normal
		GLint normalIndex = ShapeShader::getInstance().shader.getAttribute("normal");
		glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normalIndex);
		glBindVertexArray(0); // Unbind VAO


		//Velocity pts:
		for (int i = 0; i < output.velocityPts.size(); i++) {
			//velVertices.push_back(output.velocityPts[i].x);
			//velVertices.push_back(output.velocityPts[i].y);
			//velVertices.push_back(output.velocityPts[i].z);
			ShapeUtils::AddPoint(velVertices, output.velocityPts[i], norm);
		}

		glGenVertexArrays(1, &VAOvel);
		glGenBuffers(1, &VBOvel);
		glBindVertexArray(VAOvel);
		glBindBuffer(GL_ARRAY_BUFFER, VBOvel);
		glBufferData(GL_ARRAY_BUFFER, velVertices.size() * sizeof(GLfloat), &velVertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(posIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(posIndex);

		// Normal
		glVertexAttribPointer(normalIndex, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(normalIndex);
		glBindVertexArray(0); // Unbind VAO

	}

	//All callback stuff:

	bool showImpulse = true;
	bool showVelocity = true;

	bool showXZPlane = true;
	bool showXYPlane = false;
	bool showYZPlane = false;
	bool lookDownXaxis = false;
	bool lookDownZaxis = false;
	bool lookDownYaxis = false;
	void impulseStep();

	bool shouldClose() {
		return graphShouldClose;
	}

	void cleanup();

	void checkImpulseViews(glm::vec3 &mouseClick);

	void toggleImpulseBtnOnClick() {
		showImpulse = !showImpulse;
	}


	void
		toggleVelocityBtnOnClick() {
		showVelocity = !showVelocity;
	}

	bool restartImpulse = false;
	void
		restartImpulseOnClick() {
		restartImpulse = true;
	}

	bool continueImpulse = false;
	void
		continueImpulseOnClick() {
		continueImpulse = true;
	}

	void
		showXZPlaneOnClick() {
		showXZPlane = !showXZPlane;
	}
	void
		showXYPlaneOnClick() {
		showXYPlane = !showXYPlane;
	}
	void
		showYZPlaneOnClick() {
		showYZPlane = !showYZPlane;
	}
	void
		lookDownXaxisOnClick() {
		lookDownXaxis = !lookDownXaxis;
	}
	void
		lookDownYaxisOnClick() {
		lookDownYaxis = !lookDownYaxis;
	}

	void lookDownZaxisOnClick() {
		lookDownZaxis = !lookDownZaxis;
	}
};