#pragma once


#include "GLView.h"

class GLLabel : public GLView {

public:

	GLLabel(float width, float height) : GLView(width, height)  {
		mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

};