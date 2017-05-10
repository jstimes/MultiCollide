#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <emscripten.h>
//#include "Tests2D.h"

// GLEW
// GLFW
// GLM Mathematics
#include "ShapeUtils.h" // includes all necessary glm headers, GLEW & GLFW defs, mathutils, shader

// Other Libs
//#include <SOIL.h> //only used for textures

//EIGEN - SVD 
#include <Eigen/Dense>
#include <Eigen\SVD>

#include "Shape.h"
#include "Quaternion.h"
#include "TetrahedralMesh.h"
#include "Superquadric.h"
#include "Cube.h"
#include "Icosahedron.h"

#include "Circle.h"
#include "Triangle.h"
#include "Square.h"
#include "Polygon.h"

#include "Frame.h"

#include "Impact2D.h"
#include "Impact.h"
#include "ImpactClosedForm.h"
#include "ShapeShader.h"
#include "ColorShapeShader.h"

#include "GLViewIncludes.h"

#include "Keyboard.h"
#include "Text.h"
#include "Camera.h"

#include "CollisionDetector.h"

const bool USING_EMSCRIPTEN = true;

bool usingClosedFormImpact = false;
bool using2DShapes = false;
float frictionCoefficient = 0.05f;
float restitutionCoefficient = 1.0f;


//For cast debugging:
Superquadric* castToSuperquadric(Shape *shape, std::string method) {
	if(USING_EMSCRIPTEN)
		std::cout << "Before cast (" << method << ")" << std::endl;

	Superquadric* sq = static_cast<Superquadric*>(shape);

	if (USING_EMSCRIPTEN)
		std::cout << "After cast (" << method << ")" << std::endl;

	return sq;
}

// Function prototypes
float maxDistanceFromCentroid(TetrahedralMesh &mesh);
void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals);
bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c);

//Input callbacks
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void window_size_callback(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void AddPairOfPoints(std::vector<GLfloat> &vector, glm::vec3 pt1, glm::vec3 pt2);
void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2, ParamPoint &pp1, ParamPoint &pp2);
bool BoundingSphereTest(Shape &s1, Shape &s2);
void Collision(Shape &s1, ParamPoint &contactPt1, Shape &s2, ParamPoint &contactPt2, int i, int j);

bool checkForCollision(int shapeI, int shapeJ, ParamPoint &ppI, ParamPoint &ppJ);
void HandleCollision(int i, int j, ParamPoint &ppI, ParamPoint &ppJ);

void InitOpenGL();
void InitSetupSceneTransform();
void InitLeftPanel();
void InitBottomPanel();
void InitViews();

void PositionShapeToAdd(Shape *shape);
void Render();
void Close();
void DestroyViews();
void DestroyShapes();

glm::vec3 ClosestPointEstimate(Superquadric &sq, glm::mat4 rotation, glm::vec3 trans, glm::vec3 vectorToOtherSq) {
	//Normalize vector, then convert to local coordinates of superquadric
	//vectorToOtherSq = glm::normalize(vectorToOtherSq);

	//untranslate:
	//vectorToOtherSq = (-1.0f * trans) + vectorToOtherSq;

	//unrotate - use transpose for inverse rotation:
	glm::vec3 normal = glm::vec3(glm::transpose(rotation) * glm::vec4(vectorToOtherSq.x, vectorToOtherSq.y, vectorToOtherSq.z, 1.0f));
	normal = glm::normalize(normal);

	////find the point on the superquadric where it has a normal vector equal to this vector
	glm::vec2 surfaceParams = sq.SurfaceParamValuesFromNormal(normal);
	glm::vec3 localPt = sq.PointsFromSurfaceParams(surfaceParams.x, surfaceParams.y);

	//return pt converted back to global coordinates:
	return trans + glm::vec3(rotation * glm::vec4(localPt.x, localPt.y, localPt.z, 1.0f));
}

struct SidePanel {
	GLPanel *panel;

	GLButton *sphereBtn;
	GLButton *cubeBtn;
	GLButton *ellipsoidBtn;
	GLButton *superquadricBtn;
	GLButton *icosahedronBtn;
	GLButton *runBtn;
	GLButton *resetBtn;
	GLButton *positionBtn;
	GLButton *rotateBtn;
	GLButton *scaleUniformBtn;
	GLButton *velocityBtn;
	GLButton *angularVelocityBtn;

} sidePanelStr;

// Window dimensions
GLuint WIDTH = 800, HEIGHT = 700;

GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame

float zOffset = -6.0f;

//glm::vec3 initialCameraPos = glm::vec3(-3.0f, 1.0f, 0.0f);
//glm::vec3 cameraPos = initialCameraPos;
//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

//Matrices:
glm::mat4 i4;
//glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

////Shader uniform locations:
//GLint modelLoc;
//GLint viewLoc;
//GLint projLoc;
//GLint lightPosLoc;
//GLint objectColorLoc;
//GLint sceneTransformLoc;

//Scene state:
bool paused = false;
bool reverse = false;
bool setup = true;

bool closeProgram = false;

//If true, objects should start animating from current spot in scene setup
//If false, two+ objects should be in contact before pressing run.
//  and upon pressing run, an initial position should be found so that objects
//  are animated towards each other and hit with the preset velocities at configured contact point
bool isFreeFly = true;

bool plotPrincipalFrame = true;
bool plotRotationAxis = false;


float contactAccuracy = 0.001f;


float mouseXvp = 0.0f;
float mouseYvp = 0.0f;
float mouseX = 0.0f;
float mouseY = 0.0f;
bool mouseDown[2] = { false, false };
std::vector<GLView*> views;
GLPanel *leftPanel;
GLPanel *bottomPanel;

GLView *pauseBtn;
GLView *stopBtn;

Shader shapeShader;
GLFWwindow* window;
std::vector<Shape*> shapes;
Shape *pickedShape;
int pickedShapeIndex = -1;
glm::vec3 selectedColor = glm::vec3(.2f, .7f, .2f);
glm::mat4 sceneTransform;
glm::mat4 inverseSceneTransform;
glm::mat4 inverseProjection;

//Used so objects don't 're-collide' before they have  
// moved away from another colliding objects
std::map<std::pair<int, int>, int> framesSinceCollision;

//Defines what should be updated when shapes are clicked & dragged by mouse
enum CursorType { translate = 0, rotate = 1, scale_uniform = 2, velocity = 3, angular_velocity = 4 };
CursorType cursorState;

//enum CameraState { pan = 0, tilt = 1, zoom = 2 };
//CameraState cameraState = pan;
Camera camera;

void ResetCursorState() {
	switch (cursorState) {
	case translate:
		sidePanelStr.positionBtn->UnsetToggledColor();
		break;
	case rotate:
		sidePanelStr.rotateBtn->UnsetToggledColor();
		break;
	case scale_uniform:
		sidePanelStr.scaleUniformBtn->UnsetToggledColor();
		break;
	case velocity:
		sidePanelStr.velocityBtn->UnsetToggledColor();
	case angular_velocity:
		sidePanelStr.angularVelocityBtn->UnsetToggledColor();
	}
}

bool icoMode = false;
bool icoHit = false;

//Impulse stuff:
ImpactOutput previousImpactResults;
int curImpulsePt = 0;
int curVelocityPt = 0;
bool impulseMode = false;
Shape *contactShape1 = nullptr;
Shape *contactShape2 = nullptr;
float prevCameraPosX = 0.0f;
float prevCameraPosY = 0.0f;
float prevCameraTargetX = 0.0f;
float prevCameraTargetY = 0.0f;

glm::vec3 cameraContactPt;
glm::vec3 cameraTranslation;
int cameraTranslations = 0;
int numCameraTranslations = 100;
bool cameraTransDone = false;

bool newData = false;

void AddShape(Shape *shape) {
	shape->translation = shape->centroid = glm::vec3(0.0f, 0.0f, 0.0f);
	PositionShapeToAdd(shape);
	shape->InitVAOandVBO(shapeShader);
	shape->ComputeInertia();
	shapes.push_back(shape);

	newData = true;
}

Polygon *customPolygonPtr = nullptr;

//ImpulseGraph *graph = nullptr;
//void renderImpulseGraph();

extern "C" {
	int
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getNumShapes();

	

	bool
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		isNewData() {
		bool toReturn = newData;

		if (newData) {
			newData = false;
		}

		return toReturn;
	}

	//UI will query this each frame, stopping and resetting the UI if true
	bool shapeOutOfScene = false;
	bool
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		isShapeOutOfScene() {
		bool toReturn = shapeOutOfScene;
		if(shapeOutOfScene){
			shapeOutOfScene = false; // Do this so once  UI queries this and find it's true, it won't still be true after a reset or something
		}
		return toReturn;
	}

	void 
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		createNewCustomPolygon() {
		customPolygonPtr = new Polygon();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		addCustomPolygonVertex(float x, float y) {
		customPolygonPtr->AddVertex(x, y);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		doneCreatingCustomPolygon() {
		customPolygonPtr->DoneAddingVertices();
		static int polygonNum = 1;
		customPolygonPtr->name = std::string("CustomPolygon" + std::to_string(polygonNum++));
		AddShape(customPolygonPtr);
}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE 
#endif
		deselectShape() {
		if (pickedShape != nullptr) {
			pickedShape->objectColor = pickedShape->defaultColor;
		}
		pickedShape = nullptr;
		mouseDown[0] = false;
	}

	int 
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getPickedShapeIndex() {
		return pickedShapeIndex;
	}

	bool
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		isImpulseMode() {
		return impulseMode;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setPlotPricipalFrame(bool plot) {
		plotPrincipalFrame = plot;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setPlotRotaionAxis(bool plot) {
		plotRotationAxis = plot;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapePositionX(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapePositionY(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapePositionZ(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeVelocityX(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeVelocityY(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeVelocityZ(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularVelocityX(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularVelocityY(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularVelocityZ(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularVelocitySpeed(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeRotationAxisX(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeRotationAxisY(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeRotationAxisZ(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeRotationAngle(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeMass(unsigned int shapeIndex);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeFriction(/*unsigned int shapeIndex*/);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeRestitution(/*unsigned int shapeIndex*/);

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeScale(unsigned int shapeIndex);

	//Web interface can't rely on original shape index
	// for getting/updating data since shapes can be removed
	// so uses name as unique ID since that never changes
	// Web interface grabs the name of an object right after 
	// it's created
	const char*
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeName(int index) {
		return shapes[index]->name.c_str();
	}

	const char* 
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getContactShape1Name() {
		return contactShape1->name.c_str();
	}

	const char*
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getContactShape2Name() {
		return contactShape2->name.c_str();
	}

	int
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeIndexForName(const char *name) {
		for (int i = 0; i < shapes.size(); i++) {
			if (shapes[i]->name.compare(std::string(name)) == 0) {
				return i;
			}
		}
		return -1;
	}

	/*
		Used to save scene to a file and reupload
	*/
	const char*
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getSceneCSV();

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaXX(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[0][0];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaXY(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[0][1];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaXZ(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[0][2];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaYX(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[1][0];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaYY(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[1][1];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaYZ(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[1][2];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaZX(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[2][0];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaZY(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[2][1];
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getShapeAngularInertiaZZ(unsigned int shapeIndex) {
		return shapes[shapeIndex]->angularInertia[2][2];
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapePositionX(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapePositionY(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapePositionZ(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeVelocityX(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeVelocityY(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeVelocityZ(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeAngularVelocityX(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeAngularVelocityY(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeAngularVelocityZ(unsigned int shapeIndex, float value);

//	void
//#ifdef EMSCRIPTEN_KEEPALIVE
//		EMSCRIPTEN_KEEPALIVE
//#endif
//		setShapeAngularVelocitySpeed(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeRotationAxisX(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeRotationAxisY(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeRotationAxisZ(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeRotationAngle(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeMass(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeFriction(/*unsigned int shapeIndex,*/ float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeRestitution(/*unsigned int shapeIndex,*/ float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setShapeScale(unsigned int shapeIndex, float value);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setCollisionMode(bool freeFly) {
		isFreeFly = freeFly;
	}


	//void EMSCRIPTEN_KEEPALIVE setShapeAttribute(unsigned int shapeIndex, std::string attribute, float value);
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		removeShape(unsigned int index);

	int
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		getSelectedShape();

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setSelectedShape(unsigned int shapeIndex);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		addDuplicateShape(unsigned int shapeIndex);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		focusCameraOnShape(unsigned int index);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		createCustomSuperquadric(float a1, float a2, float a3, float e1, float e2);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddSuperquadric(Superquadric *superquad) {
		SuperEllipsoid::sqSolidEllipsoid(*superquad);
		Superquadric::InitializeClosestPoints(*superquad);
		AddShape(superquad);
	}

	//void AddSquareOnClick();
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddSphereOnClick() {

		/*if (using2DShapes) {
			AddSquareOnClick();
		}*/
		
		
		Superquadric *sphere = new Superquadric();
		sphere->CreateSphere();
		static int sphereNum = 1;
		sphere->name = std::string("Sphere" + std::to_string(sphereNum++));
		sphere->setShapeCSVcode(0);
		AddSuperquadric(sphere);
		/*Tetra *ico = new Tetra();
		ico->name = "Ico";
		ico->translation = ico->centroid = glm::vec3(0.9f, 0.0f, 0.0f);
		PositionShapeToAdd(ico);
		Shader shader = ColorShapeShader::getInstance().shader;
		ico->InitVAOandVBO(shader);

		AddShape(ico);*/
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddCircleOnClick() {
		Circle *circle = new Circle();
		static int circleNum = 1;
		circle->name = std::string("Circle" + std::to_string(circleNum++));
		
		AddShape(circle);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddPolygonOnClick(int numSides) {
		Polygon *polygon = new Polygon(numSides);
		static int regpolygonNum = 1;
		polygon->name = std::string("RegularPolygon" + std::to_string(regpolygonNum++));
		AddShape(polygon);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddSquareOnClick() {
		Square *square = new Square();
		static int squareNum = 1;
		square->name = std::string("Square" + std::to_string(squareNum++));
		AddShape(square);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddTriangleOnClick() {
		Triangle *triangle = new Triangle();
		
		static int triangleNum = 1;
		triangle->name = std::string("Triangle" + std::to_string(triangleNum++));
		
		AddShape(triangle);
	}


	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		PauseOnClick() {
		paused = !paused;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		StopOnClick() {
		//DestroyViews();

		for (unsigned int i = 0; i < shapes.size(); i++) {
			shapes[i]->centroid = shapes[i]->translation;
			shapes[i]->time = 0.0f;
		}
		paused = false;
		//InitLeftPanel();
		//InitBottomPanel();
		//InitCameraControls();
		//InitViews();
		setup = true;
		if (!USING_EMSCRIPTEN)
			InitSetupSceneTransform();

	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		resetCameraState() {
		camera.resetCameraState();
	}

	//Clears shapes and resets camera
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		ResetOnClick() {
		std::cout << "Clicked Reset" << std::endl;
		DestroyShapes();
		shapes.clear();
		resetCameraState();

		icoMode = false;
		icoHit = false;
		impulseMode = false;
		setup = true;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		RunOnClick() {

		if (shapes.size() == 0) {
			return;
		}

		if (!isFreeFly) {
			int numShapes = getNumShapes();
			/*for (int i = 0; i < numShapes; i++) {
				shapes[i]->centroid = shapes[i]->translation = shapes[i]->centroid + -shapes[i]->curVelocity;
			}*/

			for (int i = 0; i < numShapes; i++) {
				shapes[i]->BackwardsIntegrate();
			}
		}

		setup = false;
		sceneTransform = i4;
		inverseSceneTransform = i4;
		if (pickedShape != nullptr) {
			pickedShape->objectColor = pickedShape->defaultColor;
		}
		pickedShape = nullptr;

		//HidePanels();

		//views.push_back(pauseBtn);
		//views.push_back(stopBtn);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddIcosahedronOnClick() {

		ResetOnClick();

		Icosahedron *ico = new Icosahedron();
		static int icoNum = 1;
		ico->name = std::string("Icosahedron" + std::to_string(icoNum++));

		ico->translation = ico->centroid = glm::vec3(0.9f, 0.0f, 0.0f);
		PositionShapeToAdd(ico);
		Shader shader = ColorShapeShader::getInstance().shader;
		ico->InitVAOandVBO(shader);


		Tetra *tetra = new Tetra();
		static int tetraNum = 1;
		tetra->name = std::string("Tetrahedron" + std::to_string(tetraNum++));
		tetra->translation = tetra->centroid = glm::vec3(-.74f, 0.0f, 0.6f);
		//PositionShapeToAdd(tetra);
		tetra->InitVAOandVBO(shapeShader);

		//ico->frictionCoefficient = tetra->frictionCoefficient = .8f;
		//ico->restitutionCoefficient = tetra->restitutionCoefficient = .95f;
		setShapeFriction(.8f);
		setShapeRestitution(.95f);

		ico->curVelocity = glm::vec3(.95f, -.1f, .5f);
		tetra->curVelocity = glm::vec3(1.0f, .1f, .1f);
		//ico->curVelocity = glm::vec3(-1.0f, 0.0f, 0.0f);
		//tetra->curVelocity = glm::vec3(1.0f, 0.0f, 0.0f);

		ico->angularVelocityAxis = glm::vec3(1.0f, 1.0f, 1.0f);
		//ico->angularVelocity = .1f;
		ico->angularVelocityAxis = glm::normalize(ico->angularVelocityAxis);
		ico->angularVelocityAxis *= .1f;

		tetra->angularVelocityAxis = glm::vec3(1.0f, 1.0f, -1.0f);
		//tetra->angularVelocity = .1f;
		tetra->angularVelocityAxis = glm::normalize(tetra->angularVelocityAxis);
		tetra->angularVelocityAxis *= .1f;

		ico->mass = 3.0f;
		tetra->mass = 1.0f;

		//Disallows user interaction: ... ? TODO 
		shapes.push_back(ico);
		shapes.push_back(tetra);

		icoMode = true;
		newData = true;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddCubeOnClick() {
		Cube *cube = new Cube();
		cube->InitVAOandVBO(shapeShader);
		PositionShapeToAdd(cube);
		shapes.push_back(cube);
		static int cubeNum = 1;
		cube->name = std::string("Cube" + std::to_string(cubeNum++));
		
		newData = true;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddSuperquadricOnClick() {
		//std::cout << "Clicked Add Superquadric" << std::endl;
		Superquadric *superquad = new Superquadric();
		superquad->a1 = 1.0f;
		superquad->a2 = 1.0f;
		superquad->a3 = 1.0f;
		superquad->e1 = 1.2f;
		superquad->e2 = 1.2f;  // .3f  1.0f  2.0f  3.0f
		superquad->u1 = -glm::pi<float>() / 2.0f;
		superquad->u2 = glm::pi<float>() / 2.0f;
		superquad->v1 = -glm::pi<float>();
		superquad->v2 = glm::pi<float>();
		superquad->u_segs = 23;
		superquad->v_segs = 23;
		static int superquadNum = 1;
		superquad->setShapeCSVcode(3);
		superquad->name = std::string("Superquadric" + std::to_string(superquadNum++));

		AddSuperquadric(superquad);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AddEllipsoidOnClick() {
		//std::cout << "Clicked Add Ellipsoid" << std::endl;
		Superquadric *superquad = new Superquadric();
		superquad->a1 = 1.0f;
		superquad->a2 = 1.25f;
		superquad->a3 = 1.0f;
		superquad->e1 = 1.0f;
		superquad->e2 = 1.0f;  // .3f  1.0f  2.0f  3.0f
		superquad->u1 = -glm::pi<float>() / 2.0f;
		superquad->u2 = glm::pi<float>() / 2.0f;
		superquad->v1 = -glm::pi<float>();
		superquad->v2 = glm::pi<float>();
		superquad->u_segs = 23;
		superquad->v_segs = 23;
		static int ellipsoidNum = 1;
		superquad->name = std::string("Ellipsoid" + std::to_string(ellipsoidNum++));
		superquad->setShapeCSVcode(2);
		AddSuperquadric(superquad);
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		uploadMesh(char* meshName, char* meshData);

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		uploadMeshName(char* filename);


	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		TranslateOnClick() {
		if (!USING_EMSCRIPTEN) ResetCursorState();
		cursorState = translate;
		//std::cout << "Cursor state: translate" << std::endl;
		if (!USING_EMSCRIPTEN) sidePanelStr.positionBtn->SetToggledColor();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		RotateOnClick() {
		if (!USING_EMSCRIPTEN) ResetCursorState();
		cursorState = rotate;
		//std::cout << "Cursor state: rotate" << std::endl;
		if (!USING_EMSCRIPTEN) sidePanelStr.rotateBtn->SetToggledColor();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		ScaleOnClick() {
		if (!USING_EMSCRIPTEN) ResetCursorState();
		cursorState = scale_uniform;
		//std::cout << "Cursor state: scale" << std::endl;
		if (!USING_EMSCRIPTEN) sidePanelStr.scaleUniformBtn->SetToggledColor();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		VelocityOnClick() {
		if (!USING_EMSCRIPTEN) ResetCursorState();
		cursorState = velocity;
		//std::cout << "Cursor state: velocity" << std::endl;
		if (!USING_EMSCRIPTEN) sidePanelStr.velocityBtn->SetToggledColor();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		AngularVelocityOnClick() {
		if (!USING_EMSCRIPTEN) ResetCursorState();
		cursorState = angular_velocity;
		//std::cout << "Cursor state: angular velocity" << std::endl;
		if (!USING_EMSCRIPTEN) sidePanelStr.angularVelocityBtn->SetToggledColor();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setCameraStatePan() {
		camera.setPan();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setCameraStateRotate() {
		camera.setTilt();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		setCameraStateZoom() {
		camera.setZoom();
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		duplicateShape(unsigned int shapeIndex);

	//Callback functions for impulse graph:
//	void
//#ifdef EMSCRIPTEN_KEEPALIVE 
//		EMSCRIPTEN_KEEPALIVE
//#endif
//		toggleImpulseBtnOnClick() {
//		graph->toggleImpulseBtnOnClick();
//	}
//
//
//	void
//#ifdef EMSCRIPTEN_KEEPALIVE 
//		EMSCRIPTEN_KEEPALIVE
//#endif
//		toggleVelocityBtnOnClick() {
//		graph->toggleVelocityBtnOnClick();
//	}

	/*bool restartImpulse = false;
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		restartImpulseOnClick() {
		graph->restartImpulseOnClick();
	}
	bool continueImpulse = false;
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		continueImpulseOnClick() {
		graph->continueImpulseOnClick();
	}*/

	/*void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		showXZPlaneOnClick() {
		graph->showXZPlaneOnClick();
	}
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		showXYPlaneOnClick() {
		graph->showXYPlaneOnClick();
	}
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		showYZPlaneOnClick() {
		graph->showYZPlaneOnClick();
	}
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		lookDownXaxisOnClick() {
		graph->lookDownXaxisOnClick();
	}
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		lookDownYaxisOnClick() {
		graph->lookDownYaxisOnClick();
	}
	void
#ifdef EMSCRIPTEN_KEEPALIVE
		EMSCRIPTEN_KEEPALIVE
#endif
		lookDownZaxisOnClick() {
		graph->lookDownZaxisOnClick();
	}*/

	void
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		continueImpulseOnClick() {
		impulseMode = false;
		contactShape1->objectColor = contactShape1->defaultColor;
		contactShape2->objectColor = contactShape2->defaultColor;

		camera.cameraPos.x = prevCameraPosX;
		camera.cameraPos.y = prevCameraPosY;
		camera.cameraTarget.x = prevCameraTargetX;
		camera.cameraTarget.y = prevCameraTargetY;
		camera.calcView();

		newData = true;
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		generateGraphs(bool shouldGenerateGraphs) {
		usingClosedFormImpact = !shouldGenerateGraphs;

		//IF graphs aren't desired by user, use the closed form 
		// impact code instead of numerical integration all the way
		// since it is faster and won't wasted memory
	}

	void
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		use2DShapes(bool shouldUse2D) {
			using2DShapes = shouldUse2D;
			ResetOnClick();
		}



	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV1endX() {
		return previousImpactResults.v1end.x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV1endY() {
		return previousImpactResults.v1end.y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV1endZ() {
		return previousImpactResults.v1end.z;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV2endX() {
		return previousImpactResults.v2end.x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV2endY() {
		return previousImpactResults.v2end.y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsV2endZ() {
		return previousImpactResults.v2end.z;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW1endX() {
		return previousImpactResults.w1end.x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW1endY() {
		return previousImpactResults.w1end.y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW1endZ() {
		return previousImpactResults.w1end.z;
	}

	float 
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW2endX() {
		return previousImpactResults.w2end.x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW2endY() {
		return previousImpactResults.w2end.y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsW2endZ() {
		return previousImpactResults.w2end.z;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsIendX() {
		return previousImpactResults.Iend.x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsIendY() {
		return previousImpactResults.Iend.y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsIendZ() {
		return previousImpactResults.Iend.z;
	}

	int
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsEndOfSliding() {
		return previousImpactResults.endOfSliding;
	}

	int
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getImpactResultsEndOfCompression() {
		return previousImpactResults.endOfCompression;
	}

/*
For the following functions, to use on the client:
while(isMoreXXpts()){
	x = getNextXXPtX()
	y = ''Y()
	z = ''Z()
}

*/

	bool
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		isMoreImpulsePts() {
		return curImpulsePt < previousImpactResults.impulsePts.size() - 1;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextImpulsePtX() {
		return previousImpactResults.impulsePts[curImpulsePt].x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextImpulsePtY() {
		return previousImpactResults.impulsePts[curImpulsePt].y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextImpulsePtZ() {
		return previousImpactResults.impulsePts[curImpulsePt++].z;
	}

	bool
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		isMoreVelocityPts() {
		return curVelocityPt < previousImpactResults.velocityPts.size() - 1;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextVelocityPtX() {
		return previousImpactResults.velocityPts[curVelocityPt].x;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextVelocityPtY() {
		return previousImpactResults.velocityPts[curVelocityPt].y;
	}

	float
#ifdef EMSCRIPTEN_KEEPALIVE 
		EMSCRIPTEN_KEEPALIVE
#endif
		getNextVelocityPtZ() {
		return previousImpactResults.velocityPts[curVelocityPt++].z;
	}
}

using Eigen::MatrixXd;

void InitLeftPanel() {

	float buttonWidth = .325f;

	sidePanelStr.panel = new GLPanel(.6f, 1.4f, GLPanel::vertical, .05f, .0125f, .0125f);
	sidePanelStr.panel->Translate(glm::vec3(-1.0f, -.4f, 0.0f));
	sidePanelStr.panel->SetColor(glm::vec4(.1f, .1f, .1f, 1.0f));

	sidePanelStr.sphereBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.sphereBtn->SetColor(glm::vec4(.2f, .2f, .8f, 1.0f));
	sidePanelStr.sphereBtn->SetClickListener(AddSphereOnClick);
	sidePanelStr.sphereBtn->SetText("+ Sphere");
	sidePanelStr.sphereBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.cubeBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.cubeBtn->SetColor(glm::vec4(.2f, .2f, .8f, 1.0f));
	sidePanelStr.cubeBtn->SetClickListener(AddCubeOnClick);
	sidePanelStr.cubeBtn->SetText("+ Cube");
	sidePanelStr.cubeBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.ellipsoidBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.ellipsoidBtn->SetColor(glm::vec4(.2f, .2f, .8f, 1.0f));
	sidePanelStr.ellipsoidBtn->SetClickListener(AddEllipsoidOnClick);
	sidePanelStr.ellipsoidBtn->SetText("+ Ellipsoid");
	sidePanelStr.ellipsoidBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.superquadricBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.superquadricBtn->SetColor(glm::vec4(.2f, .2f, .8f, 1.0f));
	sidePanelStr.superquadricBtn->SetClickListener(AddSuperquadricOnClick);
	sidePanelStr.superquadricBtn->SetText("+ Superquad");
	sidePanelStr.superquadricBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.icosahedronBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.icosahedronBtn->SetColor(glm::vec4(.2f, .2f, .8f, 1.0f));
	sidePanelStr.icosahedronBtn->SetClickListener(AddIcosahedronOnClick);
	sidePanelStr.icosahedronBtn->SetText("+ Icosahedron");
	sidePanelStr.icosahedronBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.runBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.runBtn->SetColor(glm::vec4(.4f, .9f, .4f, 1.0f));
	sidePanelStr.runBtn->SetClickListener(RunOnClick);
	sidePanelStr.runBtn->SetText("Run");

	sidePanelStr.resetBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.resetBtn->SetColor(glm::vec4(.7f, .3f, .3f, 1.0f));
	sidePanelStr.resetBtn->SetClickListener(ResetOnClick);
	sidePanelStr.resetBtn->SetText("Reset");
	sidePanelStr.resetBtn->SetTextColor(glm::vec3(1.0f, 1.0f, 1.0f));

	sidePanelStr.positionBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.positionBtn->SetColor(glm::vec4(.9f, .1f, .1f, 1.0f));
	sidePanelStr.positionBtn->SetClickListener(TranslateOnClick);
	sidePanelStr.positionBtn->SetText("Translate");
	sidePanelStr.positionBtn->SetToggledColor();
	cursorState = translate;

	sidePanelStr.rotateBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.rotateBtn->SetColor(glm::vec4(.9f, .1f, .1f, 1.0f));
	sidePanelStr.rotateBtn->SetClickListener(RotateOnClick);
	sidePanelStr.rotateBtn->SetText("Rotate");
	sidePanelStr.rotateBtn->UnsetToggledColor();

	sidePanelStr.scaleUniformBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.scaleUniformBtn->SetColor(glm::vec4(.9f, .1f, .1f, 1.0f));
	sidePanelStr.scaleUniformBtn->SetClickListener(ScaleOnClick);
	sidePanelStr.scaleUniformBtn->SetText("Scale Uniform");
	sidePanelStr.scaleUniformBtn->UnsetToggledColor();

	sidePanelStr.velocityBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.velocityBtn->SetColor(glm::vec4(.9f, .1f, .1f, 1.0f));
	sidePanelStr.velocityBtn->SetClickListener(VelocityOnClick);
	sidePanelStr.velocityBtn->SetText("Velocity");
	sidePanelStr.velocityBtn->UnsetToggledColor();

	sidePanelStr.angularVelocityBtn = new GLButton(buttonWidth, .1f);
	sidePanelStr.angularVelocityBtn->SetColor(glm::vec4(.9f, .1f, .1f, 1.0f));
	sidePanelStr.angularVelocityBtn->SetClickListener(AngularVelocityOnClick);
	sidePanelStr.angularVelocityBtn->SetText("Angular Velocity");
	sidePanelStr.angularVelocityBtn->UnsetToggledColor();

	sidePanelStr.panel->AddView(sidePanelStr.sphereBtn);
	sidePanelStr.panel->AddView(sidePanelStr.cubeBtn);
	sidePanelStr.panel->AddView(sidePanelStr.ellipsoidBtn);
	sidePanelStr.panel->AddView(sidePanelStr.superquadricBtn);
	sidePanelStr.panel->AddView(sidePanelStr.icosahedronBtn);
	sidePanelStr.panel->AddView(sidePanelStr.runBtn);
	sidePanelStr.panel->AddView(sidePanelStr.resetBtn);
	sidePanelStr.panel->AddView(sidePanelStr.positionBtn);
	sidePanelStr.panel->AddView(sidePanelStr.rotateBtn);
	sidePanelStr.panel->AddView(sidePanelStr.scaleUniformBtn);
	sidePanelStr.panel->AddView(sidePanelStr.velocityBtn);
	sidePanelStr.panel->AddView(sidePanelStr.angularVelocityBtn);

	views.push_back(sidePanelStr.panel);
}

bool textboxSelected;
GLTextBox *textbox;

struct BottomPanel {
	GLPanel *panel;

	GLPanel *positionPanel;
	GLLabel *positionLabelX;
	GLTextBox *positionTextX;
	GLLabel *positionLabelY;
	GLTextBox *positionTextY;
	GLLabel *positionLabelZ;
	GLTextBox *positionTextZ;

	GLPanel *velocityPanel;
	GLLabel *velocityLabelX;
	GLTextBox *velocityTextX;
	GLLabel *velocityLabelY;
	GLTextBox *velocityTextY;
	GLLabel *velocityLabelZ;
	GLTextBox *velocityTextZ;

	GLPanel *scalePanel;
	GLLabel *scaleLabelX;
	GLTextBox *scaleTextX;
	GLLabel *scaleLabelY;
	GLTextBox *scaleTextY;
	GLLabel *scaleLabelZ;
	GLTextBox *scaleTextZ;

	GLPanel *angularVelocityPanel;
	GLLabel *angularVelocityLabelX;
	GLTextBox *angularVelocityTextX;
	GLLabel *angularVelocityLabelY;
	GLTextBox *angularVelocityTextY;
	GLLabel *angularVelocityLabelZ;
	GLTextBox *angularVelocityTextZ;
	GLLabel *angularVelocitySpeedLabel;
	GLTextBox *angularVelocitySpeedText;

} bottomPanelStr;

class PositionXonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->translation = pickedShape->centroid = glm::vec3(std::stof(bottomPanelStr.positionTextX->GetText()), pickedShape->translation.y, pickedShape->translation.z);
	}
};

class PositionYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->translation = pickedShape->centroid = glm::vec3(pickedShape->translation.x, std::stof(bottomPanelStr.positionTextY->GetText()), pickedShape->translation.z);
	}
};

class PositionZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->translation = pickedShape->centroid = glm::vec3(pickedShape->translation.x, pickedShape->translation.y, std::stof(bottomPanelStr.positionTextZ->GetText()));
	}
};

class ScaleXonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->scaling = std::stof(bottomPanelStr.scaleTextX->GetText());
	}
};

class ScaleYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->scaling = std::stof(bottomPanelStr.scaleTextY->GetText());
	}
};

class ScaleZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->scaling = std::stof(bottomPanelStr.scaleTextZ->GetText());
	}
};

class VelocityXonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->curVelocity = glm::vec3(std::stof(bottomPanelStr.velocityTextX->GetText()), pickedShape->curVelocity.y, pickedShape->curVelocity.z);
	}
};

class VelocityYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->curVelocity = glm::vec3(pickedShape->curVelocity.x, std::stof(bottomPanelStr.velocityTextY->GetText()), pickedShape->curVelocity.z);
	}
};

class VelocityZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->curVelocity = glm::vec3(pickedShape->curVelocity.x, pickedShape->curVelocity.y, std::stof(bottomPanelStr.velocityTextZ->GetText()));
	}
};

class AngularVelocityXonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->angularVelocityAxis.x = std::stof(bottomPanelStr.angularVelocityTextX->GetText());
	}
};

class AngularVelocityYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->angularVelocityAxis.y = std::stof(bottomPanelStr.angularVelocityTextY->GetText());
	}
};

class AngularVelocityZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->angularVelocityAxis.z = std::stof(bottomPanelStr.angularVelocityTextZ->GetText());
	}
};

//class AngularVelocitySpeedonChange : public GLTextBox::TextChangedListener {
//	void OnChange() {
//		pickedShape->angularVelocity = std::stof(bottomPanelStr.angularVelocitySpeedText->GetText());
//	}
//};

void InitAnimationControls() {
	//Pause/stop buttons
	pauseBtn = new GLView(.2f, .1f);
	pauseBtn->Translate(glm::vec3(-.95f, .85f, 0.0f));
	pauseBtn->SetColor(glm::vec4(.8f, .8f, .8f, 1.0f));
	pauseBtn->SetText("Pause");
	pauseBtn->SetClickListener(PauseOnClick);
	pauseBtn->InitBuffers();

	stopBtn = new GLView(.2f, .1f);
	stopBtn->Translate(glm::vec3(-.95f, .65f, 0.0f));
	stopBtn->SetColor(glm::vec4(.8f, .8f, .8f, 1.0f));
	stopBtn->SetText("Stop");
	stopBtn->SetClickListener(StopOnClick);
	stopBtn->InitBuffers();
}

GLView *focusedView = nullptr;

void InitBottomPanel() {
	bottomPanel = new GLPanel(2.0f, .6f, GLPanel::vertical, .025f, .045f, .00f);
	bottomPanel->Translate(glm::vec3(-1.0f, -1.0f, 0.0f));
	bottomPanel->SetColor(glm::vec4(.2f, .2f, .2f, 1.0f));
	bottomPanel->HideContent();

	bottomPanelStr.panel = bottomPanel;

	bottomPanelStr.positionPanel = new GLPanel(1.1f, .14f, GLPanel::horizontal, .0125f, .000025f, .00125f);
	bottomPanelStr.panel->AddView(bottomPanelStr.positionPanel);

	bottomPanelStr.positionLabelX = new GLLabel(.22f, .1f);
	bottomPanelStr.positionLabelX->SetText("Position X:");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionLabelX);

	bottomPanelStr.positionTextX = new GLTextBox(.2f, .1f);
	bottomPanelStr.positionTextX->SetText("0");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionTextX);
	bottomPanelStr.positionTextX->SetTextChangedListener(new PositionXonChange());

	bottomPanelStr.positionLabelY = new GLLabel(.075f, .1f);
	bottomPanelStr.positionLabelY->SetText("Y:");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionLabelY);

	bottomPanelStr.positionTextY = new GLTextBox(.2f, .1f);
	bottomPanelStr.positionTextY->SetText("0");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionTextY);
	bottomPanelStr.positionTextY->SetTextChangedListener(new PositionYonChange());

	bottomPanelStr.positionLabelZ = new GLLabel(.075f, .1f);
	bottomPanelStr.positionLabelZ->SetText("Z:");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionLabelZ);

	bottomPanelStr.positionTextZ = new GLTextBox(.2f, .1f);
	bottomPanelStr.positionTextZ->SetText("0");
	bottomPanelStr.positionPanel->AddView(bottomPanelStr.positionTextZ);
	bottomPanelStr.positionTextZ->SetTextChangedListener(new PositionZonChange());


	bottomPanelStr.velocityPanel = new GLPanel(1.1f, .14f, GLPanel::horizontal, .0125f, .000025f, .00125f);
	bottomPanelStr.panel->AddView(bottomPanelStr.velocityPanel);

	bottomPanelStr.velocityLabelX = new GLLabel(.22f, .1f);
	bottomPanelStr.velocityLabelX->SetText("Velocity X:");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityLabelX);

	bottomPanelStr.velocityTextX = new GLTextBox(.2f, .1f);
	bottomPanelStr.velocityTextX->SetText("0");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityTextX);
	bottomPanelStr.velocityTextX->SetTextChangedListener(new VelocityXonChange());

	bottomPanelStr.velocityLabelY = new GLLabel(.075f, .1f);
	bottomPanelStr.velocityLabelY->SetText("Y:");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityLabelY);

	bottomPanelStr.velocityTextY = new GLTextBox(.2f, .1f);
	bottomPanelStr.velocityTextY->SetText("0");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityTextY);
	bottomPanelStr.velocityTextY->SetTextChangedListener(new VelocityYonChange());

	bottomPanelStr.velocityLabelZ = new GLLabel(.075f, .1f);
	bottomPanelStr.velocityLabelZ->SetText("Z:");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityLabelZ);

	bottomPanelStr.velocityTextZ = new GLTextBox(.2f, .1f);
	bottomPanelStr.velocityTextZ->SetText("0");
	bottomPanelStr.velocityPanel->AddView(bottomPanelStr.velocityTextZ);
	bottomPanelStr.velocityTextZ->SetTextChangedListener(new VelocityZonChange());


	bottomPanelStr.scalePanel = new GLPanel(1.1f, .14f, GLPanel::horizontal, .0125f, .000025f, .00125f);
	bottomPanelStr.panel->AddView(bottomPanelStr.scalePanel);

	bottomPanelStr.scaleLabelX = new GLLabel(.22f, .1f);
	bottomPanelStr.scaleLabelX->SetText("Scale X:");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleLabelX);

	bottomPanelStr.scaleTextX = new GLTextBox(.2f, .1f);
	bottomPanelStr.scaleTextX->SetText("0");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleTextX);
	bottomPanelStr.scaleTextX->SetTextChangedListener(new ScaleXonChange());

	bottomPanelStr.scaleLabelY = new GLLabel(.075f, .1f);
	bottomPanelStr.scaleLabelY->SetText("Y:");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleLabelY);

	bottomPanelStr.scaleTextY = new GLTextBox(.2f, .1f);
	bottomPanelStr.scaleTextY->SetText("0");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleTextY);
	bottomPanelStr.scaleTextY->SetTextChangedListener(new ScaleYonChange());

	bottomPanelStr.scaleLabelZ = new GLLabel(.075f, .1f);
	bottomPanelStr.scaleLabelZ->SetText("Z:");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleLabelZ);

	bottomPanelStr.scaleTextZ = new GLTextBox(.2f, .1f);
	bottomPanelStr.scaleTextZ->SetText("0");
	bottomPanelStr.scalePanel->AddView(bottomPanelStr.scaleTextZ);
	bottomPanelStr.scaleTextZ->SetTextChangedListener(new ScaleZonChange());


	bottomPanelStr.angularVelocityPanel = new GLPanel(1.75f, .14f, GLPanel::horizontal, .0125f, .000025f, .00125f);
	bottomPanelStr.panel->AddView(bottomPanelStr.angularVelocityPanel);

	bottomPanelStr.angularVelocityLabelX = new GLLabel(.22f, .1f);
	bottomPanelStr.angularVelocityLabelX->SetText("Rot. Axis X:");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityLabelX);

	bottomPanelStr.angularVelocityTextX = new GLTextBox(.2f, .1f);
	bottomPanelStr.angularVelocityTextX->SetText("0");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityTextX);
	bottomPanelStr.angularVelocityTextX->SetTextChangedListener(new AngularVelocityXonChange());

	bottomPanelStr.angularVelocityLabelY = new GLLabel(.075f, .1f);
	bottomPanelStr.angularVelocityLabelY->SetText("Y:");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityLabelY);

	bottomPanelStr.angularVelocityTextY = new GLTextBox(.2f, .1f);
	bottomPanelStr.angularVelocityTextY->SetText("0");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityTextY);
	bottomPanelStr.angularVelocityTextY->SetTextChangedListener(new AngularVelocityYonChange());

	bottomPanelStr.angularVelocityLabelZ = new GLLabel(.075f, .1f);
	bottomPanelStr.angularVelocityLabelZ->SetText("Z:");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityLabelZ);

	bottomPanelStr.angularVelocityTextZ = new GLTextBox(.2f, .1f);
	bottomPanelStr.angularVelocityTextZ->SetText("0");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocityTextZ);
	bottomPanelStr.angularVelocityTextZ->SetTextChangedListener(new AngularVelocityZonChange());

	/*bottomPanelStr.angularVelocitySpeedLabel = new GLLabel(.22f, .1f);
	bottomPanelStr.angularVelocitySpeedLabel->SetText("Rot. Speed:");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocitySpeedLabel);

	bottomPanelStr.angularVelocitySpeedText = new GLTextBox(.2f, .1f);
	bottomPanelStr.angularVelocitySpeedText->SetText("0");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocitySpeedText);
	bottomPanelStr.angularVelocitySpeedText->SetTextChangedListener(new AngularVelocitySpeedonChange());*/

	views.push_back(bottomPanel);
}

#include <Eigen/Dense>
using namespace Eigen;

float gravity = 0.0f;// -9.8f;

float timeUpdate = 
#ifdef EMSCRIPTEN_KEEPALIVE
.01f;
#endif
#ifndef EMSCRIPTEN_KEEPALIVE
.001f;
#endif

int numFramesToUpdatePerCollision = 1;

bool go = false;

const int GOAL_FPS = 60;

bool testing = true;

void mainLoop() {
	// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
	glfwPollEvents();
	//if (testing) {
	//	uploadMesh("ok");
	//	testing = !testing;
	//}

	if (glfwWindowShouldClose(window)) {
		Close();

		if (USING_EMSCRIPTEN) {
			emscripten_force_exit(0);
		}
		closeProgram = true;
		return;
	}

	//Uncomment for wireframe mode:
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	float timePerFrame = 1.0f / (float)GOAL_FPS;
	GLfloat currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	if (deltaTime < timePerFrame) {
		return; 
	}

	lastFrame = currentFrame;

	//if (impulseMode) {
		//renderImpulseGraph();
		//TODO focus camera on shapes in contact
		//return;
	//}

	if (!setup) {
		if (!go) {
			for (unsigned int i = 0; i < shapes.size(); i++) {
				shapes[i]->time = 0.0f;
			}
			go = true;
		}


		glm::mat4 PV = projection * camera.view;

		//Update position of each shape:
		for (unsigned int i = 0; i < shapes.size(); i++) {

			if (!paused && !impulseMode) {
				if (reverse) {
					timeUpdate = -timeUpdate;
				}
				shapes[i]->time += timeUpdate;

				//float rotationFraction = .0005f;
				if (!MathUtils::isZeroVec(shapes[i]->angularVelocityAxis)) {
					float speed = MathUtils::magnitude(shapes[i]->angularVelocityAxis);
					glm::vec3 axis = glm::normalize(shapes[i]->angularVelocityAxis);
					shapes[i]->applyRotation(axis, speed * timePerFrame);// *rotationFraction);
				}
				shapes[i]->translation += shapes[i]->curVelocity * deltaTime;
			}

			if (!ShapeUtils::checkIfInFrustrum(PV, shapes[i]->translation)) {
				//std::cout << "Shape went out of scene" << std::endl;
				shapeOutOfScene = true; //UI will query this each frame, stopping and resetting the UI if true
			}

			//shapes[i]->translation = shapes[i]->centroid + (shapes[i]->curVelocity * shapes[i]->time) + glm::vec3(0.0f, .5f * gravity * shapes[i]->time * shapes[i]->time, 0.0f);
			
		}
		newData = true;
	}

	//TODO predict sphere tests
	//Find closest points/collisions
	if (!setup && !impulseMode) {
		unsigned int iStop = shapes.size() - 1;
		for (unsigned int i = 0; i < iStop; i++) {
			for (unsigned int j = i + 1; j < shapes.size(); j++) {

				std::pair<int, int> curPair(i, j);
				if (framesSinceCollision.find(curPair) != framesSinceCollision.end()) {
					if (framesSinceCollision[curPair] > 0) {
						framesSinceCollision[curPair]--;
						continue;
					}
				}

				ParamPoint ppI, ppJ;
				if (checkForCollision(i, j, ppI, ppJ)) {
					HandleCollision(i, j, ppI, ppJ);
				}
				
			}
		}
	}
	if (impulseMode && !cameraTransDone) {
		if (cameraTranslations < numCameraTranslations) {
			camera.cameraPos += cameraTranslation;
			camera.cameraTarget += cameraTranslation;
			camera.calcView();
			cameraTranslations++;
			//std::cout << "Camera pos: " << camera.cameraPos.x << ", " << camera.cameraPos.y << ", " << camera.cameraPos.z << std::endl;
			//std::cout << "Camera target: " << camera.cameraPos.x << ", " << camera.cameraPos.y << ", " << camera.cameraPos.z << std::endl;
		}
		else {
			cameraTransDone = true;
			if (!USING_EMSCRIPTEN) {
				continueImpulseOnClick();
			}
		}
	}

	Render();
	return;
}

void HandleCollision(int i, int j, ParamPoint& ppI, ParamPoint &ppJ) {
	//Change velocity, reset time, and update initial point
	Collision(*shapes[i], ppI, *shapes[j], ppJ, i, j);

	std::pair<int, int> curPair(i, j);
	framesSinceCollision[curPair] = numFramesToUpdatePerCollision;

	shapes[i]->time = shapes[j]->time = 0.0f;

	shapes[i]->centroid = shapes[i]->translation;
	shapes[j]->centroid = shapes[j]->translation;
}

int main()
{

	InitOpenGL();

	inverseProjection = glm::inverse(projection);

	if (!USING_EMSCRIPTEN) {
		InitSetupSceneTransform();
		InitLeftPanel();
		InitBottomPanel();
		InitAnimationControls();
		InitViews();
	}

	if (!USING_EMSCRIPTEN) {
		generateGraphs(true);
		while (!closeProgram)
			mainLoop();

		return 0;
	}
	else {
		emscripten_set_main_loop(mainLoop, 0, 1);
	}
}


bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
	float dotab = glm::dot(a, b);
	float dotac = glm::dot(a, c);
	float dotbc = glm::dot(b, c);
	return true;
}

bool arrows[4] = { false, false, false, false };

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	bool down = (action == 1);

	//Stop if escape key pressed
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	//Handle pause
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		paused = true;
	}
	else if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
		paused = false;
	}

	//Handle reverse time:
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		reverse = true;
	}
	else if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
		reverse = false;
	}

	if (action == GLFW_RELEASE) {
		Keyboard::Input(key);
	}

	//Handle arrow keys for shape controlling:
	if (pickedShape != nullptr) {

		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			arrows[0] = true;
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + glm::vec3(-.1f, 0.0f, 0.0f);
		}
		else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
			arrows[0] = false;
		}

		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
			arrows[1] = true;
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + glm::vec3(.1f, 0.0f, 0.0f);
		}
		else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
			arrows[1] = false;
		}

		if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
			arrows[2] = true;
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + glm::vec3(0.0f, .1f, 0.0f);
		}
		else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
			arrows[2] = false;
		}

		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
			arrows[3] = true;
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + glm::vec3(0.0f, -.1f, 0.0f);
		}
		else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
			arrows[3] = true;
		}
	}

}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset) {
	float xoffset = (float)x_offset;
	float yoffset = (float)y_offset;
	float clampMax = 5.0f;
	float clampMin = -5.0f;
	
	xoffset = MathUtils::clamp(xoffset, clampMin, clampMax);
	yoffset = MathUtils::clamp(yoffset, clampMin, clampMax);

	glm::vec3 update = glm::vec3(0.0f, 0.0f, -.1f * yoffset);
	if (pickedShape != nullptr && !icoMode) {
		newData = true;
		if (cursorState == translate && !using2DShapes) {
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + update;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.positionTextX->SetText(std::to_string(pickedShape->translation.x));
				bottomPanelStr.positionTextY->SetText(std::to_string(pickedShape->translation.y));
				bottomPanelStr.positionTextZ->SetText(std::to_string(pickedShape->translation.z));
			}
		}
		else if (cursorState == rotate) {

		}
		else if (cursorState == scale_uniform) {
			float previousScaling = pickedShape->scaling;

			pickedShape->scaling += (yoffset * .05f);

			float newScaling = pickedShape->scaling;

			float proportionalUpdate = newScaling / previousScaling;
			pickedShape->boundingSphereRadius *= proportionalUpdate;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.scaleTextX->SetText(std::to_string(pickedShape->scaling));
				bottomPanelStr.scaleTextY->SetText(std::to_string(pickedShape->scaling));
				bottomPanelStr.scaleTextZ->SetText(std::to_string(pickedShape->scaling));
			}
		}
		else if (cursorState == velocity && !using2DShapes) {
			pickedShape->curVelocity = pickedShape->curVelocity + update;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.velocityTextX->SetText(std::to_string(pickedShape->curVelocity.x));
				bottomPanelStr.velocityTextY->SetText(std::to_string(pickedShape->curVelocity.y));
				bottomPanelStr.velocityTextZ->SetText(std::to_string(pickedShape->curVelocity.z));
			}
		}
		else if (cursorState == angular_velocity && !using2DShapes) {
			pickedShape->angularVelocityAxis += update;
			//pickedShape->angularVelocity = sqrtf(pickedShape->angularVelocityAxis.x * pickedShape->angularVelocityAxis.x + pickedShape->angularVelocityAxis.y * pickedShape->angularVelocityAxis.y + pickedShape->angularVelocityAxis.z * pickedShape->angularVelocityAxis.z);
			
			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->angularVelocityAxis.x));
				bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->angularVelocityAxis.y));
				bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->angularVelocityAxis.z));
			}
		}
	}

	else {
		glm::vec3 newPos = camera.cameraPos + update;
		if(!using2DShapes || newPos.z > 0.0f) //Makes sure camera doesn't go in front of scene if 2D
			camera.handleScroll(update);
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	float xPos = (float)xpos;
	float yPos = (float)ypos;

	bool right = xPos > mouseXvp;
	bool left = xPos < mouseXvp;

	GLfloat xoffset = xPos - mouseXvp;
	GLfloat yoffset = mouseYvp - yPos;
	GLfloat sensitivity = 0.005f;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	mouseXvp = xPos;
	mouseYvp = yPos;

	mouseX = xPos / WIDTH;
	mouseX = ((mouseX - .5f) / .5f);

	mouseY = yPos / HEIGHT;
	mouseY = ((mouseY - .5f) / -.5f);

	//If mouse button is held down, there is a selected shape, and the shapes are not the ico/tetra:
	if (mouseDown[0] && pickedShape != nullptr && !icoMode) { 
		newData = true;

		if (cursorState == translate) {
			//glm::vec3 update(4.0f * xoffset / WIDTH, 4.0f * yoffset / HEIGHT, 0.0f);

			//std::cout << update.x << update.y << std::endl;

			glm::vec3 update(xoffset, yoffset, 0.0f);
			bool slideUpdate = false;

			for (unsigned int i = 0; i < shapes.size(); i++) {
				Shape *other = shapes[i];
				if (pickedShape != other && BoundingSphereTest(*pickedShape, *other)) {

					/*if (using2DShapes) {
						Shape2D *s1_2D = (Shape2D*)pickedShape;
						Shape2D *s2_2D = (Shape2D*)other;

						if (checkForCollision(*s1_2D, *s2_2D) {
							if (glm::dot(update, s2_2D->translation - s1_2D->translation) > 0.0f) {
								s1_2D->objectColor = s1_2D->contactColor;
								s2_2D->objectColor = s2_2D->contactColor;
								slideUpdate = true;
							}
							else {
								s1_2D->objectColor = selectedColor;
								s2_2D->objectColor = s2_2D->defaultColor;
							}
						}
					}*/
					//SAT Shapes - TODO better way of determining this
					if ((shapes[i]->name.find("Tetra") != shapes[i]->name.npos
						|| shapes[i]->name.find("Ico") != shapes[i]->name.npos) 
						|| using2DShapes 
						|| pickedShape->name.find("ube") != pickedShape->name.npos
						|| other->name.find("ube") != other->name.npos) {

						ParamPoint pp1;
						ParamPoint pp2;
						if (checkForCollision(pickedShapeIndex, i, pp1, pp2)) {
							if (glm::dot(update, other->translation - pickedShape->translation) > 0.0f) {
								pickedShape->objectColor = pickedShape->contactColor;
								other->objectColor = other->contactColor;

								if (using2DShapes) {
									ShapeSeparatingAxis& otherSAT = (ShapeSeparatingAxis&) *other;
									ShapeSeparatingAxis& pickedSAT = (ShapeSeparatingAxis&)*pickedShape;
									CollisionDetector::SlideS2onS1(otherSAT, pickedSAT, pp1.pt, pp2.pt, update);
								}
								slideUpdate = true;
							}
							else {
								pickedShape->objectColor = selectedColor;
								other->objectColor = other->defaultColor;
							}
						}
					}

					else {

						Superquadric *picked = castToSuperquadric(pickedShape, "cursor_position_callback");
						Superquadric *otherSq = castToSuperquadric(other, "cursor_position_callback");

						glm::vec3 closest1;
						glm::vec3 closest2;
						ParamPoint contactPt1, contactPt2;

						Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);

						if (Superquadric::checkVecForNaN(contactPt1.pt) || Superquadric::checkVecForNaN(contactPt2.pt) ||
							isnan(contactPt1.u) || isnan(contactPt1.v) || isnan(contactPt2.u) || isnan(contactPt2.v)) {

							std::cout << "Collision detecting failed" << std::endl;
						}

						//TODO  use squared distance instead
						float distance = glm::distance(closest1, closest2);
						//Todo use consistent threshold
						float slideContactAccuracy = .015f;

						bool penetrated = glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f;
						if (distance < slideContactAccuracy || penetrated) {

							//User is dragging picked shape away from otherSq
							if (glm::dot(update, (otherSq->centroid - picked->centroid)) < 0.0f) {
								otherSq->objectColor = otherSq->defaultColor;
								break;
							}

							otherSq->objectColor = otherSq->contactColor;
							slideUpdate = true;

							//Shape is in contact with/intersecting another

							/*//Undo half of update:
							pickedShape->centroid = pickedShape->translation = pickedShape->centroid - .5f * update;*/

							//Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);
							//float distance = glm::distance(closest1, closest2);

							//if (distance > slideContactAccuracy) {
							//	//Not in contact anymore
							//	return;
							//}

							//Shape is still in contact with/intersecting another
							//Undo half of update:
							//pickedShape->centroid = pickedShape->translation = pickedShape->centroid - .5f * update;

							//Adjust update so its magnitude is .05
							float mag = MathUtils::magnitude(update);
							update = .05f * (update / mag);

							//THis will be new point of contact on otherSq's surface
							glm::vec3 slideUpdate = Superquadric::SlideUpdate(*otherSq, contactPt2, update);
							if (Superquadric::checkVecForNaN(slideUpdate)) {
 								///std::cout << "nan slide update" << std::endl;
								//slideUpdate = Superquadric::SlideUpdate(*otherSq, contactPt2, update);
								return;
							}
							if (MathUtils::abs(slideUpdate.x) > .45f || MathUtils::abs(slideUpdate.y) > .45f || MathUtils::abs(slideUpdate.z) > .45f) {
								///std::cout << "too big of slide update" << std::endl;
								//slideUpdate = Superquadric::SlideUpdate(*otherSq, contactPt2, update);
								return;
							}
							float slideDist = MathUtils::magnitude(slideUpdate);
							if (slideDist > .1f) {
								///std::cout << "Slide update too big. distance: " << slideDist << std::endl;
								//slideUpdate = Superquadric::SlideUpdate(*otherSq, contactPt2, update);
								//slideUpdate = .1f * glm::normalize(slideUpdate);
								return;
							}
							///std::cout << "Slide update distance: " << slideDist << std::endl;


							glm::vec3 slidePt2 = contactPt2.pt; // + slideUpdate;


							//contactPti.pt == GLOBAL PT 


							//This is the point on pickedShape with a normal parallel to otherSq's new contact pt's normal
							// (in global space)
							glm::vec3 pickedShapeNewContactPt = Superquadric::SlidingSurfaceUpdate(*picked, *otherSq, contactPt1, contactPt2);
							if (Superquadric::checkVecForNaN(pickedShapeNewContactPt)) {
								///std::cout << "nan slide pt" << std::endl;
								pickedShapeNewContactPt = Superquadric::SlidingSurfaceUpdate(*picked, *otherSq, contactPt1, contactPt2);
								return;
							}
							glm::mat4 otherRot = otherSq->getRotationMatrix();
							///glm::vec3 pickedShapeTrans = Superquadric::getGlobalCoordinates(slidePt2, otherSq->translation, otherRot) - pickedShapeNewContactPt;
							glm::vec3 pickedShapeTrans = slidePt2 - pickedShapeNewContactPt;

							///std::cout << "Original Move: " << update.x << ", " << update.y << ", " << update.z << std::endl;
							std::cout << "Slide update: " << slideUpdate.x << ", " << slideUpdate.y << ", " << slideUpdate.z << std::endl;
							std::cout << "Slide trans: " << pickedShapeTrans.x << ", " << pickedShapeTrans.y << ", " << pickedShapeTrans.z << std::endl << std::endl;


							pickedShape->centroid = pickedShape->translation = pickedShape->centroid + pickedShapeTrans;

							Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);
							distance = glm::distance(closest1, closest2);
							if (distance < 0.0f || glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f) {
								//pickedShape->centroid = pickedShape->translation = pickedShape->centroid - pickedShapeTrans;
								///std::cout << "slide update made shapes intersect" << std::endl;

								glm::vec3 penetration = closest2 - closest1;
								pickedShape->centroid = pickedShape->translation = pickedShape->centroid + penetration;
							}
						}
						else {
							//std::cout << "Good" << std::endl;
						}

					}
				}
			}

			if (!slideUpdate) {
				pickedShape->centroid = pickedShape->translation = pickedShape->centroid + update;
			}

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.positionTextX->SetText("" + std::to_string(pickedShape->translation.x));
				bottomPanelStr.positionTextY->SetText("" + std::to_string(pickedShape->translation.y));
				bottomPanelStr.positionTextZ->SetText("" + std::to_string(pickedShape->translation.z));
			}
		}
		else if (cursorState == rotate) {

			//MultiCollide::Quaternion orientation(pickedShape->rotation);
			float angle = .035f;
			glm::vec3 axis(-yoffset, xoffset, 0.0f);
			if (MathUtils::abs(xoffset) < .001f && MathUtils::abs(yoffset) < .001f) {
				return;
			}

			if (using2DShapes) {
				axis = glm::vec3(0.0f, 0.0f, 1.0f);

				std::cout << " ( " << xoffset << ", " << yoffset << " )" << std::endl;

				if (right) {
					angle *= -1.0f;
				}
				else if (!left) {
					return;
				}
			}

			MultiCollide::Quaternion update(angle, axis);

			/*glm::quat updateQ = glm::angleAxis(angle, axis);
			std::cout << "updateQ: " << updateQ.x << " " << updateQ.y << " " << updateQ.z << " " << updateQ.w << std::endl;*/

			bool inContact = false;

			for (unsigned int i = 0; i < shapes.size(); i++) {
				Shape *other = shapes[i];
				if (pickedShape != other && BoundingSphereTest(*pickedShape, *other)) {

					Superquadric *picked = castToSuperquadric(pickedShape, "cursor_position_callback");
					Superquadric *otherSq = castToSuperquadric(other, "cursor_position_callback");

					bool ROTATE_IN_CONTACT = false;
					if (picked && other && ROTATE_IN_CONTACT) {

						glm::vec3 closest1;
						glm::vec3 closest2;
						ParamPoint contactPt1, contactPt2;

						Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);

						//TODO  use squared distance instead
						float distance = glm::distance(closest1, closest2);
						//Todo use consistent threshold
						float slideContactAccuracy = .015f;
						if (distance < slideContactAccuracy || glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f) {

							//Shape is in contact with/intersecting another
							inContact = true;


							//This is the point on pickedShape with the same normal as the original contact pt on 
							// pickedShape after the new rotation, in its local space
							glm::vec3 pickedShapeNewContactPt = Superquadric::RotatingSurfaceUpdate(*picked, *otherSq, contactPt1, update);

							glm::vec3 contactPtGlobal = contactPt1.pt;

							//pickedShape->rotation = glm::rotate(i4, update.Angle(), update.Axis()) * pickedShape->rotation;
							pickedShape->applyRotation(update.Axis(), update.Angle());

							glm::mat4 pickedRot = pickedShape->getRotationMatrix();
							glm::vec3 newPtGlobal = ShapeUtils::getGlobalCoordinates(pickedShapeNewContactPt, pickedShape->translation, pickedRot, pickedShape->scaling);

							//Need to move the new contact pt so that it is where the original contact pt was
							glm::vec3 transUpdate = contactPtGlobal - newPtGlobal;

							std::cout << "transUpdate: " << transUpdate.x << ", " << transUpdate.y << " " << transUpdate.z << std::endl;

							pickedShape->translation += transUpdate;

							if (Superquadric::checkVecForNaN(pickedShapeNewContactPt)) {
								std::cout << "nan rotate pt" << std::endl;
								return;
							}


							//TODO recheck for more collisions
							/*Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);
							distance = glm::distance(closest1, closest2);
							if (distance < contactAccuracy || glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f) {
							pickedShape->centroid = pickedShape->translation = pickedShape->centroid - slideUpdate;
							}*/
						}
						else {
							//std::cout << "Good" << std::endl;
						}

					}
				}
			}

			if (!inContact) {
				glm::mat4 rotation = pickedShape->getRotationMatrix();
				
				//glm::vec3 axis = glm::vec3(-yoffset, xoffset, 0.0f);

				if (USING_EMSCRIPTEN){
					std::cout << "Before rotation update: " << std::endl;
					MathUtils::printMat(rotation);

					std::cout << "axis: " << axis.x << ", " << axis.y << ", " << axis.z << std::endl;
					std::cout << "angle " << angle << std::endl;
				}

				pickedShape->applyRotation(axis, angle);

				if (USING_EMSCRIPTEN) {
					std::cout << "After rotation update: " << std::endl;
					rotation = pickedShape->getRotationMatrix();
					MathUtils::printMat(rotation);
				}

			}
		}
		else if (cursorState == scale_uniform) {
			float previousScaling = pickedShape->scaling;

			pickedShape->scaling += yoffset;

			float newScaling = pickedShape->scaling;

			float proportionalUpdate = newScaling / previousScaling;
			pickedShape->boundingSphereRadius *= proportionalUpdate;

			std::cout << "BS radius: " << pickedShape->boundingSphereRadius << std::endl;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.scaleTextX->SetText("" + std::to_string(pickedShape->scaling));
				bottomPanelStr.scaleTextY->SetText("" + std::to_string(pickedShape->scaling));
				bottomPanelStr.scaleTextZ->SetText("" + std::to_string(pickedShape->scaling));
			}
		}
		else if (cursorState == velocity) {
			pickedShape->curVelocity += glm::vec3(xoffset, yoffset, 0.0f);
			//std::cout << pickedShape->curVelocity.x << ", " << pickedShape->curVelocity.y << ", " << pickedShape->curVelocity.z << std::endl;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.velocityTextX->SetText("" + std::to_string(pickedShape->curVelocity.x));
				bottomPanelStr.velocityTextY->SetText("" + std::to_string(pickedShape->curVelocity.y));
				bottomPanelStr.velocityTextZ->SetText("" + std::to_string(pickedShape->curVelocity.z));
			}
		}
		else if (cursorState == angular_velocity && !using2DShapes) {
			pickedShape->angularVelocityAxis += glm::vec3(xoffset, yoffset, 0.0f);
			//pickedShape->angularVelocity = sqrtf(pickedShape->angularVelocityAxis.x * pickedShape->angularVelocityAxis.x + pickedShape->angularVelocityAxis.y * pickedShape->angularVelocityAxis.y + pickedShape->angularVelocityAxis.z * pickedShape->angularVelocityAxis.z);
			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->angularVelocityAxis.x));
				bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->angularVelocityAxis.y));
				bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->angularVelocityAxis.z));
				//TODO update speed
			}
		}
	}
	else if (mouseDown[0]) {
		if (camera.cameraState != Camera::CameraState::tilt || !using2DShapes) {
			camera.handleMouseDrag(xoffset, yoffset);
		}
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouseDown[0] = false;
	}
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		mouseDown[1] = false;
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		if (focusedView != nullptr) {
			focusedView->LoseFocus();
		}
		mouseDown[0] = true;

		glm::vec3 mouseClick = glm::vec3(mouseX, mouseY, 0.0f);

		if (!USING_EMSCRIPTEN) {
			if (setup) {
				for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
					if ((*it)->CheckClicked(mouseClick)) {

						//TODO have a left on click and right on click?

						Keyboard::RemoveAllListeners();
						(*it)->OnClick(mouseClick, focusedView);

						return;
					}
				}
			}
			//else if (impulseMode) {
			//	graph->checkImpulseViews(mouseClick);
			//}
			else {
				if (pauseBtn->CheckClicked(mouseClick)) {
					pauseBtn->OnClick(mouseClick, focusedView);
				}
				else if (stopBtn->CheckClicked(mouseClick)) {
					stopBtn->OnClick(mouseClick, focusedView);
				}
			}
		}

		//Ray picking method from:
		// http://antongerdelan.net/opengl/raycasting.html

		//No Views were clicked, check shapes:
		if (setup && shapes.size() != 0) {
			float x = (2.0f * mouseX) / WIDTH - 1.0f;
			float y = 1.0f - (2.0f * mouseY) / HEIGHT;
			float z = 1.0f;

			//Click pos in NDC:
			//glm::vec4 rayNDC = glm::vec4(x, y, -1.0f, 1.0f);
			glm::vec4 rayNDC = glm::vec4(mouseX, mouseY, -1.0f, 1.0f);

			//Click in view space:
			if (USING_EMSCRIPTEN)
				inverseSceneTransform = i4;
			glm::vec4 rayView = inverseProjection * inverseSceneTransform * rayNDC;

			rayView = glm::vec4(rayView.x, rayView.y, -1.0f, 0.0f);

			//Click in world coordinates:
			glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(camera.view) * rayView));

			//Need to adjust for the 'scene'
			//rayWorld = glm::normalize(glm::vec3(inverseSceneTransform * glm::vec4(rayWorld.x, rayWorld.y, rayWorld.z, 1.0f)));

			std::cout << rayWorld.x << ", " << rayWorld.y << ", " << rayWorld.z << std::endl;

			//Ray line = CameraPosition + rayWorld * time
			// time < 0  --> behind camera
			// Let O = CameraPosition, C = sphere centroid, r = sphere radius
			// Points on a sphere and ray: 
			//     || O + rayWorld*time - C || - r = 0
			// rearranges to quadratic: time^2 + 2*time*b + c = 0,
			//  where b = rayWorld DOT ( O - C),
			//   and  c = (O - C) DOT (O - C) - r^2

			int previousPickedShapeIndex = pickedShapeIndex;
			if (pickedShape != nullptr) {
				pickedShape->objectColor = pickedShape->defaultColor;
				pickedShapeIndex = -1;
			}
			pickedShape = nullptr;
			float minDistance = 1000000.0f;

			for (int i = 0; i < shapes.size(); i++) {
				Shape *shape = shapes[i];
				glm::vec3 C = shape->centroid;

				if (shape->UsingBoundingSphere) {
					float r = shape->boundingSphereRadius - shape->BoundingSphereBuffer;
					glm::vec3 OminusC = camera.cameraPos - C;
					float b = glm::dot(rayWorld, OminusC);
					float c = glm::dot(OminusC, OminusC) - (r * r);

					float bSquaredMinusC = (b*b) - c;

					//if less than 0, solution to quadratic is imaginary, i.e. a miss

					if (bSquaredMinusC >= 0.0f) {
						//TODO if == 0.0f

						float t1, t2, squareRoot;
						squareRoot = glm::sqrt(bSquaredMinusC);
						t1 = -b + squareRoot;
						t2 = -b - squareRoot;

						float closest = (t1 < t2 ? t1 : t2);
						if (closest > 0.0f && closest < minDistance) {
							minDistance = closest;
							pickedShape = shape;
							pickedShapeIndex = i;
						}

					}
				}
				else {
					//check for bounding box intersection:

				}
			}

			if (pickedShape != nullptr) {
				std::cout << "Picked shape: " << pickedShape->name << std::endl;
				if (!icoMode) {
					pickedShape->objectColor = selectedColor;
				}

				if (pickedShapeIndex != previousPickedShapeIndex) {

				}

				if (!USING_EMSCRIPTEN) {
					bottomPanelStr.panel->ShowContent();

					bottomPanelStr.positionTextX->SetText("" + std::to_string(pickedShape->translation.x));
					bottomPanelStr.positionTextY->SetText("" + std::to_string(pickedShape->translation.y));
					bottomPanelStr.positionTextZ->SetText("" + std::to_string(pickedShape->translation.z));

					bottomPanelStr.velocityTextX->SetText("" + std::to_string(pickedShape->curVelocity.x));
					bottomPanelStr.velocityTextY->SetText("" + std::to_string(pickedShape->curVelocity.y));
					bottomPanelStr.velocityTextZ->SetText("" + std::to_string(pickedShape->curVelocity.z));

					bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->angularVelocityAxis.x));
					bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->angularVelocityAxis.y));
					bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->angularVelocityAxis.z));
					//bottomPanelStr.angularVelocitySpeedText->SetText("" + std::to_string(pickedShape->angularVelocity));
				}
			}
			else if (!USING_EMSCRIPTEN) {
				bottomPanelStr.panel->HideContent();
			}
		}
	}
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	inverseProjection = glm::inverse(projection);
	if (USING_EMSCRIPTEN) {
		std::cout << "Resetting glViewport" << std::endl;
		int *w = new int;
		int *h = new int;
		glfwGetFramebufferSize(window, w, h);
		std::cout << "width: " << *w << ", height: " << *h << std::endl;
		glViewport(0, 0, *w, *h);

		delete w;
		delete h;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals) {
	float max = (eigenVal1 > eigenVal2 ? (eigenVal1 > eigenVal3 ? eigenVal1 : eigenVal3) : (eigenVal2 > eigenVal3 ? eigenVal2 : eigenVal3));
	float remaining = MathUtils::abs(1.0f - minNormalization);
	float a = (eigenVal1 / max) * remaining;
	float b = (eigenVal2 / max) * remaining;
	float c = (eigenVal3 / max) * remaining;
	normalizedEvals.push_back(minNormalization + a);
	normalizedEvals.push_back(minNormalization + b);
	normalizedEvals.push_back(minNormalization + c);
}

float maxDistanceFromCentroid(TetrahedralMesh &mesh) {
	float max = -1.0f;
	glm::vec3 centroid = mesh.centroid;
	typedef std::vector<Tetrahedron>::iterator iter;
	for (iter it = mesh.Tetrahedra.begin(); it != mesh.Tetrahedra.end(); it++) {
		float distA = glm::distance(centroid, it->a);
		float distB = glm::distance(centroid, it->b);
		float distC = glm::distance(centroid, it->c);
		float distD = glm::distance(centroid, it->d);
		if (distA > max) {
			max = distA;
		}
		if (distB > max) {
			max = distB;
		}
		if (distC > max) {
			max = distC;
		}
		if (distD > max) {
			max = distD;
		}
	}
	return max;
}

void AddPairOfPoints(std::vector<GLfloat> &vector, glm::vec3 pt1, glm::vec3 pt2) {
	vector.push_back(pt1.x);
	vector.push_back(pt1.y);
	vector.push_back(pt1.z);
	vector.push_back(1.0f);
	vector.push_back(1.0f);
	vector.push_back(1.0f);

	vector.push_back(pt2.x);
	vector.push_back(pt2.y);
	vector.push_back(pt2.z);
	vector.push_back(1.0f);
	vector.push_back(1.0f);
	vector.push_back(1.0f);
}

void AddPoint(std::vector<GLfloat> &vector, glm::vec3 &pt, glm::vec3 &normal) {
	vector.push_back(pt.x);
	vector.push_back(pt.y);
	vector.push_back(pt.z);
	vector.push_back(normal.x);
	vector.push_back(normal.y);
	vector.push_back(normal.z);
}

struct ParamPointPair {
	ParamPoint p1;
	ParamPoint p2;
};

ParamPointPair initial = { { -11.0f, -11.0f, glm::vec3() },{ -11.0f, -11.0f, glm::vec3() } };

// A mapping used to store previous closest points
ParamPointPair previouses[4][4] = { { initial , initial , initial ,initial },
{ initial , initial , initial ,initial },
{ initial , initial , initial ,initial },
{ initial , initial , initial ,initial }, };

//Finds the closest points between sq1 & sq2, saves those in closest1 & closest2, and then draws a line
// between the two points
void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2, ParamPoint &pp1, ParamPoint &pp2) {
	//GLuint VAOdist, VBOdist;
	//std::vector<GLfloat> vertices;
	//shapeShader.Use();

	//if (previouses[sq1index][sq2index].p1.u < -10.f) {
		//Superquadric::ClosestPointFramework(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
		Superquadric::ClosestPointFramework(sq1, sq2, closest1, closest2, pp1, pp2);
	//}
	/*else {
		Superquadric::ClosestPointFrameworkTryPrevious(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
	}*/

	/*if (sq2index > 10) {
		std::cout << "Hey" << std::endl;
	}*/

	//AddPairOfPoints(vertices, closest1, closest2);

	//glGenVertexArrays(1, &VAOdist);
	//glGenBuffers(1, &VBOdist);
	//glBindVertexArray(VAOdist);
	//glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	//// Position attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	//glEnableVertexAttribArray(0);

	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	//glEnableVertexAttribArray(1);

	//glm::mat4 lineModel;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
	//glUniform3f(objectColorLoc, 1.0f, 0.0f, 0.0f);
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	//glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//glUniformMatrix4fv(sceneTransformLoc, 1, GL_FALSE, glm::value_ptr(sceneTransform));
	//glDrawArrays(GL_LINES, 0, vertices.size() / 6);

	//glBindVertexArray(0);

	//glDeleteBuffers(1, &VBOdist);
	//glDeleteBuffers(1, &VAOdist);
}

void DrawLine(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 color = glm::vec3(0.0f, 0.0f, 0.0f)) {

	GLuint VAOdist, VBOdist;
	std::vector<GLfloat> vertices;

	AddPairOfPoints(vertices, pt1, pt2);


	glGenVertexArrays(1, &VAOdist);
	glGenBuffers(1, &VBOdist);
	glBindVertexArray(VAOdist);
	glBindBuffer(GL_ARRAY_BUFFER, VBOdist);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glm::mat4 lineModel;
	glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
	glUniform4f(ShapeShader::getInstance().objectColorLoc, color.x, color.y, color.z, 1.0f);
	glDrawArrays(GL_LINES, 0, vertices.size() / 6);

	glBindVertexArray(0);

	glDeleteBuffers(1, &VBOdist);
	glDeleteBuffers(1, &VAOdist);
}

//Returns true if bounding spheres are intersecting/touching
// Doesn't account for object velocities !!
bool BoundingSphereTest(Shape &s1, Shape &s2) {
	glm::vec3 actualDistanceVec = s1.translation - s2.translation;
	float minAllowableDistance = s1.boundingSphereRadius + s2.boundingSphereRadius;

	//Use squared distance rather than computing square roots:
	float actualDistance = glm::dot(actualDistanceVec, actualDistanceVec);
	minAllowableDistance = minAllowableDistance * minAllowableDistance;

	return actualDistance <= minAllowableDistance;
}

void Collision(Shape &s1, ParamPoint &contactPt1, Shape &s2, ParamPoint &contactPt2, int i, int j) {

	if (icoMode && icoHit) {
		return; //For the icosahedron example, they actually collide again after initial c, but
		// just pretend it doesn't...
	}

	if (MathUtils::isZeroVec(s1.curVelocity) && MathUtils::isZeroVec(s2.curVelocity)) {
		return; //Zero velocities, just ignore impact since results will be 0
		//most likely user set up a scene with objects touching (e.g. a billiards break shot)
	}

	if (using2DShapes) {
		Impact2D i2;
		glm::vec3 r1 = glm::normalize(s2.translation - s1.translation);
		glm::vec3 r2 = glm::normalize(s1.translation - s2.translation);
		float angVel1 = MathUtils::magnitude(s1.angularVelocityAxis);
		float angVel2 = MathUtils::magnitude(s2.angularVelocityAxis);
		Impact2D::Impact2DOutput output =
			i2.impact(s1.mass, s2.mass, s1.angularInertia[0][0], s2.angularInertia[0][0],
				/*s1.*/frictionCoefficient, /*s1.*/ restitutionCoefficient,
				r1, r2,
				s1.curVelocity, s2.curVelocity,
				angVel1, angVel2);

		std::cout << "2D impact output: " << std::endl;
		std::cout << "v1end: " << output.v1.x << ", " << output.v1.y << ", " << output.v1.z << std::endl;
		std::cout << "v2end: " << output.v2.x << ", " << output.v2.y << ", " << output.v2.z << std::endl;
		std::cout << "w1end: " << output.w1 << std::endl;
		std::cout << "w2end: " << output.w2 << std::endl << std::endl;

		s1.curVelocity = output.v1;
		s2.curVelocity = output.v2;
		s1.angularVelocityAxis = glm::vec3(0.0f, 0.0f, output.w1);
		s2.angularVelocityAxis = glm::vec3(0.0f, 0.0f, output.w2);

		return;
	}

	glm::mat4 s1Rot = s1.getRotationMatrix();
	glm::mat4 s2Rot = s2.getRotationMatrix();

	glm::mat3 R1 = glm::mat3(s1Rot);
	glm::mat3 R2 = glm::mat3(s2Rot);

	//TODO compute angular inertias and use as Q
	glm::mat3 RTAN, Q1, Q2;
	glm::vec3 r1, r2, v1, v2, w1, w2;

	//Collision normal should be going from s2 into s1
	glm::vec3 normLocal = s2.GetNormalAtPoint(contactPt2);

	glm::vec4 normLocal4 = glm::vec4(normLocal.x, normLocal.y, normLocal.z, 1.0);
	glm::vec3 normGlobal = glm::vec3(s2Rot * normLocal4);

	std::cout << std::endl << "Contact normal: " << normGlobal.x << ", " << normGlobal.y << ", " << normGlobal.z << std::endl;
	if (Superquadric::checkVecForNaN(normGlobal)) {
		std::cout << "nan contact normal" << std::endl;
	}

	//Rotate global x-vector to contact normal
	glm::vec3 xAxis(1.0, 0.0, 0.0);
	RTAN = glm::mat3(MathUtils::rotationFromAtoB(xAxis, normGlobal));
	if (!usingClosedFormImpact) {
		glm::vec3 negX(-1.0, 0.0, 0.0);
		RTAN = glm::mat3(MathUtils::rotationFromAtoB(negX, normGlobal));
	}

	r1 = ShapeUtils::getLocalCoordinates(contactPt1.pt, s1.translation, s1Rot, s1.scaling);
	r2 = ShapeUtils::getLocalCoordinates(contactPt2.pt, s2.translation, s2Rot, s2.scaling);

	if (icoMode/*s1.name.find("tetra") != s1.name.npos || s1.name.find("Ico") != s1.name.npos*/) {
		/*r1 = glm::vec3(-1.0f, 0.0f, 0.0f);
		r2 = glm::vec3(1.0f, 0.0f, 0.0f);*/
		if (!usingClosedFormImpact) {
			RTAN = glm::mat3(glm::rotate(MathUtils::I4, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f)));
		}
		else {
			RTAN = glm::mat3();
		}

		r1 = glm::vec3(-.232744f, 0.0f, 0.0f);
		r2 = glm::vec3(.4375f, -.125f, -.125f);
		R2 = glm::mat3();

		icoHit = true;
		if (s1.name.find("tetra") != s1.name.npos) {
			Q2 = s1.angularInertia;
			Q1 = s2.angularInertia;
		}
		else {
			Q2 = s2.angularInertia;
			Q1 = s1.angularInertia;
		}

		//RTAN = glm::mat3();
	}
	
	v1 = s1.curVelocity;
	v2 = s2.curVelocity;
	w1 = s1.angularVelocityAxis;
	w2 = s2.angularVelocityAxis;

	std::cout << "Contact pt: " << contactPt1.pt.x << ", " << contactPt1.pt.y << ", " << contactPt1.pt.z << std::endl;
	std::cout << "v1: " << v1.x << ", " << v1.y << ", " << v1.z << std::endl;
	std::cout << "v2: " << v2.x << ", " << v2.y << ", " << v2.z << std::endl;
	std::cout << "w1: " << w1.x << ", " << w1.y << ", " << w1.z << std::endl;
	std::cout << "w2: " << w2.x << ", " << w2.y << ", " << w2.z << std::endl;
	std::cout << "r1: " << r1.x << ", " << r1.y << ", " << r1.z << std::endl;
	std::cout << "r2: " << r2.x << ", " << r2.y << ", " << r2.z << std::endl;
	std::cout << "R1: " << std::endl;
	MathUtils::printMat(R1);
	std::cout << "R2: " << std::endl;
	MathUtils::printMat(R2);
	std::cout << "RTAN: " << std::endl;
	MathUtils::printMat(RTAN);

	if (!usingClosedFormImpact) {
		//RTAN = glm::mat3(glm::rotate(i4, glm::pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)));
		Impact impact1(s1.mass, s2.mass, /*s1.*/frictionCoefficient, /*s1.*/restitutionCoefficient, R1, R2,
			Q1, Q2, RTAN, r1, r2, v1, v2, w1, w2);

		ImpactOutput output = impact1.impact();
		int numPts = output.impulsePts.size();

		//Just get a sampling of the points if there are too many
		if (numPts > 100) {
			std::vector<glm::vec3> impulsePts;
			std::vector<glm::vec3> velPts;
			
			int offset = numPts / 100;
			for (int i = 0; i < numPts; i += offset) {
				impulsePts.push_back(output.impulsePts[i]);
				velPts.push_back(output.velocityPts[i]);
			}

			previousImpactResults.v1end = output.v1end;
			previousImpactResults.v2end = output.v2end;
			previousImpactResults.w1end = output.w1end;
			previousImpactResults.w2end = output.w2end;
			previousImpactResults.endOfCompression = output.endOfCompression;
			previousImpactResults.endOfSliding = output.endOfSliding;
			previousImpactResults.Iend = output.Iend;
			previousImpactResults.impulsePts = impulsePts;
			previousImpactResults.velocityPts = velPts;
		}

		else {
			previousImpactResults = output;
		}

		if (numPts == 0) {
			std::cout << "No impulse pts" << std::endl;
		}

		s1.curVelocity = previousImpactResults.v1end;
		s2.curVelocity = previousImpactResults.v2end;

		//s1.angularVelocity = MathUtils::magnitude(previousImpactResults.w1end);
		s1.angularVelocityAxis = previousImpactResults.w1end;

		//s2.angularVelocity = MathUtils::magnitude(previousImpactResults.w2end);
		s2.angularVelocityAxis = previousImpactResults.w2end;

		std::cout << "v1end: " << previousImpactResults.v1end.x << ", " << previousImpactResults.v1end.y << ", " << previousImpactResults.v1end.z << std::endl;
		std::cout << "v2end: " << previousImpactResults.v2end.x << ", " << previousImpactResults.v2end.y << ", " << previousImpactResults.v2end.z << std::endl;
		std::cout << "w1end: " << previousImpactResults.w1end.x << ", " << previousImpactResults.w1end.y << ", " << previousImpactResults.w1end.z << std::endl;
		std::cout << "w2end: " << previousImpactResults.w2end.x << ", " << previousImpactResults.w2end.y << ", " << previousImpactResults.w2end.z << std::endl << std::endl;

		//if (!USING_EMSCRIPTEN)
			//return;

		impulseMode = true;

		contactShape1 = shapes[i];
		contactShape2 = shapes[j];

		contactShape1->objectColor = shapes[i]->contactColor;
		contactShape2->objectColor = shapes[j]->contactColor;


		prevCameraPosX = camera.cameraPos.x;
		prevCameraPosY = camera.cameraPos.y;
		prevCameraTargetX = camera.cameraTarget.x;
		prevCameraTargetY = camera.cameraTarget.y;

		cameraTranslation = glm::vec3(contactPt1.pt.x, contactPt1.pt.y, 0.0f) - glm::vec3(camera.cameraPos.x, camera.cameraPos.y, 0.0f);
		//std::cout << "Camera total translation vec: " << cameraTranslation.x << ", " << cameraTranslation.y << ", " << cameraTranslation.z << std::endl;
		cameraTranslation = (1.0f / (float)numCameraTranslations) * cameraTranslation;
		//std::cout << "Camera frame translation vec: " << cameraTranslation.x << ", " << cameraTranslation.y << ", " << cameraTranslation.z << std::endl;

		cameraContactPt = glm::vec3(contactPt1.pt.x, contactPt1.pt.y, camera.cameraPos.z);
		cameraTransDone = false;
		cameraTranslations = 0;

		curImpulsePt = 0;
		curVelocityPt = 0;

	}
	else {
		ImpactClosedForm impact1(s1.mass, s2.mass, /*s1.*/frictionCoefficient, /*s1.*/ restitutionCoefficient, R1, R2,
			Q1, Q2, RTAN, r1, r2, v1, v2, w1, w2);

		ImpactClosedFormOutput result1 = impact1.impact();

		std::cout << "Impact output: " << std::endl;
		std::cout << "v1end: " << result1.v1end.x << ", " << result1.v1end.y << ", " << result1.v1end.z << std::endl;
		std::cout << "v2end: " << result1.v2end.x << ", " << result1.v2end.y << ", " << result1.v2end.z << std::endl;
		std::cout << "w1end: " << result1.w1end.x << ", " << result1.w1end.y << ", " << result1.w1end.z << std::endl;
		std::cout << "w2end: " << result1.w2end.x << ", " << result1.w2end.y << ", " << result1.w2end.z << std::endl << std::endl;

		s1.curVelocity = result1.v1end;
		s2.curVelocity = result1.v2end;

		s1.angularVelocityAxis = result1.w1end;
		//s1.angularVelocity = glm::length(result1.w1end);
		s2.angularVelocityAxis = result1.w2end;
		//s2.angularVelocity = glm::length(result1.w2end);

		/*std::cout << "v1end: " << result1.v1end.x << ", " << result1.v1end.y << ", " << result1.v1end.z << std::endl;
		std::cout << "v2end: " << result1.v2end.x << ", " << result1.v2end.y << ", " << result1.v2end.z << std::endl;
		std::cout << "w1end: " << result1.w1end.x << ", " << result1.w1end.y << ", " << result1.w1end.z << std::endl;
		std::cout << "w2end: " << result1.w2end.x << ", " << result1.w2end.y << ", " << result1.w2end.z << std::endl;*/

		std::cout << "Iend: " << result1.Iend.x << ", " << result1.Iend.y << ", " << result1.Iend.z << std::endl << std::endl;
	}

	//Sometimes impact takes so long that the time difference makes it so shapes are too far away
	lastFrame = (float)glfwGetTime();
}

//Called on startup
void InitOpenGL() {

	// Init GLFW
	glfwInit();

	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	//For antialiasing:
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Create a GLFWwindow object that we can use for GLFW's functions
	window = glfwCreateWindow(WIDTH, HEIGHT, "MultiCollide", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetWindowSizeCallback(window, window_size_callback);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;
	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);

	//For antialiasing:
	glEnable(GL_MULTISAMPLE);

	glLineWidth(3.0f);

	shapeShader = ShapeShader::getInstance().shader;
	camera.resetCameraState();

	// Build and compile our shader program
	//*if (!USING_EMSCRIPTEN) {
		//shapeShader = Shader("vertexShader.glsl", "fragmentShader.glsl");
	//}
	/*else {
		shapeShader = Shader::ShaderFromCode("#version 100 \n attribute vec3 position;\n attribute vec3 normal;\n uniform mat4 model;\n uniform mat4 view;\n uniform mat4 projection;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n void main()\n {\n Normal = mat3(model) * normal;\n FragPos = vec3(model * vec4(position, 1.0));\n	if(position.y == 0.0){\n		Line = vec3(1.0, 1.0, 1.0);\n	}\n	else {\n		Line = vec3(-1.0, -1.0, -1.0);\n	}\n   gl_Position = projection * view * model * vec4(position, 1.0);\n }\n",
			"#version 100\n precision mediump float;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n  uniform vec3 lightPos;\n uniform vec3 objectColor;\n void main()\n {\n vec3 lightColor = vec3(1.0, 1.0, 1.0);\n float ambientStrength = 0.5;\n  vec3 ambient = ambientStrength * lightColor;\n     vec3 norm = normalize(Normal);\n    vec3 lightDir = normalize(lightPos - FragPos);\n    float diff = max(dot(norm, lightDir), 0.0);\n    vec3 diffuse = diff * lightColor;\n    vec3 result = (ambient + diffuse) * objectColor;\n    gl_FragColor = vec4(result, 1.0);\n if(objectColor.x + objectColor.y + objectColor.z < .00001) {\n 		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n 	}\n }\n");
	}*/

	/*modelLoc = glGetUniformLocation(shapeShader.Program, "model");
	viewLoc = glGetUniformLocation(shapeShader.Program, "view");
	projLoc = glGetUniformLocation(shapeShader.Program, "projection");
	lightPosLoc = glGetUniformLocation(shapeShader.Program, "lightPos");
	objectColorLoc = glGetUniformLocation(shapeShader.Program, "objectColor");
	sceneTransformLoc = glGetUniformLocation(shapeShader.Program, "sceneTransform");*/
}

void InitSetupSceneTransform() {
	sceneTransform = glm::translate(sceneTransform, glm::vec3(.3f, .3f, 0.0f));
	sceneTransform = glm::scale(sceneTransform, glm::vec3(.7f, .7f, .7f));
	inverseSceneTransform = glm::inverse(sceneTransform);
}

/*
void InitCameraControls() {

	//TODO put inside transparent panel

	GLView *upBtn = new GLView(.025f, .075f);
	upBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	upBtn->Translate(glm::vec3(.85f, .85f, 0.0f));
	upBtn->SetClickListener(camera.CameraUpOnClick);
	views.push_back(upBtn);

	GLView *leftBtn = new GLView(.075f, .025f);
	leftBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	leftBtn->Translate(glm::vec3(.85f - .075f, .85f - .025f, 0.0f));
	leftBtn->SetClickListener(camera.CameraLeftOnClick);
	views.push_back(leftBtn);

	GLView *downBtn = new GLView(.025f, .075f);
	downBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	downBtn->Translate(glm::vec3(.85f, .85f - .075f - .025f, 0.0f));
	downBtn->SetClickListener(camera.CameraDownOnClick);
	views.push_back(downBtn);

	GLView *rightBtn = new GLView(.075f, .025f);
	rightBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	rightBtn->Translate(glm::vec3(.85f + .025f, .85f - .025f, 0.0f));
	rightBtn->SetClickListener(camera.CameraRightOnClick);
	views.push_back(rightBtn);

}*/

void InitViews() {
	for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
		(*it)->InitBuffers();
	}
}

void PositionShapeToAdd(Shape *shape) {

	bool valid = false;

	while (!valid) {

		valid = true;

		for (unsigned int i = 0; i < shapes.size(); i++) {
			Shape *other = shapes[i];

			while (BoundingSphereTest(*shape, *other)) {
				valid = false;
				glm::vec3 adjustment = shape->centroid - other->centroid;
				if (adjustment.x + adjustment.y + adjustment.z < .00001f) {
					//could randomize direction...todo 3
					adjustment = glm::vec3(1.0f, 0.0f, 0.0f);
				}
				shape->translation = (shape->centroid += glm::normalize(adjustment));
			}
		}

	}
}

bool useOpenGLframe = false;
glm::mat4 rotToFrame = glm::rotate(MathUtils::I4, -MathUtils::PI_OVER_2, glm::vec3(1.0f, 0.0f, 0.0f));

void Render() {

	shapeShader.Use();

	// Clear the colorbuffer & depthbuffer
	glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 lightPos(0.0f, 3.0f, 3.0f);

	glUniform3f(ShapeShader::getInstance().lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(camera.view));
	glUniformMatrix4fv(ShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	//if (!USING_EMSCRIPTEN)
		glUniformMatrix4fv(ShapeShader::getInstance().sceneTransformLoc, 1, GL_FALSE, glm::value_ptr(sceneTransform));

	//Draw each superquadric based on its own translation && rotation
	for (std::vector<Shape*>::iterator it = shapes.begin(); it != shapes.end(); it++) {
		Shape *shape = (*it);

		glm::mat4 shapeRot = shape->getRotationMatrix();

		shape->model = glm::scale((glm::translate(MathUtils::I4, shape->translation) * shapeRot), glm::vec3(shape->scaling));
		//shape->model = glm::translate(shape->rotation * (glm::scale(i4, shape->scaling)), shape->translation);

		glUniform4f(ShapeShader::getInstance().objectColorLoc, shape->objectColor.x, shape->objectColor.y, shape->objectColor.z, 1.0f);
		//glUniform3fv(objectColorLoc, 3, &(shape->objectColor[0]));
		glUniformMatrix4fv(ShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(shape->model));

		if (shape->useCustomColors) {
			ColorShapeShader::getInstance().shader.Use();
			glUniform3f(ColorShapeShader::getInstance().lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
			glUniformMatrix4fv(ColorShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(camera.view));
			glUniformMatrix4fv(ColorShapeShader::getInstance().projLoc, 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(ColorShapeShader::getInstance().sceneTransformLoc, 1, GL_FALSE, glm::value_ptr(sceneTransform));
			glUniformMatrix4fv(ColorShapeShader::getInstance().modelLoc, 1, GL_FALSE, glm::value_ptr(shape->model));
		}

		shape->Draw(shapeShader);
		if (shape->useCustomColors) {
			ShapeShader::getInstance().shader.Use();
		}

		if (plotPrincipalFrame) {
			float scaling = shape->scaling * (shape->boundingSphereRadius + .05f);
			if (!useOpenGLframe && !using2DShapes) {
				shapeRot = shapeRot * rotToFrame;
			}
			Frame::Draw(shape->translation, shapeRot, scaling, using2DShapes);
		}
		if (plotRotationAxis) {
			glUniform4f(ShapeShader::getInstance().objectColorLoc, .8f, .1f, .1f, 1.0f);
			shape->DrawRotationAxis(shapeShader);
		}

		//Draw velociy:
		if (setup) {

			if (cursorState == velocity) {
				glUniform4f(ShapeShader::getInstance().objectColorLoc, 0.5f, 0.0f, 0.5f, 1.0f);
				shape->DrawInitialVelocity(shapeShader);
			}

			if (cursorState == angular_velocity && !using2DShapes) {
				glUniform4f(ShapeShader::getInstance().objectColorLoc, 1.0f, 0.5f, 0.0f, 1.0f);
				shape->DrawInitialAngularVelocity(shapeShader);
			}
		}
	}

	//Render global frame in lower left corner:
	glm::vec3 globalFramePos = glm::vec3(-.9f, -.9f, 0.0f);
	glm::vec3 vecToCam(0.0f, 0.0f, -1.0f);
	glm::vec3 camPosToTarget = glm::normalize(camera.cameraTarget - camera.cameraPos);
	glm::mat4 invRot = MathUtils::rotationFromAtoB(vecToCam, camPosToTarget);
	glm::mat4 frameRot = glm::inverse(invRot); 

	if (!useOpenGLframe && !using2DShapes) {
		frameRot = frameRot * rotToFrame;
	}
	
	Camera frameCam;
	frameCam.cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	frameCam.cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	frameCam.up = glm::vec3(0.0f, 1.0f, 0.0f);
	frameCam.calcView();
	glUniformMatrix4fv(ShapeShader::getInstance().viewLoc, 1, GL_FALSE, glm::value_ptr(frameCam.view));
	Frame::Draw(globalFramePos, frameRot, .2f, using2DShapes);

	if (!USING_EMSCRIPTEN) {
		if (setup) {
			for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
				(*it)->Draw();
			}
		}
		else {
			pauseBtn->Draw();
			stopBtn->Draw();
		}
	}

	// Swap the screen buffers
	glfwSwapBuffers(window);
}

//Called on tear down
void Close() {

	DestroyShapes();

	if (!USING_EMSCRIPTEN) {
		DestroyViews();
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
}

void DestroyViews() {
	//Cleanup Views:
	for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
		delete *it;
	}
	views.clear();
}

void DestroyShapes() {
	// Properly de-allocate all resources once they've outlived their purpose
	for (std::vector<Shape*>::iterator it = shapes.begin(); it != shapes.end(); it++) {
		Shape *shape = (*it);
		glDeleteVertexArrays(1, &(shape->VAO));
		glDeleteBuffers(1, &(shape->VBO));
		delete shape;
	}
}

bool checkForCollision(int shapeI, int shapeJ, ParamPoint &ppI, ParamPoint &ppJ) {
	int i = shapeI;
	int j = shapeJ;

	if (BoundingSphereTest(*shapes[i], *shapes[j])) {

		glm::vec3 closestPt1, closestPt2;
		//return CollisionDetector::Detect(shapes[i], shapes[j], closestPt1, closestPt2, ppI, ppJ);
		return shapes[i]->DispatchCollisionDetection(shapes[j], closestPt1, closestPt2, ppI, ppJ);

		//if (shapes[i]->name.find("Tetra") != shapes[i]->name.npos 
		//	|| shapes[i]->name.find("Ico") != shapes[i]->name.npos) {

		//	if (icoHit) {
		//		return false;
		//	}

		//	Tetra *tetra = nullptr;
		//	if (shapes[i]->name.find("tetra") != shapes[i]->name.npos) {
		//		tetra = (Tetra*)shapes[i];
		//	}
		//	else {
		//		tetra = (Tetra*)shapes[j];
		//	}

		//	//contact point is location of tetra-pt
		//	ppI.pt = ppJ.pt = glm::vec3(tetra->model * glm::vec4(tetra->c.x, tetra->c.y, tetra->c.z, 1.0f));
		//	return true;
		//}
		/*
		if ((shapes[i]->name.find("Tetra") != shapes[i]->name.npos || shapes[i]->name.find("Ico") != shapes[i]->name.npos)  //Tetra-Ico example
			|| using2DShapes																								// Polygons
			|| shapes[i]->name.find("ube") != shapes[i]->name.npos) {														//Cube

			//SAT shapes
			ShapeSeparatingAxis *c1 = (ShapeSeparatingAxis*)shapes[i];
			ShapeSeparatingAxis *c2 = (ShapeSeparatingAxis*)shapes[j];
			if (CollisionDetector::Detect(*c1, *c2)) {
				return true;
			}
			else return false;
		}

		glm::vec3 ptI;
		glm::vec3 ptJ;

		Superquadric *si = castToSuperquadric(shapes[i], "checkForCollision");
		Superquadric *sj = castToSuperquadric(shapes[j], "checkForCollision");
		if (si && sj) {
			Superquadric::ClosestPointFramework(*si, *sj, ptI, ptJ, ppI, ppJ);
			//if (Superquadric::IsZeroVector(ptI)) {
				
			//}
		}

		//TODO  use squared distance instead
		float distance = glm::distance(ptI, ptJ);

		//check for collision:
		if (distance < contactAccuracy || glm::dot(shapes[i]->translation - shapes[j]->translation, ptI - ptJ) < 0.0) {
			return true;
		}
		*/
	}

	return false;
}

int getNumShapes() {
	return shapes.size();
}

float getShapePositionX(unsigned int shapeIndex) {
	return shapes[shapeIndex]->translation.x;
}

float getShapePositionY(unsigned int shapeIndex) {
	return shapes[shapeIndex]->translation.y;
}

float getShapePositionZ(unsigned int shapeIndex) {
	return shapes[shapeIndex]->translation.z;
}

float getShapeVelocityX(unsigned int shapeIndex) {
	return shapes[shapeIndex]->curVelocity.x;
}

float getShapeVelocityY(unsigned int shapeIndex) {
	return shapes[shapeIndex]->curVelocity.y;
}

float getShapeVelocityZ(unsigned int shapeIndex) {
	return shapes[shapeIndex]->curVelocity.z;
}

float getShapeAngularVelocityX(unsigned int shapeIndex) {
	return shapes[shapeIndex]->angularVelocityAxis.x;
}

float getShapeAngularVelocityY(unsigned int shapeIndex) {
	return shapes[shapeIndex]->angularVelocityAxis.y;
}

float getShapeAngularVelocityZ(unsigned int shapeIndex) {
	return shapes[shapeIndex]->angularVelocityAxis.z;
}

float getShapeAngularVelocitySpeed(unsigned int shapeIndex) {
	return MathUtils::magnitude(shapes[shapeIndex]->angularVelocityAxis);
}

float getShapeRotationAxisX(unsigned int shapeIndex) {
	return shapes[shapeIndex]->rotationAxis.x;
	//return glm::quat_cast(shapes[shapeIndex]->getRotationMatrix()).x;
}

float getShapeRotationAxisY(unsigned int shapeIndex) {
	return shapes[shapeIndex]->rotationAxis.y;
	//return glm::quat_cast(shapes[shapeIndex]->getRotationMatrix()).y;
}

float getShapeRotationAxisZ(unsigned int shapeIndex) {
	return shapes[shapeIndex]->rotationAxis.z;
	//return glm::quat_cast(shapes[shapeIndex]->getRotationMatrix()).z;
}

float getShapeRotationAngle(unsigned int shapeIndex) {
	return shapes[shapeIndex]->rotationAngle;
	//return glm::quat_cast(shapes[shapeIndex]->getRotationMatrix()).w;
}

float getShapeMass(unsigned int shapeIndex) {
	return shapes[shapeIndex]->mass;
}

float getShapeFriction(/*unsigned int shapeIndex*/) {
	return /*shapes[shapeIndex]->*/frictionCoefficient;
}

float getShapeRestitution(/*unsigned int shapeIndex*/) {
	return /*shapes[shapeIndex]->*/restitutionCoefficient;
}

float getShapeScale(unsigned int shapeIndex) {
	return shapes[shapeIndex]->scaling;
}

void setShapePositionX(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->translation.x = value;
		shapes[shapeIndex]->centroid.x = value;
	}
	/*for (int i = 0; i < shapes.size(); i++) {
		if (i != shapeIndex) {
			ParamPoint pI, pJ;
			while (checkForCollision(shapeIndex, i, pI, pJ)) {
				
			}
		}
	}*/
}

void setShapePositionY(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->translation.y = value;
		shapes[shapeIndex]->centroid.y = value;
	}
}

void setShapePositionZ(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->translation.z = value;
		shapes[shapeIndex]->centroid.z = value;
	}
}

void setShapeVelocityX(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->curVelocity.x = value;
}

void setShapeVelocityY(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->curVelocity.y = value;
}

void setShapeVelocityZ(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->curVelocity.z = value;
}

void setShapeAngularVelocityX(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->angularVelocityAxis.x = value;
	//shapes[shapeIndex]->angularVelocity = sqrtf(shapes[shapeIndex]->angularVelocityAxis.x * shapes[shapeIndex]->angularVelocityAxis.x + shapes[shapeIndex]->angularVelocityAxis.y * shapes[shapeIndex]->angularVelocityAxis.y + shapes[shapeIndex]->angularVelocityAxis.z * shapes[shapeIndex]->angularVelocityAxis.z);
}

void setShapeAngularVelocityY(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->angularVelocityAxis.y = value;
	//shapes[shapeIndex]->angularVelocity = sqrtf(shapes[shapeIndex]->angularVelocityAxis.x * shapes[shapeIndex]->angularVelocityAxis.x + shapes[shapeIndex]->angularVelocityAxis.y * shapes[shapeIndex]->angularVelocityAxis.y + shapes[shapeIndex]->angularVelocityAxis.z * shapes[shapeIndex]->angularVelocityAxis.z);
}

void setShapeAngularVelocityZ(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->angularVelocityAxis.z = value;
	//shapes[shapeIndex]->angularVelocity = sqrtf(shapes[shapeIndex]->angularVelocityAxis.x * shapes[shapeIndex]->angularVelocityAxis.x + shapes[shapeIndex]->angularVelocityAxis.y * shapes[shapeIndex]->angularVelocityAxis.y + shapes[shapeIndex]->angularVelocityAxis.z * shapes[shapeIndex]->angularVelocityAxis.z);
}

//void setShapeAngularVelocitySpeed(unsigned int shapeIndex, float value) {
//	shapes[shapeIndex]->angularVelocity = value;
//}

void setShapeRotationAxisX(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->rotationAxis.x = value;
	}
}

void setShapeRotationAxisY(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->rotationAxis.y = value;
	}
}

void setShapeRotationAxisZ(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->rotationAxis.z = value;
	}
}

void setShapeRotationAngle(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->rotationAngle = value;
	}
}

void setShapeMass(unsigned int shapeIndex, float value) {
	shapes[shapeIndex]->mass = value;
}

void setShapeFriction(/*unsigned int shapeIndex,*/ float value) {
	/*shapes[shapeIndex]->*/frictionCoefficient = value;
}

void setShapeRestitution(/*unsigned int shapeIndex, */ float value) {
	/*shapes[shapeIndex]->*/restitutionCoefficient = value;
}

void setShapeScale(unsigned int shapeIndex, float value) {
	if (!icoMode) {
		shapes[shapeIndex]->scaling = value;
	}
}

void removeShape(unsigned int index) {
	Shape *shapeToDelete = shapes[index];
	shapes.erase(shapes.begin() + index);

	if (pickedShape == shapeToDelete) {
		pickedShape = nullptr;
		pickedShapeIndex = -1;
	}
	else if (index < pickedShapeIndex) {
		pickedShapeIndex--; //pickedShape got moved up in the vector
	}

	delete shapeToDelete;
}

//Return index of currently selected shape, -1 if none selected
int getSelectedShape() {
	if (pickedShape == nullptr || shapes.size() == 0) {
		return -1;
	}

	//TODO cache pickedShape index
	for (unsigned int i = 0; i < shapes.size(); i++) {
		if (shapes[i] == pickedShape) {
			return i;
		}
	}

	std::cout << "Not found" << std::endl;
	return -1;
}

void setSelectedShape(unsigned int shapeIndex) {
	if (shapeIndex < 0 || shapeIndex >= shapes.size()) {
		std::cout << "Not a valid shape index in setSelectedShape" << std::endl;
		return;
	}

	if (pickedShape != nullptr) {
		pickedShape->objectColor = pickedShape->defaultColor;
	}
	pickedShape = shapes[shapeIndex];
	pickedShape->objectColor = selectedColor;
}

void addDuplicateShape(unsigned int shapeIndex) {
	//TODO - make an abstract duplicate function for all Shapes, subclasses must provide implementation
}

void focusCameraOnShape(unsigned int shapeIndex) {
	//TODO - no idea
}

void createCustomSuperquadric(float a1, float a2, float a3, float e1, float e2) {
	Superquadric *superquad = new Superquadric();
	superquad->a1 = a1;
	superquad->a2 = a2;
	superquad->a3 = a3;
	superquad->e1 = e1;
	superquad->e2 = e2;  // .3f  1.0f  2.0f  3.0f
	superquad->u1 = -glm::pi<float>() / 2.0f;
	superquad->u2 = glm::pi<float>() / 2.0f;
	superquad->v1 = -glm::pi<float>();
	superquad->v2 = glm::pi<float>();
	superquad->u_segs = 23;
	superquad->v_segs = 23;
	static int superquadNum = 1;
	superquad->name = std::string("Custom" + std::to_string(superquadNum++));
	superquad->setShapeCSVcode(4);
	AddSuperquadric(superquad);
}

void uploadMeshName(char* filePath) {
	TetrahedralMesh *mesh = new TetrahedralMesh("C:\\Users\\Jacob\\Desktop\\Work\\meshFiles\\testTetra.txt");
	mesh->ComputeCentroid();
	mesh->Translate(-mesh->centroid);
	mesh->ComputeInertia();

	//Replace with AABB when complete
	float maxDist = maxDistanceFromCentroid(*mesh);
	mesh->boundingSphereRadius = mesh->BoundingSphereBuffer + maxDist;
	if (mesh->boundingSphereRadius > 4.0f) {
		mesh->scaling = .5f;
		mesh->boundingSphereRadius *= .5f;
	}
	mesh->UsingBoundingSphere = true;
	AddShape(mesh);
}

void uploadMesh(char* meshName, char* meshData) {
	meshName = "Mesh\0";
	meshData = "Mass\n 25.0\nVertices\n-1.0 -1.0 -1.0\n1.0 1.0 1.0\n-1.0 -1.0 1.0\n1.0 -1.0 -1.0\nTetrahedra\n1 2 3 4\nEnd\0";
	TetrahedralMesh *mesh = new TetrahedralMesh(meshName, meshData);
	mesh->ComputeCentroid();
	mesh->Translate(-mesh->centroid);
	mesh->ComputeInertia();

	//Replace with AABB when complete
	float maxDist = maxDistanceFromCentroid(*mesh);
	mesh->boundingSphereRadius = mesh->BoundingSphereBuffer + maxDist;
	if (mesh->boundingSphereRadius > 4.0f) {
		mesh->scaling = .5f;
		mesh->boundingSphereRadius *= .5f;
	}
	mesh->UsingBoundingSphere = true;

	AddShape(mesh);
}

void duplicateShape(unsigned int shapeIndex) {
	Shape *shapePtr = shapes[shapeIndex];
}

/*
	First line of csv contains scene settings
	Then shapes are listed
    Each shape will have 2 lines in the csv
    First line is a # indicating the shape type, followed by any special info (e.g. superquad params)
    2nd line is shape orientation & other properties (velocity, mass, etc.) same layout for all shapes
 */
const char* getSceneCSV() {
	std::ostringstream os;

	os << using2DShapes << "," << frictionCoefficient << "," << restitutionCoefficient << std::endl;

	std::vector<Shape*>::iterator iter;
	for (iter = shapes.begin(); iter != shapes.end(); iter++) {
		os << (*iter)->getShapeCSVline1();
		os << (*iter)->getShapeCSVline2();
	}
	os.flush();
	std::string csv = os.str();
	return csv.c_str();
}