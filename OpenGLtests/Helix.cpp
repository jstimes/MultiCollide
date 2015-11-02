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
//#include "Cylinder.h"
//#include "Cone.h"
//#include "Helix.h"
//#include "TetrahedralMesh.h"
//
//// Function prototypes
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
//	Helix helix(.3f, .1f, 1.0f);
//	std::vector<GLfloat> helixVerts = helix.vertices;
//	Cylinder cyl(.2f, 1.0f);
//	std::vector<GLfloat> verts = cyl.vertices;
//	Cone cone(.2f, .3f, false);
//	std::vector<GLfloat> coneVerts = cone.vertices;
//
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
//	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), &verts[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
//	GLuint coneVBO, coneVAO;
//	glGenVertexArrays(1, &coneVAO);
//	glGenBuffers(1, &coneVBO);
//	glBindVertexArray(coneVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
//	glBufferData(GL_ARRAY_BUFFER, coneVerts.size() * sizeof(GLfloat), &coneVerts[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
//	GLuint helixVBO, helixVAO;
//	glGenVertexArrays(1, &helixVAO);
//	glGenBuffers(1, &helixVBO);
//	glBindVertexArray(helixVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, helixVBO);
//	glBufferData(GL_ARRAY_BUFFER, helixVerts.size() * sizeof(GLfloat), &helixVerts[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
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
//			//up = !up;
//		}
//		if (up)
//			rot += 0.1f;
//		else
//			rot -= 0.1f;
//
//		rotation = glm::radians(rot);
//		
//		//Draw tetrahedron
//		glm::mat4 model;
//		//model = glm::translate(model, glm::vec3(0.0, 0.0, -3.0));
//		model = glm::rotate(model, rotation, glm::vec3(1.0f, 0.0f, 1.0f));
//		//model = glm::scale(model, glm::vec3(.2, .2, .2));
//
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
//
//		//For cylinder:
//		glDrawArrays(GL_TRIANGLES, 0, verts.size() / 2);
//
//		//CONE:
//		glBindVertexArray(coneVAO);
//		glm::mat4 coneModel;
//		coneModel = glm::translate(coneModel, glm::vec3(0.0f, -.5f, 0.0f));
//		coneModel = model * coneModel;
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(coneModel));
//		glDrawArrays(GL_TRIANGLES, 0, coneVerts.size() / 2);
//
//		glBindVertexArray(helixVAO);
//		glm::mat4 helixModel;
//		//helixModel = glm::translate(helixModel, glm::vec3(0.5f, 0.0f, 0.0f));
//		helixModel = model * helixModel;
//		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(helixModel));
//		glDrawArrays(GL_LINE_STRIP, 0, helixVerts.size() / 6);
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
//	glDeleteVertexArrays(1, &coneVAO);
//	glDeleteBuffers(1, &coneVBO);
//
//	// Terminate GLFW, clearing any resources allocated by GLFW.
//	glfwTerminate();
//
//	return 0;
//}
//
//
//
//
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