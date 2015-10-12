//#include <iostream>
//#include <fstream>
//#include <vector>
//
//// GLEW
//#define GLEW_STATIC
//#include <GL/glew.h>
//
//// GLFW
//#include <GLFW/glfw3.h>
//
//// Other Libs
//#include <SOIL.h>
//// GLM Mathematics
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtx/quaternion.hpp>
//#include <glm/gtc/quaternion.hpp>
//#include <glm/gtc/constants.hpp>
//
////EIGEN - SVD 
//#include <Eigen/Dense>
//#include <Eigen\SVD>
//
//// Other includes
//#include "Shader.h"
//#include "TetrahedralMesh.h"
//
//// Function prototypes
//void addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec);
//glm::vec3 getNormalOfTriangle(glm::vec3 &side1, glm::vec3 &side2, glm::vec3 &p3);
//void convertGlmMatToEigen(glm::mat3 &glmMat, Eigen::MatrixXd &eigenMatOut);
//bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c);
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
//
//// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
//
//GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
//GLfloat lastFrame = 0.0f;      // Time of last frame
//
////Matrices:
//glm::mat4 model;
//glm::mat4 view = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
//glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
//
//using Eigen::MatrixXd;
//
//int main()
//{
//
//#pragma region OpenGLinit
//	// Init GLFW
//	glfwInit();
//	// Set all the required options for GLFW
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
//
//	// Create a GLFWwindow object that we can use for GLFW's functions
//	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Pencil Motion", nullptr, nullptr);
//	glfwMakeContextCurrent(window);
//
//	glfwSetKeyCallback(window, key_callback);
//
//	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
//	glewExperimental = GL_TRUE;
//	// Initialize GLEW to setup the OpenGL Function pointers
//	glewInit();
//
//	// Define the viewport dimensions
//	glViewport(0, 0, WIDTH, HEIGHT);
//
//	// Setup OpenGL options
//	glEnable(GL_DEPTH_TEST);
//
//	// Build and compile our shader program
//	Shader bareShader("vertexShaderBare.glsl", "fragmentShaderBare.glsl");
//
//#pragma endregion
//
//#pragma region Vertices Setup
//
//
//	//Helix:
//	//std::vector<GLfloat> verts;
//	//GLfloat a = .5f;
//	//GLfloat b = .05f;
//	//GLfloat tMax = glm::pi<float>() * 8.0;
//	//GLfloat t;
//	//GLfloat tInc = glm::pi<float>() / 32.0f;
//	//for (t = 4.0 * tMax / -4.0f; t < tMax; t += tInc){
//	//	verts.push_back(a*cos(t));
//	//	verts.push_back(b*t);
//	//	verts.push_back(a*sin(t));
//	//	verts.push_back(0.0f); //normal for now
//	//	verts.push_back(0.0f); //normal for now
//	//	verts.push_back(0.0f); //normal for now
//	//}
//
//	const GLfloat TWO_PI = glm::pi<float>() * 2.0f;
//	//Cylinder
//	std::vector<GLfloat> verts;
//	GLfloat radius = 1.0f;
//	GLfloat diffBetweenCircles = .01f;
//
//	GLfloat t;
//	GLfloat radInc = glm::pi<float>() / 32.0f;
//	int numCircles = 100;
//	//numCircles * diffBetweenCircles = HEIGHT
//
//	std::vector<GLfloat> circleXpts;
//	std::vector<GLfloat> circleZpts;
//	std::vector<GLfloat> circleVerts;
//	int circlePts = 0;
//	for (GLfloat rad = 0; rad <= TWO_PI; rad += radInc){
//		circleXpts.push_back(radius*cos(rad));
//		circleZpts.push_back(radius*sin(rad));
//		circlePts++;
//		circleVerts.push_back(radius*cos(rad));
//		circleVerts.push_back(radius*sin(rad));
//		circleVerts.push_back(0.0f);
//		circleVerts.push_back(0.0f);
//		circleVerts.push_back(0.0f);
//		circleVerts.push_back(0.0f);
//	}
//
//	bool top = false;
//	for (int h = 0; h < numCircles; h++){
//		for (int circ = 0; circ < circlePts - 1; circ++){
//			GLfloat x1 = circleXpts[circ];
//			GLfloat y1 = diffBetweenCircles * h;
//			GLfloat z1 = circleXpts[circ];
//
//			GLfloat x2 = x1;
//			GLfloat y2 = y1 + diffBetweenCircles;
//			GLfloat z2 = z1;
//
//			GLfloat x3 = circleXpts[circ + 1];
//			GLfloat y3 = y1;
//			GLfloat z3 = circleXpts[circ + 1];
//
//			GLfloat x4 = x3;
//			GLfloat y4 = y2;
//			GLfloat z4 = z3;
//
//			glm::vec3 p1(x1, y1, z1);
//			glm::vec3 p2(x2, y2, z2);
//			glm::vec3 p3(x3, y3, z3);
//			glm::vec3 p4(x4, y4, z4);
//
//			// Triangles look like:
//			///  p2 -- p4
//			///  |  \  |
//			///  p1 -- p3
//
//			glm::vec3 normal1 = getNormalOfTriangle(p1, p2, p3);
//			glm::vec3 normal2 = getNormalOfTriangle(p3, p2, p4);
//
//			addTriangleToVector(p1, p2, p3, normal1, verts);
//			addTriangleToVector(p3, p2, p4, normal2, verts);
//		}
//	}
//
//#pragma endregion
//
//#pragma region VAO/VBOSetup
//	//Tetrahedron vertices array setup
//	GLuint VBO, VAO;
//	glGenVertexArrays(1, &VAO);
//	glGenBuffers(1, &VBO);
//	glBindVertexArray(VAO);
//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
//	glBufferData(GL_ARRAY_BUFFER, circleVerts.size() * sizeof(GLfloat), &circleVerts[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//	glBindVertexArray(0); // Unbind VAO
//
//#pragma endregion 
//
//	GLfloat phi = glm::pi<float>() / 4096.0f;
//	GLfloat rot = 1.0f;
//	GLfloat rotation;
//	bool up = true;
//
//	// Game loop
//	while (!glfwWindowShouldClose(window))
//	{
//		GLfloat currentFrame = glfwGetTime();
//		deltaTime = currentFrame - lastFrame;
//		lastFrame = currentFrame;
//
//		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
//		glfwPollEvents();
//
//		// Render
//		// Clear the colorbuffer
//		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//		if (rot >= 180.0f){
//			up = !up;
//		}
//		if (up)
//			rot += 0.01f;
//		else
//			rot -= 0.01f;
//
//		rotation = glm::radians(rot);
//
//		//Draw tetrahedron
//		glm::mat4 model;
//		//model = glm::translate(model, glm::vec3(0.0, 0.0, -3.0));
//		model = glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, 0.0f));
//		//model = glm::scale(model, glm::vec3(.2, .2, .2));
//
//		glm::mat4 allModel;
//		allModel = glm::rotate(allModel, rotation, glm::vec3(.33f, .33f, .33f));
//
//		model = allModel * model;
//
//		bareShader.Use();
//		glm::vec3 lightPos(0.0f, 0.0f, -1.0f);
//		glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
//		GLint modelLoc = glGetUniformLocation(bareShader.Program, "model");
//		GLint viewLoc = glGetUniformLocation(bareShader.Program, "view");
//		GLint projLoc = glGetUniformLocation(bareShader.Program, "projection");
//		GLint lightPosLoc = glGetUniformLocation(bareShader.Program, "lightPos");
//		GLint objectColorLoc = glGetUniformLocation(bareShader.Program, "objectColor");
//		glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
//		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//		glBindVertexArray(VAO);
//		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
//		/*glDrawArrays(GL_TRIANGLES, 0, numTris * 3);*/
//
//		//Draw Principal Frame:
//		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//		//glBindVertexArray(frameVAO);
//
//		//FOr Helix:
//		glDrawArrays(GL_LINE_STRIP, 0, circleVerts.size() / 6);
//
//		glm::mat4 model2;
//		model2 = glm::rotate(model2, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
//		model2 = allModel * model2;
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
//		glDrawArrays(GL_LINE_STRIP, 0, circleVerts.size() / 6);
//
//		glm::mat4 model3;
//		model3 = glm::rotate(model3, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
//		model3 = allModel * model3;
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
//		glDrawArrays(GL_LINE_STRIP, 0, circleVerts.size() / 6);
//
//		//For cylinder:
//		//glDrawArrays(GL_TRIANGLES, 0, verts.size() / 2);
//
//		glBindVertexArray(0);
//
//		// Swap the screen buffers
//		glfwSwapBuffers(window);
//	}
//
//	// Properly de-allocate all resources once they've outlived their purpose
//	glDeleteVertexArrays(1, &VAO);
//	glDeleteBuffers(1, &VBO);
//
//	// Terminate GLFW, clearing any resources allocated by GLFW.
//	glfwTerminate();
//
//	return 0;
//}
//
//void addTriangleToVector(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 normal, std::vector<GLfloat> &vec){
//	vec.push_back(p1.x);
//	vec.push_back(p1.y);
//	vec.push_back(p1.z);
//	vec.push_back(normal.x);
//	vec.push_back(normal.y);
//	vec.push_back(normal.z);
//
//	vec.push_back(p2.x);
//	vec.push_back(p2.y);
//	vec.push_back(p2.z);
//	vec.push_back(normal.x);
//	vec.push_back(normal.y);
//	vec.push_back(normal.z);
//
//	vec.push_back(p3.x);
//	vec.push_back(p3.y);
//	vec.push_back(p3.z);
//	vec.push_back(normal.x);
//	vec.push_back(normal.y);
//	vec.push_back(normal.z);
//}
//
//glm::vec3 getNormalOfTriangle(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3){
//	glm::vec3 normal;
//	glm::vec3 V = p2 - p1;
//	glm::vec3 W = p3 - p1;
//	normal.x = (V.y * W.z) - (V.z * W.y);
//	normal.y = (V.z * W.z) - (V.x * W.z);
//	normal.z = (V.x * W.y) - (V.y * W.x);
//	return normal;
//}
//
//void convertGlmMatToEigen(glm::mat3 &glmMat, Eigen::MatrixXd &eigenMatOut){
//	//eigenMatOut(row, col)
//	eigenMatOut(0, 0) = glmMat[0][0];
//	eigenMatOut(0, 1) = glmMat[0][1];
//	eigenMatOut(0, 2) = glmMat[0][2];
//	eigenMatOut(1, 0) = glmMat[1][0];
//	eigenMatOut(1, 1) = glmMat[1][1];
//	eigenMatOut(1, 2) = glmMat[1][2];
//	eigenMatOut(2, 0) = glmMat[2][0];
//	eigenMatOut(2, 1) = glmMat[2][1];
//	eigenMatOut(2, 2) = glmMat[2][2];
//}
//
//bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c){
//	float dotab = glm::dot(a, b);
//	float dotac = glm::dot(a, c);
//	float dotbc = glm::dot(b, c);
//	return true;
//}
//
//// Is called whenever a key is pressed/released via GLFW
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//}