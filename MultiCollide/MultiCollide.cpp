#include <iostream>
#include <fstream>
#include <vector>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
//#include <SOIL.h>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>

//EIGEN - SVD 
#include <Eigen/Dense>
#include <Eigen\SVD>

// Other includes
#include "MathUtils.h"
#include "Shader.h"
#include "Quaternion.h"
#include "TetrahedralMesh.h"
#include "Superquadric.h"
#include "Cube.h"

#include "Impact.h"

#include "GLView.h"
#include "GLButton.h"
#include "GLPanel.h"
#include "GLLabel.h"
#include "GLTextBox.h"
#include "GLRoundedButton.h"

#include "Keyboard.h"
#include "Text.h"

const bool USING_EMSCRIPTEN = false;

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
void AddPoint(std::vector<GLfloat> &vector, glm::vec3 pt, glm::vec3 normal);
void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2);
bool BoundingSphereTest(Shape &s1, Shape &s2);
void Collision(Shape &s1, glm::vec3 contactPt1, Shape &s2, glm::vec3 contactPt2);

void InitOpenGL();
void InitSetupSceneTransform();
void InitLeftPanel();
void InitBottomPanel();
void InitCameraControls();
void InitViews();
//void HidePanels();

void PositionShapeToAdd(Shape *shape);
void Render();
void Close();
void DestroyViews();
void DestroyShapes();

//Functions for emscripten:
int getNumShapes();
float getShapeAttribute(unsigned int shapeIndex, std::string attribute);
void setShapeAttribute(unsigned int shapeIndex, std::string attribute, float value);
void removeShape(unsigned int index);
int getSelectedShape();
void setSelectedShape(unsigned int shapeIndex);
void addDuplicateShape(unsigned int shapeIndex);
void focusCameraOnShape(unsigned int index);
void createCustomSuperquadric(float a1, float a2, float a3, float e1, float e2);

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

	GLRoundedButton *sphereBtn;
	GLButton *cubeBtn;
	GLButton *ellipsoidBtn;
	GLButton *superquadricBtn;
	GLButton *runBtn;
	GLButton *resetBtn;
	GLButton *positionBtn;
	GLButton *rotateBtn;
	GLButton *scaleUniformBtn;
	GLButton *velocityBtn;
	GLButton *angularVelocityBtn;

} sidePanelStr;

// Window dimensions
GLuint WIDTH = 1600, HEIGHT = 1400;

GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame

float zOffset = -6.0f;

glm::vec3 initialCameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 cameraPos = initialCameraPos;
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

//glm::vec3 initialCameraPos = glm::vec3(-3.0f, 1.0f, 0.0f);
//glm::vec3 cameraPos = initialCameraPos;
//glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

//Matrices:
glm::mat4 i4;
glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, up);
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

//Shader uniform locations:
GLint modelLoc;
GLint viewLoc;
GLint projLoc;
GLint lightPosLoc;
GLint objectColorLoc;
GLint sceneTransformLoc;

//Scene state:
bool paused = false;
bool reverse = false;
bool setup = true;


float contactAccuracy = 0.00001f;


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
glm::vec3 selectedColor = glm::vec3(.2f, .7f, .2f);
glm::mat4 sceneTransform;
glm::mat4 inverseSceneTransform;
glm::mat4 inverseProjection;

//Used so objects don't 're-collide' before they have  
// moved away from another colliding objects
std::map<std::pair<int, int>, int> framesSinceCollision;

void AddSuperquadric(Superquadric *superquad) {
	SuperEllipsoid::sqSolidEllipsoid(*superquad);
	Superquadric::InitializeClosestPoints(*superquad);
	superquad->translation = superquad->centroid = glm::vec3(0.0f, 0.0f, 0.0f);
	PositionShapeToAdd(superquad);
	superquad->InitVAOandVBO(shapeShader);
	shapes.push_back(superquad);
}

void AddSphereOnClick() {
	Superquadric *sphere = new Superquadric();
	sphere->CreateSphere();
	static int sphereNum = 1;
	sphere->name = std::string("Sphere " + std::to_string(sphereNum++));
	AddSuperquadric(sphere);
}

void AddCubeOnClick() {
	Cube *cube = new Cube();
	cube->InitVAOandVBO(shapeShader);
	PositionShapeToAdd(cube);
	shapes.push_back(cube);
}

void AddSuperquadricOnClick() {
	//std::cout << "Clicked Add Superquadric" << std::endl;
	Superquadric *superquad = new Superquadric();
	superquad->a1 = 1.0f;
	superquad->a2 = 1.0f;
	superquad->a3 = 1.0f;
	superquad->e1 = 3.0f;
	superquad->e2 = 3.0f;  // .3f  1.0f  2.0f  3.0f
	superquad->u1 = -glm::pi<float>() / 2.0f;
	superquad->u2 = glm::pi<float>() / 2.0f;
	superquad->v1 = -glm::pi<float>();
	superquad->v2 = glm::pi<float>();
	superquad->u_segs = 30;
	superquad->v_segs = 30;
	static int superquadNum = 1;
	superquad->name = std::string("Superquadric " + std::to_string(superquadNum++));

	AddSuperquadric(superquad);
}

void AddEllipsoidOnClick() {
	//std::cout << "Clicked Add Ellipsoid" << std::endl;
	Superquadric *superquad = new Superquadric();
	superquad->a1 = 1.0f;
	superquad->a2 = 1.0f;
	superquad->a3 = 1.0f;
	superquad->e1 = 1.5f;
	superquad->e2 = 1.5f;  // .3f  1.0f  2.0f  3.0f
	superquad->u1 = -glm::pi<float>() / 2.0f;
	superquad->u2 = glm::pi<float>() / 2.0f;
	superquad->v1 = -glm::pi<float>();
	superquad->v2 = glm::pi<float>();
	superquad->u_segs = 30;
	superquad->v_segs = 30;
	static int ellipsoidNum = 1;
	superquad->name = std::string("Superquadric " + std::to_string(ellipsoidNum++));

	AddSuperquadric(superquad);
}

void PauseOnClick() {
	paused = !paused;
}

void StopOnClick() {
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
	if(!USING_EMSCRIPTEN)
		InitSetupSceneTransform();

}

//Clears shapes and resets camera
void ResetOnClick() {
	std::cout << "Clicked Reset" << std::endl;
	DestroyShapes();
	shapes.clear();
	cameraPos = initialCameraPos;
	cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	view = glm::lookAt(cameraPos, cameraTarget, up);
}

void RunOnClick() {
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

//TODO add panning, zooming, and QUATERNIONS
float cameraAdjust = .1f;
void CameraUpOnClick() {
	glm::mat4 rot = glm::rotate(i4, -cameraAdjust, glm::vec3(1.0f, 0.0f, 0.0f));
	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	view = glm::lookAt(cameraPos, cameraTarget, up);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void CameraDownOnClick() {
	glm::mat4 rot = glm::rotate(i4, cameraAdjust, glm::vec3(1.0f, 0.0f, 0.0f));
	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	view = glm::lookAt(cameraPos, cameraTarget, up);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void CameraLeftOnClick() {
	glm::mat4 rot = glm::rotate(i4, -cameraAdjust, glm::vec3(0.0f, 1.0f, 0.0f));
	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	view = glm::lookAt(cameraPos, cameraTarget, up);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void CameraRightOnClick() {
	glm::mat4 rot = glm::rotate(i4, cameraAdjust, glm::vec3(0.0f, 1.0f, 0.0f));
	cameraPos = glm::vec3(rot * glm::vec4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f));
	view = glm::lookAt(cameraPos, cameraTarget, up);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

//Defines what should be updated when shapes are clicked & dragged by mouse
enum CursorType { translate = 0, rotate = 1, scale_uniform = 2, velocity = 3, angular_velocity = 4};
CursorType cursorState;

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

void TranslateOnClick() {
	if(!USING_EMSCRIPTEN) ResetCursorState();
	cursorState = translate;
	//std::cout << "Cursor state: translate" << std::endl;
	if (!USING_EMSCRIPTEN) sidePanelStr.positionBtn->SetToggledColor();
}

void RotateOnClick() {
	if (!USING_EMSCRIPTEN) ResetCursorState();
	cursorState = rotate;
	//std::cout << "Cursor state: rotate" << std::endl;
	if (!USING_EMSCRIPTEN) sidePanelStr.rotateBtn->SetToggledColor();
}

void ScaleOnClick() {
	if (!USING_EMSCRIPTEN) ResetCursorState();
	cursorState = scale_uniform;
	//std::cout << "Cursor state: scale" << std::endl;
	if (!USING_EMSCRIPTEN) sidePanelStr.scaleUniformBtn->SetToggledColor();
}

void VelocityOnClick() {
	if (!USING_EMSCRIPTEN) ResetCursorState();
	cursorState = velocity;
	//std::cout << "Cursor state: velocity" << std::endl;
	if (!USING_EMSCRIPTEN) sidePanelStr.velocityBtn->SetToggledColor();
}

void AngularVelocityOnClick() {
	if (!USING_EMSCRIPTEN) ResetCursorState();
	cursorState = angular_velocity;
	//std::cout << "Cursor state: angular velocity" << std::endl;
	if (!USING_EMSCRIPTEN) sidePanelStr.angularVelocityBtn->SetToggledColor();
}

using Eigen::MatrixXd;

void InitLeftPanel() {

	float buttonWidth = .325f;

	sidePanelStr.panel = new GLPanel(.6f, 1.4f, GLPanel::vertical, .05f, .0125f, .0125f);
	sidePanelStr.panel->Translate(glm::vec3(-1.0f, -.4f, 0.0f));
	sidePanelStr.panel->SetColor(glm::vec4(.1f, .1f, .1f, 1.0f));

	sidePanelStr.sphereBtn = new GLRoundedButton(buttonWidth, .1f);
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
		pickedShape->scaling = glm::vec3(std::stof(bottomPanelStr.scaleTextX->GetText()), pickedShape->scaling.y, pickedShape->scaling.z);
	}
};

class ScaleYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->scaling = glm::vec3(pickedShape->scaling.x, std::stof(bottomPanelStr.scaleTextY->GetText()), pickedShape->scaling.z);
	}
};

class ScaleZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->scaling = glm::vec3(pickedShape->scaling.x, pickedShape->scaling.y, std::stof(bottomPanelStr.scaleTextZ->GetText()));
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
		pickedShape->rotationAxis.x = std::stof(bottomPanelStr.angularVelocityTextX->GetText());
	}
};

class AngularVelocityYonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->rotationAxis.y = std::stof(bottomPanelStr.angularVelocityTextY->GetText());
	}
};

class AngularVelocityZonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->rotationAxis.z = std::stof(bottomPanelStr.angularVelocityTextZ->GetText());
	}
};

class AngularVelocitySpeedonChange : public GLTextBox::TextChangedListener {
	void OnChange() {
		pickedShape->angularVelocity = std::stof(bottomPanelStr.angularVelocitySpeedText->GetText());
	}
};

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

	bottomPanelStr.angularVelocitySpeedLabel = new GLLabel(.22f, .1f);
	bottomPanelStr.angularVelocitySpeedLabel->SetText("Rot. Speed:");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocitySpeedLabel);

	bottomPanelStr.angularVelocitySpeedText = new GLTextBox(.2f, .1f);
	bottomPanelStr.angularVelocitySpeedText->SetText("0");
	bottomPanelStr.angularVelocityPanel->AddView(bottomPanelStr.angularVelocitySpeedText);
	bottomPanelStr.angularVelocitySpeedText->SetTextChangedListener(new AngularVelocitySpeedonChange());

	views.push_back(bottomPanel);
}

#include <Eigen/Dense>
using namespace Eigen;

float gravity = 0.0f;// -9.8f;
float timeUpdate = .001f;
int numFramesToUpdatePerCollision = 10;

bool go = false;

bool mainLoop() {
	// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
	glfwPollEvents();

	if (glfwWindowShouldClose(window)) {
		Close();

		if (USING_EMSCRIPTEN) {
			//emscripten_force_exit(0);
		}
		return false;
	}

	//Uncomment for wireframe mode:
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GLfloat currentFrame = (float)glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	if (!setup) {
		if (!go) {
			for (unsigned int i = 0; i < shapes.size(); i++) {
				shapes[i]->time = 0.0f;
			}
			go = true;
		}
		//Update position of each superquad:
		for (unsigned int i = 0; i < shapes.size(); i++) {

			if (!paused) {
				if (reverse) {
					timeUpdate = -timeUpdate;
				}
				shapes[i]->time += timeUpdate;
			}

			shapes[i]->translation = shapes[i]->centroid + (shapes[i]->curVelocity * shapes[i]->time) + glm::vec3(0.0f, .5f * gravity * shapes[i]->time * shapes[i]->time, 0.0f);

			float rotationFraction = .0005f;
			shapes[i]->rotation = glm::rotate(i4, shapes[i]->angularVelocity * rotationFraction, shapes[i]->rotationAxis) * shapes[i]->rotation;
		}
	}

	//TODO predict sphere tests
	//Find closest points/collisions
	if (!setup) {
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

				//Closest points via brute force:
				if (BoundingSphereTest(*shapes[i], *shapes[j])) {

					glm::vec3 ptI;
					glm::vec3 ptJ;

					Superquadric *si = dynamic_cast<Superquadric*>(shapes[i]);
					Superquadric *sj = dynamic_cast<Superquadric*>(shapes[j]);
					if (si && sj) {
						DrawClosestPoints(*si, *sj, i, j, ptI, ptJ);
					}

					//TODO  use squared distance instead
					float distance = glm::distance(ptI, ptJ);

					//check for collision:
					if (distance < contactAccuracy || glm::dot(shapes[i]->translation - shapes[j]->translation, ptI - ptJ) < 0.0) {
						//Change velocity, reset time, and update initial point

						Collision(*shapes[i], ptI, *shapes[j], ptJ);

						framesSinceCollision[curPair] = numFramesToUpdatePerCollision;

						shapes[i]->time = shapes[j]->time = 0.0f;

						shapes[i]->centroid = shapes[i]->translation;
						shapes[j]->centroid = shapes[j]->translation;
					}
				}
			}
		}
	}

	Render();
	return true;
}

int main()
{
	InitOpenGL();

	inverseProjection = glm::inverse(projection);

	if(!USING_EMSCRIPTEN) {
		InitSetupSceneTransform();
		InitLeftPanel();
		InitCameraControls();
		InitBottomPanel();
		InitAnimationControls();
		InitViews();
	}

	/*TEST

	MultiCollide::Quaternion q(-glm::pi<float>() / 8.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 r(glm::rotate(glm::mat4(), -glm::pi<float>() / 8.0f, glm::vec3(0.0f, 0.0f, 1.0f)));

	glm::vec3 vec(1.0f, 0.0f, 0.0f);

	glm::vec3 qVec = q.Rotate(vec);
	glm::vec3 rVec = glm::vec3(r * glm::vec4(vec.x, vec.y, vec.z, 1.0f));


	glm::mat4 a = glm::rotate(glm::mat4(), q.Angle(), q.Axis()) * i4;





	glm::mat4 b = r * i4;


	std::cout << "Test" << std::endl;

	//double m1, m2, mu, e;
	//double initialVperpu, initialVperpw, initialVnormaln;
	//Matrix3d Q1, Q2, Rot, R1, R2, P1, P2, RTAN, S, A, Identity3;
	//Vector3d v1, v2, w1, w2, r1, r2, nhat, uhat, ohat, initialVperp, initialVnormal;
	//Vector3d nhat1, uhat1, ohat1;
	//MatrixXd uwJuzhen(3, 2);
	//Matrix2d B, BNew;
	//Vector2d c;


	//m1 = 1.0; m2 = 1.0;//mass of the two objects.
	//mu = 0.25;//frictional coefficient.
	//		  //constantForStepSize=0.008;

	//e = 1.0; //coefficient of restitution

	//r1 << 1.0, 0.0, 0.0;//the vector from center of mass to the contacting point,in local coord, r1 will be modified soon.
	//r2 << -1.0, 0.0, 0.0;

	//v1 << sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.0; //initial velocity
	//v2 << 0.0, 0.0, 0.0;
	//w1 << 0.0, 0.0, 0.0;
	//w2 << 0.0, 0.0, 0.0;

	//R1 << sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.0,
	//	sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.0,
	//	0.0, 0.0, 1.0;
	//R2 << sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.0,
	//	sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.0,
	//	0.0, 0.0, 1.0;
	//RTAN << sqrt(2.0) / 2.0, -sqrt(2.0) / 2.0, 0.0,
	//	sqrt(2.0) / 2.0, sqrt(2.0) / 2.0, 0.0,
	//	0.0, 0.0, 1.0;
	//Q1 << 0.4, 0.0, 0.0,
	//	0.0, 0.4, 0.0,
	//	0.0, 0.0, 0.4;
	//Q2 << 0.4, 0.0, 0.0,
	//	0.0, 0.4, 0.0,
	//	0.0, 0.0, 0.4;

	//Impact initial(m1, m2, mu, e, R1, R2, Q1, Q2, RTAN,
	//	r1, r2, v1, v2, w1, w2);

	//initial.impact();
	*/

	if (!USING_EMSCRIPTEN) {
		while (mainLoop()); //returns false when program should stop

		return 0;
	}
	else {
		bool joke = false;
		if (joke) {
			void(*ptr)();
			ptr = AddSphereOnClick;
			ptr();
			ptr = AddSuperquadricOnClick;
			ptr();
			ptr = AddCubeOnClick;
			ptr();
			ptr = AddEllipsoidOnClick;
			ptr();

			ptr = RunOnClick;
			ptr();
			ptr = ResetOnClick;
			ptr();
			ptr = StopOnClick;
			ptr();
			ptr = PauseOnClick;
			ptr();

			ptr = TranslateOnClick;
			ptr();
			ptr = RotateOnClick;
			ptr();
			ptr = ScaleOnClick;
			ptr();
			ptr = VelocityOnClick;
			ptr();
			ptr = AngularVelocityOnClick;
			ptr();
		}
		//emscripten_set_main_loop(mainLoop, 0, 1);
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
		else if(key== GLFW_KEY_LEFT && action == GLFW_RELEASE) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	glm::vec3 update = glm::vec3(0.0f, 0.0f, -.1f * yoffset);
	if (pickedShape != nullptr) {
		if (cursorState == translate) {
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + update;

			bottomPanelStr.positionTextX->SetText(std::to_string(pickedShape->translation.x));
			bottomPanelStr.positionTextY->SetText(std::to_string(pickedShape->translation.y));
			bottomPanelStr.positionTextZ->SetText(std::to_string(pickedShape->translation.z));
		}
		else if (cursorState == rotate) {
			
		}
		else if (cursorState == scale_uniform) {
			pickedShape->scaling += yoffset * .05f;

			bottomPanelStr.scaleTextX->SetText(std::to_string(pickedShape->scaling.x));
			bottomPanelStr.scaleTextY->SetText(std::to_string(pickedShape->scaling.y));
			bottomPanelStr.scaleTextZ->SetText(std::to_string(pickedShape->scaling.z));
		}
		else if (cursorState == velocity) {
			pickedShape->curVelocity = pickedShape->curVelocity + update;

			bottomPanelStr.velocityTextX->SetText(std::to_string(pickedShape->curVelocity.x));
			bottomPanelStr.velocityTextY->SetText(std::to_string(pickedShape->curVelocity.y));
			bottomPanelStr.velocityTextZ->SetText(std::to_string(pickedShape->curVelocity.z));
		}
		else if (cursorState == angular_velocity) {
			pickedShape->rotationAxis += update;
			pickedShape->angularVelocity = sqrtf(pickedShape->rotationAxis.x * pickedShape->rotationAxis.x + pickedShape->rotationAxis.y * pickedShape->rotationAxis.y + pickedShape->rotationAxis.z * pickedShape->rotationAxis.z);
			bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->rotationAxis.x));
			bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->rotationAxis.y));
			bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->rotationAxis.z));
		}
	}

	else {
		cameraPos += update;
		cameraTarget += update;
		view = glm::lookAt(cameraPos, cameraTarget, up);
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	float xPos = (float)xpos;
	float yPos = (float)ypos;

	GLfloat xoffset = xPos - mouseXvp;
	GLfloat yoffset = mouseYvp - yPos;
	GLfloat sensitivity = 0.005f;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	if (mouseDown[0] && pickedShape != nullptr) {
		if (cursorState == translate) {
			//glm::vec3 update(4.0f * xoffset / WIDTH, 4.0f * yoffset / HEIGHT, 0.0f);

			//std::cout << update.x << update.y << std::endl;

			glm::vec3 update(xoffset, yoffset, 0.0f);
			pickedShape->centroid = pickedShape->translation = pickedShape->centroid + update;

			for (unsigned int i = 0; i < shapes.size(); i++) {
				Shape *other = shapes[i];
				if (pickedShape != other && BoundingSphereTest(*pickedShape, *other)) {

					Superquadric *picked = dynamic_cast<Superquadric*>(pickedShape);
					Superquadric *otherSq = dynamic_cast<Superquadric*>(other);
					if (picked && other) {
						
						glm::vec3 closest1;
						glm::vec3 closest2;
						ParamPoint contactPt1, contactPt2;

						Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);

						//TODO  use squared distance instead
						float distance = glm::distance(closest1, closest2);
						//Todo use consistent threshold
						float slideContactAccuracy = .015f;
						if (distance < slideContactAccuracy || glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f) {

							//Shape is now in contact with/intersecting another
							//Undo half of update:
							pickedShape->centroid = pickedShape->translation = pickedShape->centroid - .5f * update;

							Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);
							float distance = glm::distance(closest1, closest2);

							if (distance > slideContactAccuracy) {
								//Not in contact anymore
								return;
							}

							//Shape is still in contact with/intersecting another
							//Undo half of update:
							pickedShape->centroid = pickedShape->translation = pickedShape->centroid - .5f * update;

							//THis will be new point of contact on otherSq's surface
							glm::vec3 slideUpdate = Superquadric::SlideUpdate(*otherSq, contactPt2, update);
							glm::vec3 slidePt2 = contactPt2.pt + slideUpdate;


							//contactPti.pt == GLOBAL PT 


							//This is the point on pickedShape with a normal parallel to otherSq's new contact pt
							// (in global space)
							glm::vec3 pickedShapeNewContactPt = Superquadric::SlidingSurfaceUpdate(*picked, *otherSq, contactPt1, slidePt2);
							if (Superquadric::checkVecForNaN(pickedShapeNewContactPt)) {
								std::cout << "nan slide pt" << std::endl;
								return;
							}
							glm::vec3 pickedShapeTrans = Superquadric::getGlobalCoordinates(slidePt2, otherSq->translation, otherSq->rotation) - pickedShapeNewContactPt;


							std::cout << "Original Move: " << update.x << ", " << update.y << ", " << update.z << std::endl;
							std::cout << "Slide update: " << slideUpdate.x << ", " << slideUpdate.y << ", " << slideUpdate.z << std::endl;
							std::cout << "Slide trans: " << pickedShapeTrans.x << ", " << pickedShapeTrans.y << ", " << pickedShapeTrans.z << std::endl;
							

							pickedShape->centroid = pickedShape->translation = pickedShape->centroid + pickedShapeTrans;

							Superquadric::ClosestPointFramework(*picked, *otherSq, closest1, closest2, contactPt1, contactPt2);
							distance = glm::distance(closest1, closest2);
							if (distance < contactAccuracy || glm::dot(picked->translation - otherSq->translation, closest1 - closest2) < 0.0f) {
								pickedShape->centroid = pickedShape->translation = pickedShape->centroid - slideUpdate;
							}
						}
						else {
							std::cout << "Good" << std::endl;
						}

					}
				}
			}

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.positionTextX->SetText("" + std::to_string(pickedShape->translation.x));
				bottomPanelStr.positionTextY->SetText("" + std::to_string(pickedShape->translation.y));
				bottomPanelStr.positionTextZ->SetText("" + std::to_string(pickedShape->translation.z));
			}
		}
		else if (cursorState == rotate) {
			//pickedShape->rotation

			//MultiCollide::Quaternion orientation(pickedShape->rotation);
			MultiCollide::Quaternion update( .035f, glm::vec3(-yoffset, xoffset, 0.0f));

			bool inContact = false;

			for (unsigned int i = 0; i < shapes.size(); i++) {
				Shape *other = shapes[i];
				if (pickedShape != other && BoundingSphereTest(*pickedShape, *other)) {

					Superquadric *picked = dynamic_cast<Superquadric*>(pickedShape);
					Superquadric *otherSq = dynamic_cast<Superquadric*>(other);
					if (picked && other) {

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

							pickedShape->rotation = glm::rotate(i4, update.Angle(), update.Axis()) * pickedShape->rotation;

							glm::vec3 newPtGlobal = Superquadric::getGlobalCoordinates(pickedShapeNewContactPt, pickedShape->translation, pickedShape->rotation);

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
				glm::mat4 a = glm::rotate(glm::mat4(), update.Angle(), update.Axis()) * pickedShape->rotation;
				glm::mat4 b = glm::rotate(i4, .075f, glm::vec3(-yoffset, xoffset, 0.0f)) * pickedShape->rotation;

				pickedShape->rotation = glm::rotate(i4, .075f, glm::vec3(-yoffset, xoffset, 0.0f)) * pickedShape->rotation;
			}
		}
		else if (cursorState == scale_uniform) {
			//TODO - uniform & directional options?
			
			pickedShape->scaling += yoffset;

			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.scaleTextX->SetText("" + std::to_string(pickedShape->scaling.x));
				bottomPanelStr.scaleTextY->SetText("" + std::to_string(pickedShape->scaling.y));
				bottomPanelStr.scaleTextZ->SetText("" + std::to_string(pickedShape->scaling.z));
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
		else if (cursorState == angular_velocity) {
			pickedShape->rotationAxis += glm::vec3(xoffset, yoffset, 0.0f);
			pickedShape->angularVelocity = sqrtf(pickedShape->rotationAxis.x * pickedShape->rotationAxis.x + pickedShape->rotationAxis.y * pickedShape->rotationAxis.y + pickedShape->rotationAxis.z * pickedShape->rotationAxis.z);
			if (!USING_EMSCRIPTEN) {
				bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->rotationAxis.x));
				bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->rotationAxis.y));
				bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->rotationAxis.z));
				//TODO update speed
			}
		}
	}

	mouseXvp = xPos;
	mouseYvp = yPos;

	mouseX = xPos / WIDTH;
	mouseX = ((mouseX - .5f) / .5f);

	mouseY = yPos / HEIGHT;
	mouseY = ((mouseY - .5f) / -.5f);


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
			glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(view) * rayView));

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

			if (pickedShape != nullptr) {
				pickedShape->objectColor = pickedShape->defaultColor;
			}
			pickedShape = nullptr;
			float minDistance = 1000000.0f;

			for (std::vector<Shape*>::iterator it = shapes.begin(); it != shapes.end(); it++) {
				Shape *shape = (*it);
				glm::vec3 C = shape->centroid;
				float r = shape->boundingSphereRadius - shape->BoundingSphereBuffer;
				glm::vec3 OminusC = cameraPos - C;
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
					}

				}
			}

			if (pickedShape != nullptr) {
				std::cout << "Picked shape: " << pickedShape->name << std::endl;
				pickedShape->objectColor = selectedColor;
				if (!USING_EMSCRIPTEN) {
					bottomPanelStr.panel->ShowContent();

					bottomPanelStr.positionTextX->SetText("" + std::to_string(pickedShape->translation.x));
					bottomPanelStr.positionTextY->SetText("" + std::to_string(pickedShape->translation.y));
					bottomPanelStr.positionTextZ->SetText("" + std::to_string(pickedShape->translation.z));

					bottomPanelStr.velocityTextX->SetText("" + std::to_string(pickedShape->curVelocity.x));
					bottomPanelStr.velocityTextY->SetText("" + std::to_string(pickedShape->curVelocity.y));
					bottomPanelStr.velocityTextZ->SetText("" + std::to_string(pickedShape->curVelocity.z));

					bottomPanelStr.angularVelocityTextX->SetText("" + std::to_string(pickedShape->rotationAxis.x));
					bottomPanelStr.angularVelocityTextY->SetText("" + std::to_string(pickedShape->rotationAxis.y));
					bottomPanelStr.angularVelocityTextZ->SetText("" + std::to_string(pickedShape->rotationAxis.z));
					bottomPanelStr.angularVelocitySpeedText->SetText("" + std::to_string(pickedShape->angularVelocity));
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
	glm::dvec3 centroid = mesh.MeshCentroid;
	typedef std::vector<Tetrahedron>::iterator iter;
	for (iter it = mesh.Tetrahedra.begin(); it != mesh.Tetrahedra.end(); it++) {
		float distA = (float)glm::distance(centroid, it->a);
		float distB = (float)glm::distance(centroid, it->b);
		float distC = (float)glm::distance(centroid, it->c);
		float distD = (float)glm::distance(centroid, it->d);
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

void AddPoint(std::vector<GLfloat> &vector, glm::vec3 pt, glm::vec3 normal) {
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
void DrawClosestPoints(Superquadric &sq1, Superquadric &sq2, int sq1index, int sq2index, glm::vec3 &closest1, glm::vec3 &closest2) {
	//GLuint VAOdist, VBOdist;
	//std::vector<GLfloat> vertices;
	//shapeShader.Use();

	if (previouses[sq1index][sq2index].p1.u < -10.f) {
		Superquadric::ClosestPointFramework(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
	}
	else {
		Superquadric::ClosestPointFrameworkTryPrevious(sq1, sq2, closest1, closest2, previouses[sq1index][sq2index].p1, previouses[sq1index][sq2index].p2);
	}

	if (sq2index > 10) {
		std::cout << "Hey" << std::endl;
	}

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
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(lineModel));
	glUniform3f(objectColorLoc, color.x, color.y, color.z);
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

void Collision(Shape &s1, glm::vec3 contactPt1, Shape &s2, glm::vec3 contactPt2) {

	glm::dmat3 R1 = glm::mat3(s1.rotation);
	glm::dmat3 R2 = glm::mat3(s2.rotation);

	//Todo get velocity updated by time
	// -- func

	//TODO Q matrices...?
	glm::dmat3 RTAN, Q1, Q2;
	glm::dvec3 r1, r2, v1, v2, w1, w2;

	//B, my graphics coordinate basis, is (x, y, z)
	//B', the basis for impact analysis is  with z' in my y position, y' in my x spot, and x' in my z 
	//Tprime = T': B' -> B
	//glm::mat3 Tprime = glm::mat3(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	//glm::mat3 Tprime = glm::mat3(glm::rotate(glm::mat4(), glm::pi<float>() / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(), glm::pi<float>() / 2.0f, glm::vec3(0.0f, 0.0f, 1.0f)));

	//Basis for contact pt, (a, b, c), c is normal to contact plane
	Superquadric* sq = dynamic_cast<Superquadric*>(&s1);

	//TODO RTAN for other shapes..
	if (sq) {
		glm::vec3 normLocal = sq->unitnormal(Superquadric::getLocalCoordinates(contactPt1, sq->translation, sq->rotation));
		glm::vec3 normGlobal = glm::vec3(sq->rotation * glm::vec4(normLocal.x, normLocal.y, normLocal.z, 1.0));

		//Rotate y to contact normal
		RTAN = glm::mat3(ShapeUtils::rotationFromAtoB(glm::vec3(1.0, 0.0, 0.0), normGlobal));
	}

	r1 = Superquadric::getLocalCoordinates(contactPt1, s1.translation, s1.rotation);
	r2 = Superquadric::getLocalCoordinates(contactPt2, s2.translation, s2.rotation);
	v1 = s1.curVelocity;
	v2 = s2.curVelocity;
	w1 = s1.angularVelocity * s1.rotationAxis;
	w2 = s2.angularVelocity * s2.rotationAxis;

	////EX 1:
	//r1 = glm::dvec3(1.0, 0.0, 0.0);
	//r2 = glm::dvec3(-1.0, 0.0, 0.0);
	//v1 = glm::dvec3(1.0, 0.0, 0.0);
	//v2 = glm::dvec3(0.0, 0.0, 0.0);
	//w1 = w2 = glm::dvec3(0.0f);
	//R1 = R2 = RTAN = glm::dmat3();

	Impact impact1(s1.mass, s2.mass, s1.frictionCoefficient, s1.restitutionCoefficient, R1, R2,
		Q1, Q2, RTAN, r1, r2, v1, v2, w1, w2);

	ImpactOutput result1 = impact1.impact();

	s1.curVelocity = result1.v1end;
	s2.curVelocity = result1.v2end;

	////EX 2:
	//r1 = glm::vec3(1.0, 0.0f, 0.0);
	//r2 = glm::vec3(-1.0, 0.0, 0.0);
	//v1 = glm::vec3(1.0, 0.0, 0.0);
	//w1 = glm::vec3(0.0, 1.0, 0.0);
	//v2 = w2 = glm::vec3(0.0);
	//R1 = R2 = RTAN = glm::mat3();

	//Impact impact2(s1.mass, s2.mass, s1.frictionCoefficient, s1.restitutionCoefficient, R1, R2,
	//	Q1, Q2, RTAN, r1, r2, v1, v2, w1, w2);

	//Impact::ImpactOutput result2 = impact2.impact();

	//s1.curVelocity = result2.v1end;
	//s2.curVelocity = result2.v2end;

	////EX 3:
	//r1 = glm::vec3(1.0f, 0.0f, 0.0f);
	//r2 = glm::vec3(-1.0f, 0.0f, 0.0f);
	//double sqrt2 = sqrt(2.0) / 2.0;
	//v1 = glm::dvec3(sqrt2, sqrt2, 0.0);
	//v2 = w1 = w2 = glm::vec3(0.0f);
	//R1 = R2 = RTAN = glm::dmat3(glm::rotate(glm::dmat4(), glm::pi<double>() / 4.0f, glm::dvec3(0.0f, 0.0f, 1.0f)));
	//Q1 = Q2 = glm::mat3(1.0f);

	//Impact impact(s1.mass, s2.mass, s1.frictionCoefficient, s1.restitutionCoefficient, R1, R2,
	//	Q1, Q2, RTAN, r1, r2, v1, v2, w1, w2);
	//
	//Impact::ImpactOutput result = impact.impact();

	//s1.curVelocity = result.v1end;
	//s2.curVelocity = result.v2end;
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

	glLineWidth(3.0f);

	// Build and compile our shader program
	if (!USING_EMSCRIPTEN) {
		shapeShader = Shader("vertexShader.glsl", "fragmentShader.glsl");
	}
	else {
		shapeShader = Shader::ShaderFromCode("#version 100 \n attribute vec3 position;\n attribute vec3 normal;\n uniform mat4 model;\n uniform mat4 view;\n uniform mat4 projection;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n void main()\n {\n Normal = mat3(model) * normal;\n FragPos = vec3(model * vec4(position, 1.0));\n	if(position.y == 0.0){\n		Line = vec3(1.0, 1.0, 1.0);\n	}\n	else {\n		Line = vec3(-1.0, -1.0, -1.0);\n	}\n   gl_Position = projection * view * model * vec4(position, 1.0);\n }\n",
			"#version 100\n precision mediump float;\n varying vec3 Normal;\n varying vec3 FragPos;\n varying vec3 Line;\n  uniform vec3 lightPos;\n uniform vec3 objectColor;\n void main()\n {\n vec3 lightColor = vec3(1.0, 1.0, 1.0);\n float ambientStrength = 0.5;\n  vec3 ambient = ambientStrength * lightColor;\n     vec3 norm = normalize(Normal);\n    vec3 lightDir = normalize(lightPos - FragPos);\n    float diff = max(dot(norm, lightDir), 0.0);\n    vec3 diffuse = diff * lightColor;\n    vec3 result = (ambient + diffuse) * objectColor;\n    gl_FragColor = vec4(result, 1.0);\n if(objectColor.x + objectColor.y + objectColor.z < .00001) {\n 		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n 	}\n }\n");
	}

	modelLoc = glGetUniformLocation(shapeShader.Program, "model");
	viewLoc = glGetUniformLocation(shapeShader.Program, "view");
	projLoc = glGetUniformLocation(shapeShader.Program, "projection");
	lightPosLoc = glGetUniformLocation(shapeShader.Program, "lightPos");
	objectColorLoc = glGetUniformLocation(shapeShader.Program, "objectColor");
	if(!USING_EMSCRIPTEN)
		sceneTransformLoc = glGetUniformLocation(shapeShader.Program, "sceneTransform");
}

void InitSetupSceneTransform() {
	sceneTransform = glm::translate(sceneTransform, glm::vec3(.3f, .3f, 0.0f));
	sceneTransform = glm::scale(sceneTransform, glm::vec3(.7f, .7f, .7f));
	inverseSceneTransform = glm::inverse(sceneTransform);
}

void InitCameraControls() {

	//TODO put inside transparent panel

	GLView *upBtn = new GLView(.025f, .075f);
	upBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	upBtn->Translate(glm::vec3(.85f, .85f, 0.0f));
	upBtn->SetClickListener(CameraUpOnClick);
	views.push_back(upBtn);

	GLView *leftBtn = new GLView(.075f, .025f);
	leftBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	leftBtn->Translate(glm::vec3(.85f - .075f, .85f -  .025f, 0.0f));
	leftBtn->SetClickListener(CameraLeftOnClick);
	views.push_back(leftBtn);

	GLView *downBtn = new GLView(.025f, .075f);
	downBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	downBtn->Translate(glm::vec3(.85f, .85f - .075f - .025f, 0.0f));
	downBtn->SetClickListener(CameraDownOnClick);
	views.push_back(downBtn);

	GLView *rightBtn = new GLView(.075f, .025f);
	rightBtn->SetColor(glm::vec4(.9f, .9f, .9f, 1.0f));
	rightBtn->Translate(glm::vec3(.85f + .025f, .85f - .025f, 0.0f));
	rightBtn->SetClickListener(CameraRightOnClick);
	views.push_back(rightBtn);

}

void InitViews() {
	for (std::vector<GLView*>::iterator it = views.begin(); it != views.end(); it++) {
		(*it)->InitBuffers();
	}
}

void PositionShapeToAdd(Shape *shape) {

	bool valid = false;

	while(!valid){

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

void Render() {

	shapeShader.Use();
	
	// Clear the colorbuffer
	glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 lightPos(0.0f, 3.0f, 3.0f);

	glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	if(!USING_EMSCRIPTEN)
		glUniformMatrix4fv(sceneTransformLoc, 1, GL_FALSE, glm::value_ptr(sceneTransform));

	//Draw each superquadric based on its own translation && rotation
	for (std::vector<Shape*>::iterator it = shapes.begin(); it != shapes.end(); it++) {
		Shape *shape = (*it);

		shape->model = glm::scale((glm::translate(glm::mat4(), shape->translation) * shape->rotation), shape->scaling);
		//shape->model = glm::translate(shape->rotation * (glm::scale(i4, shape->scaling)), shape->translation);

		glUniform3f(objectColorLoc, shape->objectColor.x, shape->objectColor.y, shape->objectColor.z);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(shape->model));
		
		shape->Draw(shapeShader);

		//Draw velociy:
		if (setup) {

			shape->DrawInitialVelocity(shapeShader);

			if (cursorState == angular_velocity) {
				shape->DrawInitialAngularVelocity(shapeShader);
			}
		}
	}

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

int getNumShapes() {
	return shapes.size();
}

float getShapeAttribute(unsigned int shapeIndex, std::string attribute) {
	if (shapeIndex < 0 || shapeIndex >= shapes.size()) {
		std::cout << "Not a valid shape index" << std::endl;
		return NAN;
	}

	Shape *shape = shapes[shapeIndex];
	if (attribute == "positionx") {
		return shape->translation.x;
	}
	else if (attribute == "positiony") {
		return shape->translation.y;
	}
	else if (attribute == "positionz") {
		return shape->translation.z;
	}
	else if (attribute == "velocityx") {
		return shape->curVelocity.x;
	}
	else if (attribute == "velocityy") {
		return shape->curVelocity.y;
	}
	else if (attribute == "velocityz") {
		return shape->curVelocity.z;
	}
	else if (attribute == "angularvelocityx") {
		return shape->rotationAxis.x;
	}
	else if (attribute == "angularvelocityy") {
		return shape->rotationAxis.y;
	}
	else if (attribute == "angularvelocityz") {
		return shape->rotationAxis.z;
	}
	else if (attribute == "mass") {
		return shape->mass;
	}
	else if (attribute == "friction") {
		return shape->frictionCoefficient;
	}
	else if (attribute == "restitution") {
		return shape->restitutionCoefficient;
	}
	else if (attribute == "scale") {
		return shape->scaling.x; // only allowing uniform scaling so should be the same for each component
	}
	else {
		return 0.0;
	}
}

void setShapeAttribute(unsigned int shapeIndex, std::string attribute, float value) {
	if (shapeIndex < 0 || shapeIndex >= shapes.size()) {
		std::cout << "Not a valid shape index" << std::endl;
		return;
	}

	Shape *shape = shapes[shapeIndex];
	if (attribute == "positionx") {
		shape->translation.x = value;
	}
	else if (attribute == "positiony") {
		shape->translation.y = value;
	}
	else if (attribute == "positionz") {
		shape->translation.z = value;
	}
	else if (attribute == "velocityx") {
		shape->curVelocity.x = value;
	}
	else if (attribute == "velocityy") {
		shape->curVelocity.y = value;
	}
	else if (attribute == "velocityz") {
		shape->curVelocity.z = value;
	}
	else if (attribute == "angularvelocityx") {
		shape->rotationAxis.x = value;
	}
	else if (attribute == "angularvelocityy") {
		shape->rotationAxis.y = value;
	}
	else if (attribute == "angularvelocityz") {
		shape->rotationAxis.z = value;
	}
	else if (attribute == "mass") {
		shape->mass = value;
	}
	else if (attribute == "friction") {
		shape->frictionCoefficient = value;
	}
	else if (attribute == "restitution") {
		shape->restitutionCoefficient = value;
	}
	else if (attribute == "scale") {
		shape->scaling.x = shape->scaling.y = shape->scaling.z = value;
	}
}

std::string getShapeName(unsigned int shapeIndex) {
	if (shapeIndex < 0 || shapeIndex >= shapes.size()) {
		std::cout << "Not a valid shape index in getShapeName" << std::endl;
		return "NOT VALID SHAPE INDEX";
	}

	return shapes[shapeIndex]->name;
}

void removeShape(unsigned int index) {
	Shape *shapeToDelete = shapes[index];
	shapes.erase(shapes.begin() + index);

	if (pickedShape == shapeToDelete) {
		pickedShape = nullptr;
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
	superquad->e2 = e1;  // .3f  1.0f  2.0f  3.0f
	superquad->u1 = -glm::pi<float>() / 2.0f;
	superquad->u2 = glm::pi<float>() / 2.0f;
	superquad->v1 = -glm::pi<float>();
	superquad->v2 = glm::pi<float>();
	superquad->u_segs = 30;
	superquad->v_segs = 30;
	static int superquadNum = 1;
	superquad->name = std::string("Custom " + std::to_string(superquadNum++));

	AddSuperquadric(superquad);
}