#pragma once

#include "GLView.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GLButton : public GLView {


	glm::vec4 toggledColor = glm::vec4(1.0f, 1.0f, .0f, 1.0f);
	glm::vec4 defaultColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

public:

	GLButton(float width, float height) : GLView(width, height) {}

	void SetToggledColor() {
		this->mColor = toggledColor;
	}

	void UnsetToggledColor() {
		this->mColor = defaultColor;
	}

};
