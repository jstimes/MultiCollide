#pragma once

#include "GLView.h"
#include "ShapeUtils.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Triangle {
	glm::vec3 a, b, c, n;
};

struct Primitive {
	std::vector<GLfloat> vertices;

	GLuint VAO, VBO;

	void AddTriangle(Triangle t) {
		ShapeUtils::addTriangleToVector(t.a, t.b, t.c, t.n, vertices);
	}

	void InitVaoVbo() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	void Draw(Shader &shader, glm::mat4 model, glm::vec4 objectColor, glm::vec3 lightPos) {

		shader.Use();
		glBindVertexArray(VAO);
		glUniform3f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z);
		glUniformMatrix4fv(shader.getUniform("model"), 1, false, glm::value_ptr(model));
		glUniform3f(shader.getUniform("lightPos"), lightPos.x, lightPos.y, lightPos.z);

		glUniformMatrix4fv(shader.getUniform("view"), 1, false, glm::value_ptr(glm::mat4()));
		glUniformMatrix4fv(shader.getUniform("projection"), 1, false, glm::value_ptr(glm::mat4()));
		glUniformMatrix4fv(shader.getUniform("sceneTransform"), 1, false, glm::value_ptr(glm::mat4()));

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

};

class GLRoundedButton : public GLView {


	glm::vec4 toggledColor = glm::vec4(1.0f, 1.0f, .0f, 1.0f);
	glm::vec4 defaultColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	float cornerRadius;
	int numCornerTriStrips;

	glm::vec3 translation;
	glm::mat4 model;

	Primitive cornerUL;
	Primitive cornerLL;
	Primitive cornerUR;
	Primitive cornerLR;
	Primitive verticalMiddle;
	Primitive horizontalMiddle;

	std::vector<Primitive> primitives;

public:

	GLRoundedButton(float width, float height, float cornerRadius = .05, int numCornerTriStrips = 15) : GLView(width, height) {
		
		this->cornerRadius = cornerRadius; 
		this->numCornerTriStrips = numCornerTriStrips;

		//TODO fix to idk
		mShader = Shader("vertexShader.glsl", "fragmentShader.glsl");
	}


	void SetToggledColor() {
		this->mColor = toggledColor;
	}

	void UnsetToggledColor() {
		this->mColor = defaultColor;
	}


private:

	virtual void InitBuffers() override {
		
		//First init corners:
		float halfWidth = mWidth / 2.0f;
		float halfHeight = mHeight / 2.0f;

		float theta = 0.0f;
		float end = glm::pi<float>() / 2.0f;
		float offset = end / numCornerTriStrips;

		glm::vec3 basePtUL = glm::vec3(-halfWidth + cornerRadius, halfHeight - cornerRadius, 0.0f);
		glm::vec3 basePtLL = glm::vec3(-halfWidth + cornerRadius, -halfHeight + cornerRadius, 0.0f);
		glm::vec3 basePtUR = glm::vec3(halfWidth - cornerRadius, halfHeight - cornerRadius, 0.0f);
		glm::vec3 basePtLR = glm::vec3(halfWidth - cornerRadius, -halfHeight + cornerRadius, 0.0f);

		glm::vec3 norm(0.0f, 0.0f, 1.0f);
		glm::vec3 rotAxis(0.0f, 0.0f, 0.0f);

		glm::vec3 ulVecPrev = cornerRadius * glm::vec3(-1.0, 0.0f, 0.0f);
		glm::vec3 llVecPrev = cornerRadius * glm::vec3(-1.0, 0.0f, 0.0f);
		glm::vec3 urVecPrev = cornerRadius * glm::vec3(1.0, 0.0f, 0.0f);
		glm::vec3 lrVecPrev = cornerRadius * glm::vec3(1.0, 0.0f, 0.0f);

		for (theta = offset; theta <= end; theta += offset) {
			glm::vec3 vec(cos(theta), sin(theta), 0.0f);


			glm::vec3 ulVec = cornerRadius * glm::vec3(-vec.x, vec.y, 0.0f);
			cornerUL.AddTriangle(GetCornerTriangle(basePtUL, ulVec, ulVecPrev));
			ulVecPrev = ulVec;

			glm::vec3 llVec = cornerRadius * glm::vec3(-vec.x, -vec.y, 0.0f);
			cornerLL.AddTriangle(GetCornerTriangle(basePtLL, llVec, llVecPrev));
			llVecPrev = llVec;

			glm::vec3 urVec = cornerRadius * glm::vec3(vec.x, vec.y, 0.0f);
			cornerUR.AddTriangle(GetCornerTriangle(basePtUR, urVec, urVecPrev));
			urVecPrev = urVec;

			glm::vec3 lrVec = cornerRadius * glm::vec3(vec.x, -vec.y, 0.0f);
			cornerLR.AddTriangle(GetCornerTriangle(basePtLR, lrVec, lrVecPrev));
			lrVecPrev = lrVec;
		}

		//Then inner parts of button:
		Triangle vm1, vm2;
		vm1.a = glm::vec3(-halfWidth, halfHeight - cornerRadius, 0.0f);
		vm1.b = glm::vec3(-halfWidth, -halfHeight + cornerRadius, 0.0f);
		vm1.c = glm::vec3(halfWidth, -halfHeight + cornerRadius, 0.0f);
		vm1.n = glm::vec3(0.0f, 0.0f, 1.0f);

		vm2.a = glm::vec3(-halfWidth, halfHeight - cornerRadius, 0.0f);
		vm2.b = glm::vec3(halfWidth, halfHeight - cornerRadius, 0.0f);
		vm2.c = glm::vec3(halfWidth, -halfHeight + cornerRadius, 0.0f);
		vm2.n = glm::vec3(0.0f, 0.0f, 1.0f);

		verticalMiddle.AddTriangle(vm1);
		verticalMiddle.AddTriangle(vm2);

		Triangle hm1, hm2;
		hm1.a = glm::vec3(-halfWidth + cornerRadius, halfHeight, 0.0f);
		hm1.b = glm::vec3(-halfWidth + cornerRadius, -halfHeight, 0.0f);
		hm1.c = glm::vec3(halfWidth - cornerRadius, -halfHeight, 0.0f);
		hm1.n = glm::vec3(0.0f, 0.0f, 1.0f);

		hm2.a = glm::vec3(-halfWidth + cornerRadius, halfHeight, 0.0f);
		hm2.b = glm::vec3(halfWidth - cornerRadius, halfHeight, 0.0f);
		hm2.c = glm::vec3(halfWidth - cornerRadius, -halfHeight, 0.0f);
		hm2.n = glm::vec3(0.0f, 0.0f, 1.0f);

		horizontalMiddle.AddTriangle(hm1);
		horizontalMiddle.AddTriangle(hm2);

		primitives.push_back(cornerUL);
		primitives.push_back(cornerLL);
		primitives.push_back(cornerUR);
		primitives.push_back(cornerLR);
		primitives.push_back(verticalMiddle);
		primitives.push_back(horizontalMiddle);

		for (unsigned int i = 0; i < primitives.size(); i++) {
			primitives[i].InitVaoVbo();
		}
		
	}

	Triangle GetCornerTriangle(glm::vec3 base, glm::vec3 update1, glm::vec3 update2) {
		Triangle t;
		t.a = base;
		t.b = base + update1;
		t.c = base + update2;

		//norm is average of (0, 0, 1) and (update1 + update2)/2 (middle of far edge of triangle)
		t.n = .5f * (glm::vec3(0.0f, 0.0f, 1.0f) + (.5f * (update1 + update2)));
		
		return t;
	}

	virtual void Translate(glm::vec3 translation) override {
		//add offset since this view has center at origin, not bottom left corner
		this->translation = getTranslationOffset() + translation;
		this->mPosition = this->translation - getTranslationOffset();
	}

	glm::vec3 getTranslationOffset() {
		return glm::vec3(mWidth / 2.0f, mHeight / 2.0f, 0.0f);
	}

	virtual void Draw() override {

		if (this->mText.length() > 0) {
			float x = ((this->mPosition.x + 1.0f) / 2.0f + .01f/*+ this->mWidth / 4.0f - .01f*/) * 800.0f;  //TODO fix
			float y = ((this->mPosition.y + 1.0f) / 2.0f + this->mHeight / 4.0f - .01f) * 600.0f;
			 
			Text::Render(this->mText, x, y, .35f, this->mTextColor, this->mTextBackground);
		}

		this->model = glm::translate(glm::mat4(), this->translation);

		for (unsigned int i = 0; i < primitives.size(); i++) {
			Primitive p = primitives[i];
			p.Draw(this->mShader, this->model, this->mColor, this->translation);
		}

		/*Triangle t;
		Primitive p;

		t.a = glm::vec3(.5f, 0.0f, .05f);
		t.b = glm::vec3(-.5f, 0.0f, .05f);
		t.c = glm::vec3(0.0f, .5f, .05f);
		t.a = glm::vec3(0.0f, 0.0f, 1.0f);
		
		p.AddTriangle(t);*/
	}

	//Onclick..return false if not on actual button

};
