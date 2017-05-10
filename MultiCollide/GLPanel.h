#pragma once

#include "GLView.h"

class GLPanel : public GLView {

public:
	enum Orientation { vertical = 1, horizontal = 0 };
	
private:
	std::vector<GLView*> mSubviews;
	Orientation mOrientation;
	float mPaddingX;
	float mPaddingY;
	float mSpacing;

	//For making subviews lay on top of the panel:
	const float zOffset = .1f;

	bool hidden;

public:

	GLPanel(float width, float height, Orientation orientation, float xPadding, float yPadding, float viewSpacing) : GLView(width, height) {
		this->mOrientation = orientation;
		this->mPaddingX = xPadding;
		this->mPaddingY = yPadding;
		this->mSpacing = viewSpacing;

		hidden = false;

		//So subviews are on top of the panel:
		//this->mPosition.z -= zOffset;
	}

	void AddView(GLView *view) {
		glm::vec3 lowerLeft = glm::vec3();

		if (this->mSubviews.size() == 0) {
			glm::vec3 upperLeft = this->mPosition + glm::vec3(0.0f, this->mHeight, 0.0f) + glm::vec3(this->mPaddingX, -this->mPaddingY, 0.0f);
			lowerLeft = upperLeft - glm::vec3(0.0f, view->GetHeight(), 0.0f);
		}
		else {
			GLView *prevView = this->mSubviews[this->mSubviews.size() - 1];
			
			if (this->mOrientation == vertical) {
				glm::vec3 spacingUpdate = glm::vec3(0.0f, -this->mSpacing, 0.0f);
				glm::vec3 upperLeft = prevView->GetPosition() + spacingUpdate;
				lowerLeft = upperLeft - glm::vec3(0.0f, view->GetHeight(), 0.0f);

			}
			else {
				 float x = prevView->GetPosition().x + prevView->GetWidth() + this->mSpacing;
				 float y = this->mPosition.y + this->mHeight - this->mPaddingY - view->GetHeight();
				 lowerLeft = glm::vec3(x, y, 0.0f);
			}
		}
		
		//lowerLeft.z = zOffset; //0.0f;
		view->Translate(lowerLeft);
		this->mSubviews.push_back(view);
	}

	virtual void HideContent() {
		hidden = true;
	}

	virtual void ShowContent() {
		hidden = false;
	}

	virtual void OnClick(glm::vec3 clickPt, GLView *focus) override {
		//In case there is a general onClick callback for whole panel:
		GLView::OnClick(clickPt, focus); 

		if (!hidden) {
			//Check if subviews were clicked:
			for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
				if ((*it)->CheckClicked(clickPt)) {
					(*it)->OnClick(clickPt, focus);
					return; //Only one view should be clicked
				}
			}
		}
	}

	virtual void Draw() override {

		if (!hidden) {
			//Draw all subviews:
			for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
				(*it)->Draw();
			}
		}

		//Draw the panel:
		GLView::Draw();
	}

	virtual void Translate(glm::vec3 translation) override {
		GLView::Translate(translation);

		for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
			(*it)->Translate(translation);
		}
	}

	virtual void UpdateTranslate(glm::vec3 translation) override {
		GLView::UpdateTranslate(translation);

		for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
			(*it)->UpdateTranslate(translation);
		}
	}

	virtual void InitBuffers() override {
		//Init the panel's buffer:
		GLView::InitBuffers();

		//Then init sub view buffers:
		for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
			(*it)->InitBuffers();
		}
	}

	virtual void DeleteBuffers() override {
		GLView::DeleteBuffers();

		for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
			(*it)->DeleteBuffers();
		}
	}

	virtual ~GLPanel() override {
		//Panel is responsible for cleaning up all subviews:
		for (std::vector<GLView*>::iterator it = this->mSubviews.begin(); it != this->mSubviews.end(); it++) {
			delete (*it);
		}
	}

};