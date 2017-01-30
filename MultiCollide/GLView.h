#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include "Shader.h"
#include "Text.h"

//bool debug = false;

class GLView {

protected:

	float mHeight;
	float mWidth;

	std::string mText;

	// (xpos, ypos) is lower left corner of view
	glm::vec3 mPosition;

	glm::vec4 mColor;

	glm::vec3 mTextColor = glm::vec3(0.0f, 0.0f, 1.0f);

	bool mTextBackground = false;

	void(*mClickListener)();

	//TODO margin & padding, styling

	const GLchar* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 position;\n"
		"void main()\n"
		"{\n"
		"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
		"}\0";

	const GLchar* fragmentShaderSource = "#version 330 core\n"
		"out vec4 color;\n"
		"uniform vec4 objectColor;\n"
		"void main()\n"
		"{\n"
		"color = objectColor;\n"
		"}\n\0";

	Shader mShader = Shader::ShaderFromCode(this->vertexShaderSource, this->fragmentShaderSource);

	GLint mObjectColorLoc = glGetUniformLocation(mShader.Program, "objectColor");

	GLuint VAO, VBO;

	std::vector<GLfloat> vertices;


public:

	GLView(float width, float height) {
		this->mWidth = width;
		this->mHeight = height;

		this->mPosition = glm::vec3();

		this->mColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		//if(debug)
		//	std::cout << "View created" << std::endl;
	}

	std::string GetText() {
		return this->mText;
	}

	void SetText(std::string text) {
		this->mText = text;
	}

	void SetTextColor(glm::vec3 color) {
		this->mTextColor = color;
	}

	void SetWidth(float width) {
		this->mWidth = width;
	}

	void SetHeight(float height) {
		this->mHeight = height;
	}

	void SetColor(glm::vec4 color) {
		this->mColor = color;
	}

	float GetWidth() {
		return this->mWidth;
	}

	float GetHeight() {
		return this->mHeight;
	}

	glm::vec3 GetPosition() {
		return this->mPosition;
	}

	//Sets views position to this point
	virtual void Translate(glm::vec3 translation) {
		this->mPosition = translation;
	}

	//Sets views position to this point
	virtual void UpdateTranslate(glm::vec3 translation) {
		this->mPosition += translation;
	}

	//Assumes clickPt is in NDC, i.e. x,y,z are in interval (-1.0f, 1.0f)
	virtual bool CheckClicked(glm::vec3 clickPt) {
		return (clickPt.x >= this->mPosition.x && clickPt.x <= this->mPosition.x + this->mWidth)
				&& (clickPt.y >= this->mPosition.y && clickPt.y <= this->mPosition.y + this->mHeight);
	}

	void SetClickListener(void(*listener)()) {
		this->mClickListener = listener;
	}

	virtual void OnClick(glm::vec3 clickPt, GLView *focus) {
		if (this->mClickListener != nullptr) {
			this->mClickListener();
		}
	}

	virtual void GiveFocus(GLView *focus) {

	}

	virtual void LoseFocus() {

	}

	//Must be called before drawing can begin
	virtual void InitBuffers() {
		glm::vec3 lowerLeft(this->mPosition.x, this->mPosition.y, this->mPosition.z);
		glm::vec3 lowerRight(this->mPosition.x + this->mWidth, this->mPosition.y, this->mPosition.z);
		glm::vec3 upperLeft(this->mPosition.x, this->mPosition.y + this->mHeight, this->mPosition.z);
		glm::vec3 upperRight(this->mPosition.x + this->mWidth, this->mPosition.y + this->mHeight, this->mPosition.z);

		vertices.push_back(upperLeft.x);
		vertices.push_back(upperLeft.y);
		vertices.push_back(upperLeft.z);

		vertices.push_back(lowerLeft.x);
		vertices.push_back(lowerLeft.y);
		vertices.push_back(lowerLeft.z);

		vertices.push_back(upperRight.x);
		vertices.push_back(upperRight.y);
		vertices.push_back(upperRight.z);

		vertices.push_back(upperRight.x);
		vertices.push_back(upperRight.y);
		vertices.push_back(upperRight.z);

		vertices.push_back(lowerLeft.x);
		vertices.push_back(lowerLeft.y);
		vertices.push_back(lowerLeft.z);

		vertices.push_back(lowerRight.x);
		vertices.push_back(lowerRight.y);
		vertices.push_back(lowerRight.z);

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
	}

	//InitBuffers() must be called before this method is
	virtual void Draw() {

		if (this->mText.length() > 0) {
			float x = ((this->mPosition.x + 1.0f) / 2.0f + .01f/*+ this->mWidth / 4.0f - .01f*/) * 800.0f ;  //TODO fix
			float y = ((this->mPosition.y + 1.0f) / 2.0f + this->mHeight / 4.0f - .01f) * 600.0f ;

			//std::cout << "Pos: " << x << ", " << y << std::endl;
			
			Text::Render(this->mText, x, y, .35f, this->mTextColor, this->mTextBackground);
		}

		this->mShader.Use();
		glBindVertexArray(VAO);
		glUniform4f(this->mObjectColorLoc, this->mColor.x, this->mColor.y, this->mColor.z, this->mColor.w);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
		glBindVertexArray(0);

		
		
	}

	virtual void DeleteBuffers() {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	virtual ~GLView() {

		DeleteBuffers();

		//if(debug)
		//	std::cout << "View destroyed" << std::endl;
	}
};