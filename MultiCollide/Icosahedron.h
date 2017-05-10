#pragma once

#include "ShapeSeparatingAxis.h"
#include "ShapeUtils.h"
#include "ColorShapeShader.h"

class Icosahedron : public ShapeSeparatingAxis {

	std::vector<GLfloat> vertices;

	float initialScale = .25f;

	glm::vec4 pentagonColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 hexagonColor = glm::vec4(.9f, .9f, .9f, 1.0f);

public:

	Icosahedron() {
		//Actual Icosahedron:
		/*float t = (1 + sqrtf(5)) / 2;

		std::vector<glm::vec3> pts;
		pts.push_back(glm::vec3(-1.0f, t, 0.0f));
		pts.push_back(glm::vec3(1.0f, t, 0.0f));
		pts.push_back(glm::vec3(-1.0f, -t, 0.0f));
		pts.push_back(glm::vec3(1.0f, -t, 0.0f));

		pts.push_back(glm::vec3(0.0f, -1.0f, t));
		pts.push_back(glm::vec3(0.0f, 1.0f, t));
		pts.push_back(glm::vec3(0.0f, -1.0f, -t));
		pts.push_back(glm::vec3(0.0f, 1.0f, -t));

		pts.push_back(glm::vec3(t, 0.0f, -1.0f));
		pts.push_back(glm::vec3(t, 0.0f, 1.0f));
		pts.push_back(glm::vec3(-t, 0.0f, -1.0f));
		pts.push_back(glm::vec3(-t, 0.0f, 1.0f));

		int indices[60] = { 0, 11,  5,    0,  5,  1,    0,  1,  7,    0,  7, 10,    0, 10, 11,
		1,  5,  9,    5, 11,  4,   11, 10,  2,   10,  7,  6,    7,  1,  8,
		3,  9,  4,    3,  4,  2,    3,  2,  6,    3,  6,  8,    3,  8,  9,
		4,  9,  5,    2,  4, 11,    6,  2, 10,    8,  6,  7,    9,  8,  1 };

		for (int i = 0; i < 60; i+=3) {
		glm::vec3 p1, p2, p3, n;
		p1 = pts[indices[i]];
		p2 = pts[indices[i + 1]];
		p3 = pts[indices[i + 2]];
		n = ShapeUtils::getNormalOfTriangle(p1, p2, p3);
		ShapeUtils::addTriangleToVector(p1, p2, p3, n, vertices);
		}*/

		useCustomColors = true;

		//Truncated icosahedron:

		float a = 0.0f;
		float b = 1.0f;
		float c = 2.0f;
		float d = (1.0f + sqrtf(5.0f)) / 2.0f;
		float e = 3.0f * d;
		float f = 1.0f + 2.0f * d;
		float g = 2.0f + d;
		float h = 2.0f * d;

		corners.push_back(glm::vec3(+a, +b, +e));
		corners.push_back(glm::vec3(+a, +b, -e));
		corners.push_back(glm::vec3(+a, -b, +e));
		corners.push_back(glm::vec3(+a, -b, -e));

		corners.push_back(glm::vec3(+b, +e, +a));
		corners.push_back(glm::vec3(+b, -e, +a));
		corners.push_back(glm::vec3(-b, +e, +a));
		corners.push_back(glm::vec3(-b, -e, +a));

		corners.push_back(glm::vec3(+e, +a, +b));
		corners.push_back(glm::vec3(-e, +a, +b));
		corners.push_back(glm::vec3(+e, +a, -b));
		corners.push_back(glm::vec3(-e, +a, -b));

		corners.push_back(glm::vec3(+c, +f, +d));
		corners.push_back(glm::vec3(+c, +f, -d));
		corners.push_back(glm::vec3(+c, -f, +d));
		corners.push_back(glm::vec3(-c, +f, +d));
		corners.push_back(glm::vec3(+c, -f, -d));
		corners.push_back(glm::vec3(-c, +f, -d));
		corners.push_back(glm::vec3(-c, -f, +d));
		corners.push_back(glm::vec3(-c, -f, -d));

		corners.push_back(glm::vec3(+f, +d, +c));
		corners.push_back(glm::vec3(+f, -d, +c));
		corners.push_back(glm::vec3(-f, +d, +c));
		corners.push_back(glm::vec3(+f, +d, -c));
		corners.push_back(glm::vec3(-f, -d, +c));
		corners.push_back(glm::vec3(+f, -d, -c));
		corners.push_back(glm::vec3(-f, +d, -c));
		corners.push_back(glm::vec3(-f, -d, -c));

		corners.push_back(glm::vec3(+d, +c, +f));
		corners.push_back(glm::vec3(-d, +c, +f));
		corners.push_back(glm::vec3(+d, +c, -f));
		corners.push_back(glm::vec3(+d, -c, +f));
		corners.push_back(glm::vec3(-d, +c, -f));
		corners.push_back(glm::vec3(-d, -c, +f));
		corners.push_back(glm::vec3(+d, -c, -f));
		corners.push_back(glm::vec3(-d, -c, -f));

		corners.push_back(glm::vec3(+b, +g, +h));
		corners.push_back(glm::vec3(+b, +g, -h));
		corners.push_back(glm::vec3(+b, -g, +h));
		corners.push_back(glm::vec3(-b, +g, +h));
		corners.push_back(glm::vec3(+b, -g, -h));
		corners.push_back(glm::vec3(-b, +g, -h));
		corners.push_back(glm::vec3(-b, -g, +h));
		corners.push_back(glm::vec3(-b, -g, -h));

		corners.push_back(glm::vec3(+g, +h, +b));
		corners.push_back(glm::vec3(+g, -h, +b));
		corners.push_back(glm::vec3(-g, +h, +b));
		corners.push_back(glm::vec3(+g, +h, -b));
		corners.push_back(glm::vec3(-g, -h, +b));
		corners.push_back(glm::vec3(+g, -h, -b));
		corners.push_back(glm::vec3(-g, +h, -b));
		corners.push_back(glm::vec3(-g, -h, -b));

		corners.push_back(glm::vec3(+h, +b, +g));
		corners.push_back(glm::vec3(-h, +b, +g));
		corners.push_back(glm::vec3(+h, +b, -g));
		corners.push_back(glm::vec3(+h, -b, +g));
		corners.push_back(glm::vec3(-h, +b, -g));
		corners.push_back(glm::vec3(-h, -b, +g));
		corners.push_back(glm::vec3(+h, -b, -g));
		corners.push_back(glm::vec3(-h, -b, -g));

		addPentagon(corners[0], corners[28], corners[36], corners[39], corners[29]);
		addPentagon(corners[1], corners[32], corners[41], corners[37], corners[30]);
		addPentagon(corners[2], corners[33], corners[42], corners[38], corners[31]);
		addPentagon(corners[3], corners[34], corners[40], corners[43], corners[35]);
		addPentagon(corners[4], corners[12], corners[44], corners[47], corners[13]);
		addPentagon(corners[5], corners[16], corners[49], corners[45], corners[14]);
		addPentagon(corners[6], corners[17], corners[50], corners[46], corners[15]);
		addPentagon(corners[7], corners[18], corners[48], corners[51], corners[19]);
		addPentagon(corners[8], corners[20], corners[52], corners[55], corners[21]);
		addPentagon(corners[9], corners[24], corners[57], corners[53], corners[22]);
		addPentagon(corners[10], corners[25], corners[58], corners[54], corners[23]);
		addPentagon(corners[11], corners[26], corners[56], corners[59], corners[27]);

		addHexagon(corners[0], corners[2], corners[31], corners[55], corners[52], corners[28]);
		addHexagon(corners[0], corners[29], corners[53], corners[57], corners[33], corners[2]);
		addHexagon(corners[1], corners[3], corners[35], corners[59], corners[56], corners[32]);
		addHexagon(corners[1], corners[30], corners[54], corners[58], corners[34], corners[3]);
		addHexagon(corners[4], corners[6], corners[15], corners[39], corners[36], corners[12]);
		addHexagon(corners[4], corners[13], corners[37], corners[41], corners[17], corners[6]);
		addHexagon(corners[5], corners[7], corners[19], corners[43], corners[40], corners[16]);
		addHexagon(corners[5], corners[14], corners[38], corners[42], corners[18], corners[7]);
		addHexagon(corners[8], corners[10], corners[23], corners[47], corners[44], corners[20]);
		addHexagon(corners[8], corners[21], corners[45], corners[49], corners[25], corners[10]);
		addHexagon(corners[9], corners[11], corners[27], corners[51], corners[48], corners[24]);
		addHexagon(corners[9], corners[22], corners[46], corners[50], corners[26], corners[11]);
		addHexagon(corners[12], corners[36], corners[28], corners[52], corners[20], corners[44]);
		addHexagon(corners[13], corners[47], corners[23], corners[54], corners[30], corners[37]);
		addHexagon(corners[14], corners[45], corners[21], corners[55], corners[31], corners[38]);
		addHexagon(corners[15], corners[46], corners[22], corners[53], corners[29], corners[39]);
		addHexagon(corners[16], corners[40], corners[34], corners[58], corners[25], corners[49]);
		addHexagon(corners[17], corners[41], corners[32], corners[56], corners[26], corners[50]);
		addHexagon(corners[18], corners[42], corners[33], corners[57], corners[24], corners[48]);
		addHexagon(corners[19], corners[51], corners[27], corners[59], corners[35], corners[43]);

		scaling *= initialScale;
		boundingSphereRadius = initialScale * (sqrtf(e*e + 1.0f)) + BoundingSphereBuffer;

		ComputeInertia();
	}

	void addPentagon(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e) {
		glm::vec3 n = glm::normalize((a + b + c + d + e) / 5.0f);
		ShapeUtils::addTriangleToVectorWithColor(a, b, c, n, pentagonColor, vertices);
		ShapeUtils::addTriangleToVectorWithColor(a, c, d, n, pentagonColor, vertices);
		ShapeUtils::addTriangleToVectorWithColor(a, d, e, n, pentagonColor, vertices);

		normals.push_back(n);
	}

	void addColorToVec(std::vector<GLfloat> &vec, glm::vec4 &color) {
		vec.push_back(color.x);
		vec.push_back(color.y);
		vec.push_back(color.z);
		vec.push_back(color.w);
	}

	void addHexagon(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 e, glm::vec3 f) {
		glm::vec3 n = glm::normalize((a + b + c + d + e + f) / 6.0f);
		ShapeUtils::addTriangleToVectorWithColor(a, b, c, n, hexagonColor, vertices);
		ShapeUtils::addTriangleToVectorWithColor(a, c, d, n, hexagonColor, vertices);
		ShapeUtils::addTriangleToVectorWithColor(a, d, f, n, hexagonColor, vertices);
		ShapeUtils::addTriangleToVectorWithColor(d, e, f, n, hexagonColor, vertices);

		normals.push_back(n);
	}

	virtual void ComputeInertia() override {
		angularInertia = glm::mat3();
		//angularInertia[0][0] = angularInertia[1][0] = angularInertia[1][1] = angularInertia[2][0] = angularInertia[2][1] = angularInertia[2][2] = .0671673f;
		angularInertia[0][0] = angularInertia[1][1] = angularInertia[2][2] = .0671673f;
	}

	virtual void InitVAOandVBO(Shader &shader) override {

		//shader.Use();
		ColorShapeShader::getInstance().shader.Use();
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(ColorShapeShader::getInstance().shader.getAttribute("position"), 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(ColorShapeShader::getInstance().shader.getAttribute("position"));

		// Normal
		glVertexAttribPointer(ColorShapeShader::getInstance().shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(ColorShapeShader::getInstance().shader.getAttribute("normal"));
		//glBindVertexArray(0); // Unbind VAO

		// Color
		glVertexAttribPointer(ColorShapeShader::getInstance().shader.getAttribute("color"), 4, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(ColorShapeShader::getInstance().shader.getAttribute("color"));
		glBindVertexArray(0); // Unbind VAO
	}

	virtual void Draw(Shader &shader) override {

		glUniformMatrix4fv(ColorShapeShader::getInstance().shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		//glUniform4f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z, 1.0f);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 10);
		glBindVertexArray(0);

	}

	virtual ~Icosahedron() override {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "10" << std::endl;
		return os.str();
	}

};

class Tetra : public ShapeSeparatingAxis {

	std::vector<GLfloat> vertices;

	float initialScale = .5f;

public:

	float t = 1.0f / sqrtf(2.0f);
	//The point facing right
	glm::vec3 c = glm::vec3(0.0f, 1.0f, t);

	Tetra() {

		glm::vec3 a(-1.0f, 0.0f, -t);
		glm::vec3 b(1.0f, 0.0f, -t);

		glm::vec3 d(0.0f, -1.0f, t);

		corners.push_back(a);
		corners.push_back(b);
		corners.push_back(c);
		corners.push_back(d);

		glm::vec3 n1 = ShapeUtils::getNormalOfTriangle(a, c, b);
		glm::vec3 n2 = ShapeUtils::getNormalOfTriangle(a, b, d);
		glm::vec3 n3 = ShapeUtils::getNormalOfTriangle(a, d, c);
		glm::vec3 n4 = ShapeUtils::getNormalOfTriangle(b, c, d);

		normals.push_back(n1);
		normals.push_back(n2);
		normals.push_back(n3);
		normals.push_back(n4);

		ShapeUtils::addTriangleToVector(a, c, b, n1, vertices);
		ShapeUtils::addTriangleToVector(a, b, d, n2, vertices);
		ShapeUtils::addTriangleToVector(a, d, c, n3, vertices);
		ShapeUtils::addTriangleToVector(b, c, d, n4, vertices);

		scaling *= initialScale;

		boundingSphereRadius = initialScale * sqrtf(t*t + 1) + BoundingSphereBuffer;

		//To point to right:
		this->applyRotation(glm::vec3(1.0f, 0.0f, 0.0f), glm::pi<float>() / 2.0f);
		this->applyRotation(glm::vec3(0.0f, 0.0f, 1.0f), -glm::pi<float>() / 8.0f);

		ComputeInertia();
	}

	virtual void ComputeInertia() override {
		angularInertia = glm::mat3();
		angularInertia[0][0] = .027135f;
		angularInertia[1][1] = .017239f;
		angularInertia[2][2] = .022813f;
	}

	virtual void InitVAOandVBO(Shader &shader) override {

		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

		// Position attribute
		glVertexAttribPointer(shader.getAttribute("position"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal
		glVertexAttribPointer(shader.getAttribute("normal"), 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0); // Unbind VAO
	}

	virtual void Draw(Shader &shader) override {

		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glUniform4f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z, 1.0f);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

	virtual ~Tetra() override {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "11" << std::endl;
		return os.str();
	}
};