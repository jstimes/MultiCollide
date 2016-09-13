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
////#include <SOIL.h>
//
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
//#include "Superquadric.h"
//
//#include "GLView.h"
//#include "GLPanel.h"
//
//// Function prototypes
//float maxDistanceFromCentroid(TetrahedralMesh &mesh);
//void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals);
//bool verifyOrthogonalVecs(glm::dvec3 a, glm::dvec3 b, glm::dvec3 c);
//
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
//void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
//
//void AddPairOfPoints(std::vector<GLfloat> &vector, glm::vec3 pt1, glm::vec3 pt2);
//void AddPoint(std::vector<GLfloat> &vector, glm::vec3 pt, glm::vec3 normal);
//void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2);
//bool BoundingSphereTest(Superquadric &sq1, Superquadric & sq2);
//
//void onClickTester() {
//	std::cout << "Clicked" << std::endl;
//}
//
//glm::vec3 ClosestPointEstimate(Superquadric &sq, glm::mat4 rotation, glm::vec3 trans, glm::vec3 vectorToOtherSq) {
//	//Normalize vector, then convert to local coordinates of superquadric
//	//vectorToOtherSq = glm::normalize(vectorToOtherSq);
//
//	//untranslate:
//	//vectorToOtherSq = (-1.0f * trans) + vectorToOtherSq;
//
//	//unrotate - use transpose for inverse rotation:
//	glm::vec3 normal = glm::vec3(glm::transpose(rotation) * glm::vec4(vectorToOtherSq.x, vectorToOtherSq.y, vectorToOtherSq.z, 1.0f));
//	normal = glm::normalize(normal);
//
//	////find the point on the superquadric where it has a normal vector equal to this vector
//	glm::vec2 surfaceParams = sq.SurfaceParamValuesFromNormal(normal);
//	glm::vec3 localPt = sq.PointsFromSurfaceParams(surfaceParams.x, surfaceParams.y);
//
//	//return pt converted back to global coordinates:
//	return trans + glm::vec3(rotation * glm::vec4(localPt.x, localPt.y, localPt.z, 1.0f));
//}
//
//// Window dimensions
//const GLuint WIDTH = 800, HEIGHT = 600;
//
//GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
//GLfloat lastFrame = 0.0f;      // Time of last frame
//
//float zOffset = -6.0f;
//
// //Matrices:
//glm::mat4 i4;
//glm::mat4 view = glm::translate(i4, glm::vec3(0.0f, 0.0f, zOffset));
//glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
//
////Shader uniform locations:
//GLint modelLoc;
//GLint viewLoc;
//GLint projLoc;
//GLint lightPosLoc;
//GLint objectColorLoc;
//
//bool paused = false;
//bool reverse = false; 
//
//float mouseX = 0.0f;
//float mouseY = 0.0f;
//std::vector<GLView*> views;
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
//	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "MultiCollide", nullptr, nullptr);
//	glfwMakeContextCurrent(window);
//
//	glfwSetKeyCallback(window, key_callback);
//
//	glfwSetCursorPosCallback(window, cursor_position_callback);
//
//	glfwSetMouseButtonCallback(window, mouse_button_callback);
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
//	glLineWidth(3.0f);
//
//	// Build and compile our shader program
//	Shader bareShader("vertexShader.glsl", "fragmentShader.glsl");
//
//	modelLoc = glGetUniformLocation(bareShader.Program, "model");
//	viewLoc = glGetUniformLocation(bareShader.Program, "view");
//	projLoc = glGetUniformLocation(bareShader.Program, "projection");
//	lightPosLoc = glGetUniformLocation(bareShader.Program, "lightPos");
//	objectColorLoc = glGetUniformLocation(bareShader.Program, "objectColor");
//
//#pragma endregion
//
//	bool run1 = false;
//
//	Superquadric sq1 = Superquadric::CreateSphere(); 
//	Superquadric sq2 = Superquadric::CreateSphere();
//	Superquadric sq3 = Superquadric::CreateSphere();
//	Superquadric sq4 = Superquadric::CreateSphere();
//
//	if (run1) {
//		sq1.a1 = 1.0f;
//		sq1.a2 = 1.0f;
//		sq1.a3 = 1.0f;
//		sq1.e1 = 2.0f;
//		sq1.e2 = 1.0f;  // .3f  1.0f  2.0f  3.0f
//		sq1.u1 = -glm::pi<float>() / 2.0f;
//		sq1.u2 = glm::pi<float>() / 2.0f;
//		sq1.v1 = -glm::pi<float>();
//		sq1.v2 = glm::pi<float>();
//		sq1.u_segs = 30;
//		sq1.v_segs = 30;
//
//		sq2.a1 = 1.0f;
//		sq2.a2 = 1.0f;
//		sq2.a3 = 1.0f;
//		sq2.e2 = 3.0f;
//		sq2.e1 = 1.0f;
//		sq2.u1 = -glm::pi<float>() / 2.0f;
//		sq2.u2 = glm::pi<float>() / 2.0f;
//		sq2.v1 = -glm::pi<float>();
//		sq2.v2 = glm::pi<float>();
//		sq2.u_segs = 30;
//		sq2.v_segs = 30;
//
//		sq3.a1 = 1.0f;
//		sq3.a2 = 1.0f;
//		sq3.a3 = 1.0f;
//		sq3.e2 = 3.0f;
//		sq3.e1 = 3.0f;
//		sq3.u1 = -glm::pi<float>() / 2.0f;
//		sq3.u2 = glm::pi<float>() / 2.0f;
//		sq3.v1 = -glm::pi<float>();
//		sq3.v2 = glm::pi<float>();
//		sq3.u_segs = 30;
//		sq3.v_segs = 30;
//
//		sq4.a1 = 1.0f;
//		sq4.a2 = 1.0f;
//		sq4.a3 = 1.0f;
//		sq4.e2 = 3.0f;
//		sq4.e1 = 3.0f;
//		sq4.u1 = -glm::pi<float>() / 2.0f;
//		sq4.u2 = glm::pi<float>() / 2.0f;
//		sq4.v1 = -glm::pi<float>();
//		sq4.v2 = glm::pi<float>();
//		sq4.u_segs = 30;
//		sq4.v_segs = 30;
//	}
//
//
//	//double area = SuperEllipsoid::SurfaceArea(sq1, 3.14f / 4.0f, 3.14f / 4.0f);
//
//	//std::cout << area << std::endl;
//
//	std::vector<Superquadric> sqs;
//	sqs.push_back(sq1);
//	sqs.push_back(sq2);
//	sqs.push_back(sq3);
//	sqs.push_back(sq4);
//
//	for (std::vector<Superquadric>::iterator it = sqs.begin(); it != sqs.end(); it++) {
//		//SuperEllipsoid::uniformAreaPoints2(*it);
//		SuperEllipsoid::sqSolidEllipsoid(*it);
//		//SuperEllipsoid::uniformAreaPoints3(*it);
//		Superquadric::InitializeClosestPoints(*it);
//		it->InitVAOandVBO();
//	}
//
//	float SCALE = 1.0f; //Scaling factor for the shapes
//
//	glm::vec3 sq1centroid(-1.5f, -0.4f, -1.5f);
//	glm::vec3 sq2centroid(1.5f, -0.4f, -1.5f);
//	glm::vec3 sq3centroid(1.5f, -0.4f, 1.5f);
//	glm::vec3 sq4centroid(-1.5f, -0.4f, 1.5f);
//
//	//Initially no superquadrics have any rotation:
//	sqs[0].rotation = sqs[1].rotation = sqs[2].rotation = sqs[3].rotation = glm::mat4();
//
//	sqs[0].rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
//	sqs[1].rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
//	sqs[2].rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
//	sqs[3].rotationAxis = glm::vec3(1.0f, 0.3f, 1.0f);
//
//	sqs[0].translation = sqs[0].centroid = sq1centroid;
//	sqs[1].translation = sqs[1].centroid = sq2centroid;
//	sqs[2].translation = sqs[2].centroid = sq3centroid;
//	sqs[3].translation = sqs[3].centroid = sq4centroid;
//
//	glm::vec3 closestPtsq1; 
//	glm::vec3 closestPtsq2; 
//
//	GLfloat phi = glm::pi<float>() / 2048.0f;
//
//	GLfloat rotation = phi;
//	rotation = -glm::pi<float>() / 2.0f;
//	bool rotateX = true;
//	bool rotate = true;
//
//	glm::vec3 initialVelocity1(1.5f, 2.0f, 1.5f);
//	sqs[0].afterVelocity = glm::vec3(-1.5f, 2.0f, -1.5f);
//	glm::vec3 initialVelocity2(-1.5f, 2.0f, 1.5f);
//	sqs[1].afterVelocity = glm::vec3(1.5f, 2.0f, -1.5f);
//
//	glm::vec3 initialVelocity3(-1.5f, 2.0f, -1.5f);
//	sqs[2].afterVelocity = glm::vec3(1.5f, 2.0f, 1.5f);
//	glm::vec3 initialVelocity4(1.8f, 2.2f, -1.5f);
//	sqs[3].afterVelocity = glm::vec3(-1.5f, 2.0f, 1.5f);
//
//	float contactAccuracy = 0.00001f;
//	float gravity = -9.8f;
//	float startTime = glfwGetTime();
//
//	sqs[0].curVelocity = initialVelocity1;
//	sqs[1].curVelocity =  initialVelocity2;
//	sqs[2].curVelocity =  initialVelocity3;
//	sqs[3].curVelocity = initialVelocity4;
//
//	if (!run1) {
//		sqs[0].centroid = glm::vec3(0.0f, -2.5f, -2.5f);
//		sqs[0].curVelocity = glm::vec3(0.0f, 2.0f, 0.0f);
//		sqs[0].afterVelocity = glm::vec3(0.0f, .2f, 0.0f);
//
//		sqs[1].centroid = glm::vec3(0.0f, 0.0f, -2.5f);
//		sqs[1].curVelocity = glm::vec3(0.0f, -1.0f, 0.0f);
//		sqs[1].afterVelocity = glm::vec3(0.0f, 1.2f, 0.0f);
//
//		sqs[2].centroid = glm::vec3(0.0f, 2.5f, -2.50f);
//		sqs[2].curVelocity = glm::vec3(0.0f, -0.5f, 0.0f);
//		sqs[2].afterVelocity = glm::vec3(0.0f, 2.9f, 0.0f);
//
//		sqs[3].centroid = glm::vec3(0.0f, 5.0f, -2.5f);
//		sqs[3].curVelocity = glm::vec3(0.0f, -0.2f, 0.0f);
//		sqs[3].afterVelocity = glm::vec3(0.0f, 1.6f, 0.0f);
//
//		sqs[0].translation = sq1centroid = sqs[0].centroid;
//		sqs[1].translation = sq2centroid = sqs[1].centroid;
//		sqs[2].translation = sq3centroid = sqs[2].centroid;
//		sqs[3].translation = sq4centroid = sqs[3].centroid;
//	}
//
//	float speed = 1.0f;
//	sqs[0].time = sqs[1].time = sqs[2].time = sqs[3].time = 0.0f;
//	float timeUpdate = .0006f;
//	bool UseBoundingSphere = true;
//
//	bool displayOne = false;
//	bool displayNonsingular = false;
//	bool done = false;
//	bool drawPlane = true;
//
//	GLPanel *testView = new GLPanel(2.0, .6f, GLPanel::horizontal, .1f, .1f, .1f);
//	testView->Translate(glm::vec3(-1.0f, -1.0f, 0.0f));
//	testView->SetColor(glm::vec4(.0f, .0f, .0f, 1.0f));
//	testView->SetClickListener(onClickTester);
//
//	/*GLView *testView2 = new GLView(.3f, .2f);
//	testView2->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
//	testView2->SetClickListener(onClickTester);
//
//	GLView *testView3 = new GLView(.3f, .2f);
//	testView3->SetColor(glm::vec4(.9f, .9f, .3f, 1.0f));
//	testView3->SetClickListener(onClickTester);
//
//	GLView *testView4 = new GLView(.3f, .2f);
//	testView4->SetColor(glm::vec4(.9f, .3f, .3f, 0.0f));
//	testView4->SetClickListener(onClickTester);
//
//	testView->AddView(testView2);
//	testView->AddView(testView3);
//	testView->AddView(testView4);*/
//
//	float panelWidth = 2.0f / 5.0f;
//	float panelHeight = 2.0f / 5.0f;
//
//	float yStart = -1.0f;
//	float xStart = -1.0f;
//
//	for (int j = 0; j < 5; j++) {
//		float yPt = yStart + j*panelHeight;
//		for (int i = 0; i < 5; i++) {
//			float xPt = xStart + i*panelWidth;
//			GLPanel *testView = new GLPanel(panelWidth, panelHeight, GLPanel::vertical, .1f, .1f, .1f);
//			testView->Translate(glm::vec3(xPt, yPt, 0.0f));
//			testView->SetColor(glm::vec4(abs(j*.1f + i*.1f), abs(j*.1f + i*.1f), .0f, 1.0f));
//			testView->SetClickListener(onClickTester);
//			testView->InitBuffers();
//			views.push_back(testView);
//		}
//	}
//
//	//views.push_back(testView);
//
//	// Render loop
//	while (!glfwWindowShouldClose(window))
//	{
//		glfwPollEvents();
//		if (displayOne) {
//			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//			// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
//			//glfwPollEvents();
//			// Render
//			// Clear the colorbuffer
//			glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//			bareShader.Use();
//			glm::dvec3 lightPos(0.0f, 3.0f, 3.0f);
//			glm::dvec3 objectColor(1.0f, 0.5f, 0.31f);
//
//			glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
//			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
//			//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1)); //all models set below
//			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//			glm::mat4 modelo;
//			modelo = glm::scale(modelo, glm::vec3(2.0f, 2.0f, 2.0f));
//			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelo));
//			glBindVertexArray(sqs[0].VAO);
//			sqs[0].Draw();
//			glBindVertexArray(0);
//
//			//Uniform points
//			std::vector<GLfloat> pts;
//			for (int i = 0; i < sqs[0].points.size(); i++) {
//				AddPairOfPoints(pts, sqs[0].points[i].pt, sqs[0].points[i].pt);
//			}
//			GLuint VAOdist, VBOdist;
//			glGenVertexArrays(1, &VAOdist);
//			glGenBuffers(1, &VBOdist);
//			glBindVertexArray(VAOdist);
//			glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
//			glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(GLfloat), &pts[0], GL_STATIC_DRAW);
//
//			// Position attribute
//			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//			glEnableVertexAttribArray(0);
//
//			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//			glEnableVertexAttribArray(1);
//
//			bareShader.Use();
//			glPointSize(15.00f);
//			objectColor = glm::vec3(1.0f, 0.0f, 0.0f);
//
//			glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
//			glDrawArrays(GL_POINTS, 0, pts.size() / 6);
//			glBindVertexArray(0);
//
//			glDeleteBuffers(1, &VBOdist);
//			glDeleteBuffers(1, &VAOdist);
//
//			if (drawPlane) {
//				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//				GLuint VAOdist, VBOdist;
//				std::vector<GLfloat> vertices;
//
//				glm::vec3 bL(-2.5f, -2.5f, 0.0f);
//				glm::vec3 bR(2.5f, -2.5f, 0.0f);
//				glm::vec3 tR(2.5f, 2.5f, 0.0f);
//				glm::vec3 tL(-2.5f, 2.5f, 0.0f);
//				glm::vec3 norm(0.0f, 0.0f, 1.0f);
//
//				AddPoint(vertices, bL, norm);
//				AddPoint(vertices, bR, norm);
//				AddPoint(vertices, tR, norm);
//
//				AddPoint(vertices, bL, norm);
//				AddPoint(vertices, tR, norm);
//				AddPoint(vertices, tL, norm);
//
//				glGenVertexArrays(1, &VAOdist);
//				glGenBuffers(1, &VBOdist);
//				glBindVertexArray(VAOdist);
//				glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
//				glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
//
//				// Position attribute
//				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//				glEnableVertexAttribArray(0);
//
//				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//				glEnableVertexAttribArray(1);
//
//				glm::mat4 lineModel;
//				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
//				glUniform3f(objectColorLoc, 1.0f, 1.0f, 1.0f);
//				glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
//
//				glBindVertexArray(0);
//
//				glDeleteBuffers(1, &VBOdist);
//				glDeleteBuffers(1, &VAOdist);
//			}
//		}
//		else if (displayNonsingular) {
//			//if (!done) {
//				std::vector<glm::vec3> points = sqs[0].nonsingulars;
//				std::vector<GLfloat> pts;
//				for (int i = 0; i < points.size() - 1; i += 2) {
//					AddPairOfPoints(pts, points[i], points[i + 1]);
//				}
//				glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
//				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//				GLuint VAOdist, VBOdist;
//				glGenVertexArrays(1, &VAOdist);
//				glGenBuffers(1, &VBOdist);
//				glBindVertexArray(VAOdist);
//				glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
//				glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(GLfloat), &pts[0], GL_STATIC_DRAW);
//
//				// Position attribute
//				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//				glEnableVertexAttribArray(0);
//
//				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//				glEnableVertexAttribArray(1);
//
//				bareShader.Use();
//				glm::dvec3 lightPos(0.0f, 3.0f, 3.0f);
//				glm::dvec3 objectColor(1.0f, 0.5f, 0.31f);
//
//				glm::mat4 lineModel;
//				lineModel = glm::translate(lineModel, glm::vec3(0.0f, 0.0f, -2.0f));
//				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
//				glUniform3f(objectColorLoc, 0.0f, 0.0f, 0.0f);
//				glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
//				glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
//				//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1)); //all models set below
//				glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//				glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//				glPointSize(5.00f);
//				glDrawArrays(GL_POINTS, 0, pts.size() / 6);
//				glBindVertexArray(0);
//
//				std::vector<GLfloat> bounds;
//				float pi = glm::pi<float>();
//				AddPairOfPoints(bounds, glm::vec3(-pi / 2.0f, -pi, 0.0f), glm::vec3(-pi / 2.0f, pi, 0.0f));
//				AddPairOfPoints(bounds, glm::vec3(-pi / 2.0f, -pi, 0.0f), glm::vec3(pi / 2.0f, -pi, 0.0f));
//
//				GLuint VAO, VBO;
//				glGenVertexArrays(1, &VAO);
//				glGenBuffers(1, &VBO);
//				glBindVertexArray(VAO);
//				glBindBuffer(GL_ARRAY_BUFFER, VBO);
//				glBufferData(GL_ARRAY_BUFFER, bounds.size() * sizeof(GLfloat), &bounds[0], GL_STATIC_DRAW);
//
//				// Position attribute
//				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//				glEnableVertexAttribArray(0);
//
//				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//				glEnableVertexAttribArray(1);
//
//				glPointSize(8.00f);
//				glDrawArrays(GL_LINES, 0, bounds.size() / 6);
//
//				glBindVertexArray(0);
//
//				glDeleteBuffers(1, &VBOdist);
//				glDeleteBuffers(1, &VAOdist);
//				done = true;
//				//	}
//		}
//		else {
//			//Uncomment for wireframe mode:
//			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//			GLfloat currentFrame = glfwGetTime();
//			deltaTime = currentFrame - lastFrame;
//			lastFrame = currentFrame;
//
//			// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
//			//glfwPollEvents();
//
//			sqs[0].time += timeUpdate;
//			sqs[1].time += timeUpdate;
//			sqs[2].time += timeUpdate;
//			sqs[3].time += timeUpdate;
//
//			if (paused) {
//				sqs[0].time -= timeUpdate;
//				sqs[1].time -= timeUpdate;
//				sqs[2].time -= timeUpdate;
//				sqs[3].time -= timeUpdate;
//				//continue;
//			}
//			if (reverse && !paused) {
//				sqs[0].time -= 2.0f * timeUpdate;
//				sqs[1].time -= 2.0f * timeUpdate;
//				sqs[2].time -= 2.0f * timeUpdate;
//				sqs[3].time -= 2.0f * timeUpdate;
//			}
//
//			// Render
//			// Clear the colorbuffer
//			glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
//			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//			//Update position of each superquad:
//			for (int i = 0; i < sqs.size(); i++) {
//				sqs[i].translation = sqs[i].centroid + (sqs[i].curVelocity * sqs[i].time) + glm::vec3(0.0f, .5f * gravity * sqs[i].time * sqs[i].time, 0.0f);
//			}
//
//			if (!paused && !reverse) {
//				rotation += phi;
//			}
//			else if (!paused && reverse) {
//				rotation -= phi; //?
//			}
//
//			bareShader.Use();
//			glm::dvec3 lightPos(0.0f, 3.0f, 3.0f);
//			glm::dvec3 objectColor(1.0f, 0.5f, 0.31f);
//
//			glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
//			glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
//			//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1)); //all models set below
//			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
//			glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//
//			//Draw each superquadric based on its own translation && rotation
//			for (std::vector<Superquadric>::iterator it = sqs.begin(); it != sqs.end(); it++) {
//				glm::mat4 modelo;
//				modelo = glm::translate(modelo, it->translation);
//
//				if (run1) {
//					it->rotation = glm::rotate(glm::mat4(), rotation, it->rotationAxis);
//				}
//
//				modelo = modelo * it->rotation;
//				modelo = glm::scale(modelo, glm::vec3(SCALE, SCALE, SCALE));
//				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelo));
//				glBindVertexArray(it->VAO);
//				it->Draw();
//				glBindVertexArray(0);
//			}
//
//			//Find closest points/collisions
//			for (int i = 0; i < sqs.size() - 1; i++) {
//				for (int j = i + 1; j < sqs.size(); j++) {
//
//					//Closest points via brute force:
//					if (BoundingSphereTest(sqs[i], sqs[j])) {
//
//						DrawClosestPoints(sqs[i], sqs[j], i, j, closestPtsq1, closestPtsq2);
//
//						float distance = glm::distance(closestPtsq1, closestPtsq2);
//
//						//check for collision:
//						if (distance < contactAccuracy || glm::dot(sqs[i].translation - sqs[j].translation, closestPtsq1 - closestPtsq2) < 0.0) {
//							//Change velocity, reset time, and update initial point
//							sqs[i].curVelocity = sqs[i].afterVelocity;
//							sqs[j].curVelocity = sqs[j].afterVelocity;
//
//							if (!run1) {
//								sqs[i].afterVelocity = -sqs[i].afterVelocity;
//								sqs[j].afterVelocity = -sqs[j].afterVelocity;
//							}
//
//							sqs[i].time = sqs[j].time = 0.0f;
//
//							sqs[i].centroid = sqs[i].translation;
//							sqs[j].centroid = sqs[j].translation;
//						}
//					}
//				}
//			}
//		}
//
//		for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
//			(*it)->Draw();
//		}
//
//		// Swap the screen buffers
//		glfwSwapBuffers(window);
//	}
//
//	// Properly de-allocate all resources once they've outlived their purpose
//	for (std::vector<Superquadric>::iterator it = sqs.begin(); it != sqs.end(); it++) {
//		glDeleteVertexArrays(1, &(it->VAO));
//		glDeleteBuffers(1, &(it->VBO));
//	}
//
//	//Cleanup Views:
//	for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
//		delete *it;
//	}
//
//	// Terminate GLFW, clearing any resources allocated by GLFW.
//	glfwTerminate();
//
//	return 0;
//}
//
//
//bool verifyOrthogonalVecs(glm::dvec3 a, glm::dvec3 b, glm::dvec3 c) {
//	float dotab = glm::dot(a, b);
//	float dotac = glm::dot(a, c);
//	float dotbc = glm::dot(b, c);
//	return true;
//}
//
//// Is called whenever a key is pressed/released via GLFW
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
//{
//	//Stop if escape key pressed
//	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//		glfwSetWindowShouldClose(window, GL_TRUE);
//
//	//Handle pause
//	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
//		paused = true;
//	}
//	else if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
//		paused = false;
//	}
//
//	//Handle reverse time:
//	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
//		reverse = true;
//	}
//	else if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
//		reverse = false;
//	}
//
//	//Handle arrow keys for rotating camera:
//	if (key == GLFW_KEY_LEFT) {
//		view = glm::rotate(view, .05f, glm::vec3(0.0f, 1.0f, 0.0f));
//	}
//	else if (key == GLFW_KEY_RIGHT) {
//		view = glm::rotate(view, -.05f, glm::vec3(0.0f, 1.0f, 0.0f));
//	}
//
//	if (key == GLFW_KEY_UP) {
//		view = glm::rotate(view, .05f, glm::vec3(1.0f, 0.0f, 0.0f));
//	}
//	else if (key == GLFW_KEY_DOWN) {
//		view = glm::rotate(view, -.05f, glm::vec3(1.0f, 0.0f, 0.0f));
//	}
//
//}
//
//void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	mouseX = xpos / WIDTH;
//	mouseX = ((mouseX - .5f) / .5f);
//
//	mouseY = ypos / HEIGHT;
//	mouseY = ((mouseY - .5f) / -.5f);
//
//
//}
//
//void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
//{
//	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//		glm::vec3 mouseClick = glm::vec3(mouseX, mouseY, 0.0f);
//		for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
//			if ((*it)->CheckClicked(mouseClick)) {
//				(*it)->OnClick(mouseClick);
//			}
//		}
//
//	}
//}
//
//void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals) {
//	float max = (eigenVal1 > eigenVal2 ? (eigenVal1 > eigenVal3 ? eigenVal1 : eigenVal3) : (eigenVal2 > eigenVal3 ? eigenVal2 : eigenVal3));
//	float remaining = abs(1.0f - minNormalization);
//	float a = (eigenVal1 / max) * remaining;
//	float b = (eigenVal2 / max) * remaining;
//	float c = (eigenVal3 / max) * remaining;
//	normalizedEvals.push_back(minNormalization + a);
//	normalizedEvals.push_back(minNormalization + b);
//	normalizedEvals.push_back(minNormalization + c);
//}
//
//float maxDistanceFromCentroid(TetrahedralMesh &mesh) {
//	float max = -1.0f;
//	glm::dvec3 centroid = mesh.MeshCentroid;
//	typedef std::vector<Tetrahedron>::iterator iter;
//	for (iter it = mesh.Tetrahedra.begin(); it != mesh.Tetrahedra.end(); it++) {
//		float distA = glm::distance(centroid, it->a);
//		float distB = glm::distance(centroid, it->b);
//		float distC = glm::distance(centroid, it->c);
//		float distD = glm::distance(centroid, it->d);
//		if (distA > max) {
//			max = distA;
//		}
//		if (distB > max) {
//			max = distB;
//		}
//		if (distC > max) {
//			max = distC;
//		}
//		if (distD > max) {
//			max = distD;
//		}
//	}
//	return max;
//}
//
//void AddPairOfPoints(std::vector<GLfloat> &vector, glm::vec3 pt1, glm::vec3 pt2) {
//	vector.push_back(pt1.x);
//	vector.push_back(pt1.y);
//	vector.push_back(pt1.z);
//	vector.push_back(1.0f);
//	vector.push_back(1.0f);
//	vector.push_back(1.0f);
//
//	vector.push_back(pt2.x);
//	vector.push_back(pt2.y);
//	vector.push_back(pt2.z);
//	vector.push_back(1.0f);
//	vector.push_back(1.0f);
//	vector.push_back(1.0f);
//}
//
//void AddPoint(std::vector<GLfloat> &vector, glm::vec3 pt, glm::vec3 normal) {
//	vector.push_back(pt.x);
//	vector.push_back(pt.y);
//	vector.push_back(pt.z);
//	vector.push_back(normal.x);
//	vector.push_back(normal.y);
//	vector.push_back(normal.z);
//}
//
//struct ParamPointPair {
//	ParamPoint p1;
//	ParamPoint p2;
//};
//
//ParamPointPair initial = { {-11.0f, -11.0f, glm::vec3()},{ -11.0f, -11.0f, glm::vec3() } };
//
//// A mapping used to store previous closest points
//ParamPointPair previouses[4][4] = { { initial , initial , initial ,initial },
//									{ initial , initial , initial ,initial }, 
//									{ initial , initial , initial ,initial }, 
//									{ initial , initial , initial ,initial }, };
//
////Finds the closest points between sq1 & sq2, saves those in closest1 & closest2, and then draws a line
//// between the two points
//void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2) {
//	GLuint VAOdist, VBOdist;
//	std::vector<GLfloat> vertices;
//
//	if (previouses[sq1index][sq2index].p1.u < -10.f) {
//		Superquadric::ClosestPointFramework(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
//	}
//	else {
//		Superquadric::ClosestPointFrameworkTryPrevious(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
//	}
//
//	if (sq2index > 10) {
//		std::cout << "Hey" << std::endl;
//	}
//	
//	AddPairOfPoints(vertices, closest1, closest2);
//	
//
//	glGenVertexArrays(1, &VAOdist);
//	glGenBuffers(1, &VBOdist);
//	glBindVertexArray(VAOdist);
//	glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
//	glm::mat4 lineModel;
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
//	glUniform3f(objectColorLoc, 0.0f, 0.0f, 0.0f);
//	glDrawArrays(GL_LINES, 0, vertices.size() / 6);
//
//	glBindVertexArray(0);
//
//	glDeleteBuffers(1, &VBOdist);
//	glDeleteBuffers(1, &VAOdist);
//}
//
//void DrawLine(glm::vec3 pt1, glm::vec3 pt2) {
//
//	GLuint VAOdist, VBOdist;
//	std::vector<GLfloat> vertices;
//
//	AddPairOfPoints(vertices, pt1, pt2);
//
//
//	glGenVertexArrays(1, &VAOdist);
//	glGenBuffers(1, &VBOdist);
//	glBindVertexArray(VAOdist);
//	glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
//	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
//
//	// Position attribute
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//	glEnableVertexAttribArray(1);
//
//	glm::mat4 lineModel;
//	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
//	glUniform3f(objectColorLoc, 0.0f, 0.0f, 0.0f);
//	glDrawArrays(GL_LINES, 0, vertices.size() / 6);
//
//	glBindVertexArray(0);
//
//	glDeleteBuffers(1, &VBOdist);
//	glDeleteBuffers(1, &VAOdist);
//}
//
////Returns true if bounding spheres are intersecting/touching
//// Doesn't account for object velocities !!
//bool BoundingSphereTest(Superquadric &sq1, Superquadric & sq2) {
//	glm::vec3 actualDistanceVec = sq1.translation - sq2.translation;
//	float minAllowableDistance = sq1.boundingSphereRadius + sq2.boundingSphereRadius;
//
//	//Use squared distance rather than computing square roots:
//	float actualDistance = glm::dot(actualDistanceVec, actualDistanceVec);
//	minAllowableDistance = minAllowableDistance * minAllowableDistance;
//
//	return actualDistance <= minAllowableDistance;
//}