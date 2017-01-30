#include "ImpulseGraph.h"
#include "Frame.h"

//TODO how about a set color function?
// - How about rename impulserenderer?

//Scene manip vars and funcs:
std::vector<GLView*> impulseViews;

GLButton *restart = nullptr;
GLButton *continu = nullptr;
GLButton *showXYPlaneBtn = nullptr;
GLButton *showYZPlaneBtn = nullptr;
GLButton *showXZPlaneBtn = nullptr;
GLButton *lookDownXaxisBtn = nullptr;
GLButton *lookDownYaxisBtn = nullptr;
GLButton *lookDownZaxisBtn = nullptr;

GLButton *toggleImpulseBtn = nullptr;
GLButton *toggleVelocityBtn = nullptr;


void ImpulseGraph::initImpulseViews() {
	float xCoord = -.99;
	float btnWidth = .42f;
	float btnHeight = .1f;
	float btnOffset = .175f;
	float btnYstart = .75f;

	restart = new GLButton(btnWidth, btnHeight);
	restart->SetText("Restart");
	restart->Translate(glm::vec3(xCoord, btnYstart, 0.0f));
	restart->InitBuffers();
	//restart->SetClickListener(restartImpulseOnClick);
	impulseViews.push_back(restart);

	continu = new GLButton(btnWidth, btnHeight);
	continu->SetText("Continue");
	continu->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	continu->InitBuffers();
	//continu->SetClickListener(continueImpulseOnClick);
	impulseViews.push_back(continu);

	lookDownXaxisBtn = new GLButton(btnWidth, btnHeight);
	lookDownXaxisBtn->SetText("Look down X-axis");
	lookDownXaxisBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	lookDownXaxisBtn->InitBuffers();
	//lookDownXaxisBtn->SetClickListener(lookDownXaxisOnClick);
	impulseViews.push_back(lookDownXaxisBtn);

	lookDownYaxisBtn = new GLButton(btnWidth, btnHeight);
	lookDownYaxisBtn->SetText("Look down Y-axis");
	lookDownYaxisBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	lookDownYaxisBtn->InitBuffers();
	//lookDownYaxisBtn->SetClickListener(lookDownYaxisOnClick);
	impulseViews.push_back(lookDownYaxisBtn);

	lookDownZaxisBtn = new GLButton(btnWidth, btnHeight);
	lookDownZaxisBtn->SetText("Look down Z-axis");
	lookDownZaxisBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	lookDownZaxisBtn->InitBuffers();
	//lookDownZaxisBtn->SetClickListener(lookDownZaxisOnClick);
	impulseViews.push_back(lookDownZaxisBtn);

	showXYPlaneBtn = new GLButton(btnWidth, btnHeight);
	showXYPlaneBtn->SetText("Toggle X-Y Plane");
	showXYPlaneBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	showXYPlaneBtn->InitBuffers();
	//showXYPlaneBtn->SetClickListener(showXYPlaneOnClick);
	impulseViews.push_back(showXYPlaneBtn);

	showYZPlaneBtn = new GLButton(btnWidth, btnHeight);
	showYZPlaneBtn->SetText("Toggle Y-Z Plane");
	showYZPlaneBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	showYZPlaneBtn->InitBuffers();
	//showYZPlaneBtn->SetClickListener(showYZPlaneOnClick);
	impulseViews.push_back(showYZPlaneBtn);

	showXZPlaneBtn = new GLButton(btnWidth, btnHeight);
	showXZPlaneBtn->SetText("Toggle X-Z Plane");
	showXZPlaneBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	showXZPlaneBtn->InitBuffers();
	//showXZPlaneBtn->SetClickListener(showXZPlaneOnClick);
	impulseViews.push_back(showXZPlaneBtn);

	toggleImpulseBtn = new GLButton(btnWidth, btnHeight);
	toggleImpulseBtn->SetText("Toggle Impulse Graph");
	toggleImpulseBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	toggleImpulseBtn->InitBuffers();
	//toggleImpulseBtn->SetClickListener(toggleImpulseBtnOnClick);
	impulseViews.push_back(toggleImpulseBtn);

	toggleVelocityBtn = new GLButton(btnWidth, btnHeight);
	toggleVelocityBtn->SetText("Toggle Velocity Graph");
	toggleVelocityBtn->Translate(glm::vec3(xCoord, btnYstart - impulseViews.size() * btnOffset, 0.0f));
	toggleVelocityBtn->InitBuffers();
	//toggleVelocityBtn->SetClickListener(toggleVelocityBtnOnClick);
	impulseViews.push_back(toggleVelocityBtn);
}

void ImpulseGraph::impulseStep() {
	glEnable(GL_BLEND);
	//glEnable(GL_PROGRAM_POINT_SIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int verticesPerDataPt = 6;

	//while (i < vertices.size() || jacobSays) {
	if (curDataPoint < vertices.size() || jacobSays) {
		curDataPoint += verticesPerDataPt;

		if (jacobSays && curDataPoint > vertices.size()) {
			done = true;
			impulseCamera.setTilt();

			if (restartImpulse) {
				curDataPoint = verticesPerDataPt;
				done = false;
				restartImpulse = false;
			}
			else {
				glClearColor(.8f, .8f, 1.0f, 1.0f);
				curDataPoint = vertices.size() - 1;
			}

			if (lookDownXaxis) {
				impulseCamera.lookDownXaxis();
				lookDownXaxis = false;
			}
			if (lookDownYaxis) {
				impulseCamera.lookDownYaxis();
				lookDownYaxis = false;
			}
			if (lookDownZaxis) {
				impulseCamera.lookDownZaxis();
				lookDownZaxis = false;
			}

			if (continueImpulse) {
				jacobSays = false;
				continueImpulse = false;
				graphShouldClose = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (done) {
			for (int v = 0; v < impulseViews.size(); v++) {
				impulseViews[v]->Draw();
			}
		}

		for (int graph = 0; graph < 2; graph++) {
			if (showImpulse && showVelocity) {
				float sceneScale = .75f;

				if (graph == 0) {
					float actualScale = sceneScale - leftRight * .01f;
					sceneTransform = glm::translate(MathUtils::I4, glm::vec3(-.35f - leftRight * .01f, 0.0f, 0.0f)) * glm::scale(MathUtils::I4, glm::vec3(actualScale));
				}
				else {
					float actualScale = sceneScale + leftRight * .01f;
					sceneTransform = glm::translate(MathUtils::I4, glm::vec3(.35f - leftRight * .01f, 0.0f, 0.0f)) * glm::scale(MathUtils::I4, glm::vec3(actualScale));
				}
			}
			else if (showImpulse) {
				if (graph == 0) {
					sceneTransform = MathUtils::I4;
				}
				else continue;
			}
			else if (showVelocity) {
				if (graph == 1) {
					sceneTransform = MathUtils::I4;
				}
				else continue;
			}
			else {
				continue;
			}

			ShapeShader::getInstance().shader.Use();

			/*impulseCamera.cameraPos += impulseCamera.cameraPos;
			impulseCamera.calcView();*/
			glUniformMatrix4fv(ShapeShader::getInstance().sceneTransformLoc, 1, GL_FALSE, glm::value_ptr(sceneTransform));
			glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(impulseCamera.view));
			glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(iproj));
			glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(MathUtils::I4));
			glUniform3f(ShapeShader::getInstance().lightPosLoc, 0.0f, 3.0f, 3.0f);
			glLineWidth(2.0f);
			glm::vec3 zeroVec(0.0f);
			Frame::Draw(zeroVec, MathUtils::I4, 2.0f);

			//glPointSize(8.0f);
			glLineWidth(4.0f);

			ShapeShader::getInstance().shader.Use(); //ImpulseRenderer::getInstance().impulseShader.Use();
			glUniformMatrix4fv(ShapeShader::getInstance().sceneTransformLoc /*ImpulseRenderer::getInstance().isceneTransformLoc*/, 1, GL_FALSE, glm::value_ptr(sceneTransform));
			glUniformMatrix4fv(ShapeShader::getInstance().viewLoc /*ImpulseRenderer::getInstance().iviewLoc*/, 1, GL_FALSE, glm::value_ptr(impulseCamera.view));
			imodel = MathUtils::I4; //imodel = glm::mat4(rtan);
			glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(imodel));
			glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(iproj));


			if (graph == 0) {
				glBindVertexArray(VAOdist);
				//glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
			}
			else {
				glBindVertexArray(VAOvel);
				//glBindBuffer(GL_ARRAY_BUFFER, VBOvel);
			}

			if (graph == 0) {
				if (curDataPoint / verticesPerDataPt < endOfSlidingIndex) {
					glUniform4f(ShapeShader::getInstance().objectColorLoc, slidingColor.x, slidingColor.y, slidingColor.z, 1.0f);
					glDrawArrays(GL_LINE_STRIP, 0, curDataPoint / verticesPerDataPt);
				}
				else {
					//Draw sliding points as one color
					glUniform4f(ShapeShader::getInstance().objectColorLoc, slidingColor.x, slidingColor.y, slidingColor.z, 1.0f);
					glDrawArrays(GL_LINE_STRIP, 0, endOfSlidingIndex);

					//Then draw remaining points up to i as another color:
					glUniform4f(ShapeShader::getInstance().objectColorLoc, stickingColor.x, stickingColor.y, stickingColor.z, 1.0f);
					//glDrawArrays(GL_LINE_STRIP, endOfSlidingIndex * 6, curDataPoint / verticesPerDataPt);
					glDrawArrays(GL_LINE_STRIP, endOfSlidingIndex, curDataPoint / verticesPerDataPt);
				}
			}
			else {

				glUniform4f(ShapeShader::getInstance().objectColorLoc, velocityColor.x, velocityColor.y, velocityColor.z, 1.0f);
				glDrawArrays(GL_LINE_STRIP, 0, curDataPoint / verticesPerDataPt);
				//glDrawArrays(GL_LINE_STRIP, 0, numDataPts);
			}

			if ((curDataPoint / verticesPerDataPt) >= endOfCompressionIndex) {
				//Draw sliding points as one color
				glUniform4f(ShapeShader::getInstance().objectColorLoc, 1.0f, .0f, .0f, 1.0f);

				if (graph == 0) {
					imodel = glm::translate(MathUtils::I4, endOfCompressionImpulsePt);
				}
				else {
					imodel = glm::translate(MathUtils::I4, endOfCompressionVelocityPt);
				}
				glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(imodel));

				glBindVertexArray(ImpulseRenderer::getInstance().VAOpt);
				glDrawArrays(GL_POINTS, 0, 1);
			}
			if ((curDataPoint / verticesPerDataPt) >= endOfSlidingIndex) {
				//Draw sliding points as one color
				glUniform4f(ShapeShader::getInstance().objectColorLoc, .0f, 1.0f, .0f, 1.0f);

				if (graph == 0) {
					imodel = glm::translate(MathUtils::I4, endOfSlidingImpulsePt);
				}
				else {
					imodel = glm::translate(MathUtils::I4, endOfSlidingVelocityPt);
				}
				glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(imodel));

				glBindVertexArray(ImpulseRenderer::getInstance().VAOpt);
				glDrawArrays(GL_POINTS, 0, 1);
			}

			//Draw planes last since transparent shapes must be 
			// rendered last for proper blending
			ShapeShader::getInstance().shader.Use();
			if (showXZPlane) {
				glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(MathUtils::I4));
				glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(impulseCamera.view));
				glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(iproj));
				glBindVertexArray(ImpulseRenderer::getInstance().planeVAO);
				glUniform4f(ShapeShader::getInstance().objectColorLoc, .2f, 1.0f, .2f, .3f);
				glDrawArrays(GL_TRIANGLES, 0, ImpulseRenderer::getInstance().planeVertices.size() / 6);
			}
			if (showXYPlane) {
				glm::mat4 model = glm::rotate(MathUtils::I4, -glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
				glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(impulseCamera.view));
				glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(iproj));
				glBindVertexArray(ImpulseRenderer::getInstance().planeVAO);
				glUniform4f(ShapeShader::getInstance().objectColorLoc, 0.2f, .2f, 1.0f, .3f);
				glDrawArrays(GL_TRIANGLES, 0, ImpulseRenderer::getInstance().planeVertices.size() / 6);
			}
			if (showYZPlane) {
				glm::mat4 model = glm::rotate(MathUtils::I4, glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
				glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(model));
				glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(impulseCamera.view));
				glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(iproj));
				glBindVertexArray(ImpulseRenderer::getInstance().planeVAO);
				glUniform4f(ShapeShader::getInstance().objectColorLoc, 1.0f, 0.0f, .2f, .3f);
				glDrawArrays(GL_TRIANGLES, 0, ImpulseRenderer::getInstance().planeVertices.size() / 6);
			}

			glBindVertexArray(0);
		}
		//glfwSwapBuffers(window);
	}
}

void ImpulseGraph::cleanup() {
	glBindVertexArray(0);

	glDeleteBuffers(1, &VBOdist);
	glDeleteBuffers(1, &VAOdist);

	glDeleteBuffers(1, &VBOvel);
	glDeleteBuffers(1, &VAOvel);

	//ImpulseRenderer::instance.cleanup();

	for (int v = impulseViews.size() - 1; v >= 0; v--) {
		GLView *back = impulseViews.back();
		impulseViews.pop_back();
		back->DeleteBuffers();
		delete back;
		back = nullptr;
	}
}

void ImpulseGraph::checkImpulseViews(glm::vec3 &mouseClick) {
	for (int v = 0; v < impulseViews.size(); v++) {
		if (impulseViews[v]->CheckClicked(mouseClick)) {
			impulseViews[v]->OnClick(mouseClick, nullptr);
		}
	}
}