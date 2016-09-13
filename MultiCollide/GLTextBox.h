#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "GLView.h"
#include "Keyboard.h"

class GLTextBox : public GLView, public Keyboard::Listener {

	bool allHighlighted = false;

public:

	class TextChangedListener {
	public:
		virtual void OnChange() = 0;
	};

private:
	TextChangedListener *textChangedListener;


public:

	GLTextBox(float width, float height) : GLView(width, height) {
		mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		
	}

	void SetTextChangedListener(TextChangedListener *listener) {
		this->textChangedListener = listener;
	}

	virtual void OnClick(glm::vec3 clickPt, GLView *focus) override {
		mTextBackground = true;
		allHighlighted = true;
		std::cout << "Textbox clicked" << std::endl;
		GLView::OnClick(clickPt, focus);

		Keyboard::AddListener(this);
	}


	virtual void KeyPressed(char c) override {
		if (allHighlighted) {
			this->mText.clear();
			this->mText = c;
			allHighlighted = false;
			mTextBackground = false;
		}
		else {
			this->mText += c;
		}

		if (textChangedListener != nullptr && (this->mText.length() > 2 || (this->mText.length() > 1 && this->mText[1] != '.' && this->mText[0] != '-')  || (this->mText[0] != '.' && this->mText[0] != '-'))) {
			textChangedListener->OnChange();
		}
	}

	virtual void GiveFocus(GLView *focus) override {
		focus = this;
	}

	virtual void LoseFocus() override {
		Keyboard::RemoveListener(this);
		allHighlighted = false;
		mTextBackground = false;
	}

};