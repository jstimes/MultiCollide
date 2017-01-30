#pragma once

#include "ShapeUtils.h"

class Camera {

	//GLint viewLoc;

public:

	enum CameraState { pan = 0, tilt = 1, zoom = 2 };
	CameraState cameraState = pan;

	glm::mat4 view;

	glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
	glm::vec3 cameraPos = initialCameraPos;
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	void resetCameraState() {
		cameraPos = initialCameraPos;
		cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void setPan() {
		cameraState = pan;
	}

	void setTilt() {
		cameraState = tilt;
	}

	void setZoom() {
		cameraState = zoom;
	}


	//float cameraAdjust = .1f;
	//void CameraUpOnClick() {
	//	glm::mat4 rot = glm::rotate(MathUtils::I4, -cameraAdjust, glm::vec3(1.0f, 0.0f, 0.0f));
	//	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	//	view = glm::lookAt(cameraPos, cameraTarget, up);
	//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//}

	//void CameraDownOnClick() {
	//	glm::mat4 rot = glm::rotate(MathUtils::I4, cameraAdjust, glm::vec3(1.0f, 0.0f, 0.0f));
	//	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	//	view = glm::lookAt(cameraPos, cameraTarget, up);
	//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//}

	//void CameraLeftOnClick() {
	//	glm::mat4 rot = glm::rotate(MathUtils::I4, -cameraAdjust, glm::vec3(0.0f, 1.0f, 0.0f));
	//	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	//	view = glm::lookAt(cameraPos, cameraTarget, up);
	//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//}

	//void CameraRightOnClick() {
	//	glm::mat4 rot = glm::rotate(MathUtils::I4, cameraAdjust, glm::vec3(0.0f, 1.0f, 0.0f));
	//	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	//	view = glm::lookAt(cameraPos, cameraTarget, up);
	//	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//}

	void handleScroll(glm::vec3 &update) {
		cameraPos += update;
		cameraTarget += update;
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void handleMouseDrag(float xoffset, float yoffset) {
		if (cameraState == pan) {
			glm::vec3 update = glm::vec3(-xoffset, -yoffset, 0.0f);
			cameraPos += update;
			cameraTarget += update;
		}
		else if (cameraState == tilt) {
			glm::mat4 xRotate = glm::rotate(MathUtils::I4, yoffset, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 yRotate = glm::rotate(MathUtils::I4, xoffset, glm::vec3(0.0f, 1.0f, 0.0f));

			cameraPos = glm::vec3(yRotate * xRotate * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
			up = glm::vec3(yRotate * xRotate * glm::vec4(up.x, up.y, up.z, 1.0f));
		}
		else {
			//zoom:
			cameraPos += yoffset * cameraPos;
		}
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	//Keyboard callbacks:
	float orbitAmount = .01f;
	void orbitRight() {

		glm::mat4 yRotate = glm::rotate(MathUtils::I4, orbitAmount, glm::vec3(0.0f, 1.0f, 0.0f));

		cameraPos = glm::vec3(yRotate * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
		up = glm::vec3(yRotate * glm::vec4(up.x, up.y, up.z, 1.0f));
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void orbitUp() {
		glm::mat4 xRotate = glm::rotate(MathUtils::I4, -orbitAmount, glm::vec3(1.0f, 0.0f, 0.0f));

		cameraPos = glm::vec3(xRotate * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
		up = glm::vec3(xRotate * glm::vec4(up.x, up.y, up.z, 1.0f));
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void orbitDown() {
		glm::mat4 xRotate = glm::rotate(MathUtils::I4, orbitAmount, glm::vec3(1.0f, 0.0f, 0.0f));

		cameraPos = glm::vec3(xRotate * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
		up = glm::vec3(xRotate * glm::vec4(up.x, up.y, up.z, 1.0f));
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void orbitLeft() {
		glm::mat4 yRotate = glm::rotate(MathUtils::I4, -orbitAmount, glm::vec3(0.0f, 1.0f, 0.0f));

		cameraPos = glm::vec3(yRotate * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
		//up = glm::vec3(yRotate * glm::vec4(up.x, up.y, up.z, 1.0f));
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void lookDownXaxis() {
		cameraPos = glm::vec3(2.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void lookDownYaxis() {
		cameraPos = glm::vec3(.0f, 2.0f, 0.0f);
		up = glm::vec3(0.0f, 0.0f, 1.0f);
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void lookDownZaxis() {
		cameraPos = glm::vec3(0.0f, 0.0f, 2.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}

	void calcView() {
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}
};