#include <iostream>
#include <fstream>
#include <vector>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libs
#include <SOIL.h>
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
#include "Shader.h"
#include "TetrahedralMesh.h"
#include "Cylinder.h"

// Function prototypes
void convertGlmMatToEigen(glm::dmat3 &glmMat, Eigen::MatrixXd &eigenMatOut);
float maxDistanceFromCentroid(TetrahedralMesh &mesh);
void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals);
bool verifyOrthogonalVecs(glm::dvec3 a, glm::dvec3 b, glm::dvec3 c);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;      // Time of last frame

//Matrices:
glm::mat4 model;
glm::mat4 view = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);

using Eigen::MatrixXd;

const bool ELEPHANT = false;
const bool CUSTOM = true;
const bool CUBE = true;
const bool REGULAR = true;
const bool DOUBLEREG = true;

int main()
{

#pragma region OpenGLinit
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Pencil Motion", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

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
	Shader bareShader("vertexShaderBare.glsl", "fragmentShaderBare.glsl");

#pragma endregion

#pragma region MeshSetup
	TetrahedralMesh mesh;
	TetrahedralMesh testMesh;
	if (CUBE) {
		if (!REGULAR) {
			glm::dvec3 topBackRight(.5f, .5f, -.5f);
			glm::dvec3 topFrontRight(.5f, .5f, .5f);
			glm::dvec3 topFrontLeft(-.5f, .5f, .5f);
			glm::dvec3 topBackLeft(-.5f, .5f, -.5f);
			glm::dvec3 botBackRight(.5f, -.5f, -.5f);
			glm::dvec3 botFrontRight(.5f, -.5f, .5f);
			glm::dvec3 botFrontLeft(-.5f, -.5f, .5f);
			glm::dvec3 botBackLeft(-.5f, -.5f, -.5f);

			Tetrahedron topRight(topBackRight, topFrontRight, topBackLeft, botBackRight);
			Tetrahedron topLeft(topFrontLeft, topBackLeft, botFrontLeft, topFrontRight);
			Tetrahedron botRight(botBackRight, topFrontRight, botFrontLeft, botFrontRight);
			Tetrahedron botLeft(botBackLeft, botBackRight, botFrontLeft, topBackLeft);
			Tetrahedron middle(topFrontRight, topBackLeft, botFrontLeft, botBackRight);

			topRight.Translate(glm::dvec3(.1f, .1f, -.1f));
			topLeft.Translate(glm::dvec3(-.1f, .1f, .1f));
			botLeft.Translate(glm::dvec3(-.1f, -.1f, -.1f));
			botRight.Translate(glm::dvec3(.1f, -.1f, .1f));

			//split:
			glm::dvec3 botMiddle(0.0f, -0.5f, 0.0f);
			Tetrahedron botRightA(botBackRight, topFrontRight, botMiddle, botFrontRight);
			Tetrahedron botRightB(botMiddle, topFrontRight, botFrontLeft, botFrontRight);

			mesh.AddTetrahedron(topRight);
			mesh.AddTetrahedron(topLeft);
			mesh.AddTetrahedron(botLeft);
			mesh.AddTetrahedron(botRight);
			mesh.AddTetrahedron(middle);
			//mesh.AddTetrahedron(botRightB);

			testMesh.AddTetrahedron(topRight);
			testMesh.AddTetrahedron(topLeft);
			testMesh.AddTetrahedron(botLeft);
			testMesh.AddTetrahedron(botRight);
		}
		else {

			//Regular:
			glm::dvec3 reg1(0.0f, 0.0f, sqrt((2.0 / 3.0)) - 1.0f / (2.0f*sqrt(6.0)));
			glm::dvec3 reg2(-1.0f / (2.0f*sqrt(3.0)), -.5f, -1.0f / (2.0f*sqrt(6.0)));
			glm::dvec3 reg3(-1.0f / (2.0f*sqrt(3.0)), .5f, -1.0f / (2.0f*sqrt(6.0)));
			glm::dvec3 reg4(1.0f / (sqrt(3.0)), 0.0f, -1.0f / (2.0f*sqrt(6.0)));

			reg1 = glm::dvec3(.15, .0, .0);
			reg2 = glm::dvec3(-.15, .0, .0);
			reg3 = glm::dvec3(0.0, sqrt(.0675), .0);
			reg4 = glm::dvec3(0.0, .0866025404, .2704163457);
			Tetrahedron reg(reg1, reg2, reg3, reg4);
			//mesh.Translate(cent);
			//reg.Translate(glm::dvec3(.55f, .55f, .55f));
			mesh.AddTetrahedron(reg);
			if (DOUBLEREG){
				//Tetrahedron invertedReg(-reg1, -reg2, -reg3, -reg4); looks cool
				Tetrahedron invertedReg(reg1, reg2, reg3, glm::dvec3(0.0, .0866025404, -.2704163457));
				mesh.AddTetrahedron(invertedReg);
			}
		}

		//testMesh.ComputeMeshAttributes();
		mesh.ComputeMeshAttributes();
		double doubleVol = mesh.Volume;
		if (doubleVol > 0){
			doubleVol += 22.0f;
		}


	}
#pragma region CUSTOM
	else if(CUSTOM){
		//Mesh vertices
		glm::dvec3 a1(-.3f, 0.0f, 0.0f);
		glm::dvec3 a2(.3f, 0.0f, 0.0f);
		glm::dvec3 b(0.0, 0.0, 0.0);
		glm::dvec3 c1(0.0f, 0.0f, -.3f);
		glm::dvec3 c2(0.0f, 0.0f, .3f);
		glm::dvec3 d1(0.0f, .3f, 0.0f);
		glm::dvec3 d2(0.0f, -.3f, 0.0f);

		Tetrahedron t1(a1, b, c1, d1);
		t1.normalABD = glm::dvec3(0.0f, 0.0f, 1.0f);
		t1.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t1.normalCDB = glm::dvec3(1.0f, 0.0f, 0.0f);
		t1.normalACD = glm::dvec3(-.333f, .333f, -.333f);

		Tetrahedron t2(a2, b, c1, d1);
		t2.normalABD = glm::dvec3(0.0f, 0.0f, 1.0f);
		t2.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t2.normalCDB = glm::dvec3(-1.0f, 0.0f, 0.0f);
		t2.normalACD = glm::dvec3(.333f, .333f, -.333f);

		Tetrahedron t3(a1, b, c2, d1);
		t3.normalABD = glm::dvec3(0.0f, 0.0f, -1.0f);
		t3.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t3.normalCDB = glm::dvec3(1.0f, 0.0f, 0.0f);
		t3.normalACD = glm::dvec3(-.333f, .333f, .333f);

		Tetrahedron t4(a2, b, c2, d1);
		t4.normalABD = glm::dvec3(0.0f, 0.0f, -1.0f);
		t4.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t4.normalCDB = glm::dvec3(-1.0f, 0.0f, 0.0f);
		t4.normalACD = glm::dvec3(.333f, .333f, .333f);

		Tetrahedron t5(a1, b, c1, d2);
		t1.normalABD = glm::dvec3(0.0f, 0.0f, 1.0f);
		t1.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t1.normalCDB = glm::dvec3(1.0f, 0.0f, 0.0f);
		t1.normalACD = glm::dvec3(-.333f, -.333f, -.333f);

		Tetrahedron t6(a2, b, c1, d2);
		t2.normalABD = glm::dvec3(0.0f, 0.0f, 1.0f);
		t2.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t2.normalCDB = glm::dvec3(-1.0f, 0.0f, 0.0f);
		t2.normalACD = glm::dvec3(.333f, -.333f, -.333f);

		Tetrahedron t7(a1, b, c2, d2);
		t3.normalABD = glm::dvec3(0.0f, 0.0f, -1.0f);
		t3.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t3.normalCDB = glm::dvec3(1.0f, 0.0f, 0.0f);
		t3.normalACD = glm::dvec3(-.333f, -.333f, .333f);

		Tetrahedron t8(a2, b, c2, d2);
		t4.normalABD = glm::dvec3(0.0f, 0.0f, -1.0f);
		t4.normalABC = glm::dvec3(0.0f, -1.0f, 0.0f);
		t4.normalCDB = glm::dvec3(-1.0f, 0.0f, 0.0f);
		t4.normalACD = glm::dvec3(.333f, -.333f, .333f);
	
	
		if (CUSTOM){
			mesh.AddTetrahedron(t1);
			mesh.AddTetrahedron(t2);
			mesh.AddTetrahedron(t3);
			mesh.AddTetrahedron(t4);
			mesh.AddTetrahedron(t5);
			mesh.AddTetrahedron(t6);
			mesh.AddTetrahedron(t7);
			mesh.AddTetrahedron(t8);
			//mesh.Translate(glm::dvec3(.25f, .25f, -.25f));
			//mesh.doubleTetrahedra();
			//mesh.doubleTetrahedra();
		}
	}
#pragma endregion
	else if (!ELEPHANT && !CUSTOM){
		mesh = Cylinder::MeshCylinder(.5f, 1.0f);
	}
	Cylinder cyl(.5f, 1.0f);
	bool cone = false;
	if (ELEPHANT){
		TetrahedralMesh::BuildFromFile("cone.mesh", mesh); // cone.mesh  octahedron.mesh 
		cone = true;

		//TetrahedralMesh::BuildFromFile("out_2.mesh", mesh);
		//TetrahedralMesh::BuildFromFile("icosahedron.mesh", mesh);
	}
	mesh.ComputeMeshAttributes();

	float maxDist = maxDistanceFromCentroid(mesh);

	int numTris = mesh.GetTetrahedraCount() * 4; // Each tetrahedron consists of 4 triangles
	std::vector<GLfloat> vertices;
	if (ELEPHANT || CUSTOM){
		mesh.GetGlVertices(vertices);
	}
	//std::vector<float> 
	else{
		//vertices = cyl.vertices;
	}
	//mesh.GetGlVertices(vertices);

#pragma endregion

#pragma region PrincipalFrameComputation
	MatrixXd angularInertia(3, 3);
	convertGlmMatToEigen(mesh.AngularInertia, angularInertia);
	Eigen::ComplexEigenSolver<MatrixXd> ces(angularInertia);
	typedef Eigen::ComplexEigenSolver<MatrixXd>::EigenvectorType EigenVec;
	typedef Eigen::ComplexEigenSolver<MatrixXd>::EigenvalueType EigenVal;
	EigenVec eigenVec1 = ces.eigenvectors().col(0);
	EigenVec eigenVec2 = ces.eigenvectors().col(1);
	EigenVec eigenVec3 = ces.eigenvectors().col(2);
	float eigenVal1 = ces.eigenvalues().col(0).row(0).real().value();
	float eigenVal2 = ces.eigenvalues().col(0).row(1).real().value();
	float eigenVal3 = ces.eigenvalues().col(0).row(2).real().value();

	glm::dvec3 eigenVecA(eigenVec1.row(0).real().value(), eigenVec1.row(1).real().value(), eigenVec1.row(2).real().value());
	glm::dvec3 eigenVecB(eigenVec2.row(0).real().value(), eigenVec2.row(1).real().value(), eigenVec2.row(2).real().value());
	glm::dvec3 eigenVecC(eigenVec3.row(0).real().value(), eigenVec3.row(1).real().value(), eigenVec3.row(2).real().value());

	verifyOrthogonalVecs(eigenVecA, eigenVecB, eigenVecC);

	std::vector<double> normalizedEvals;
	normalizeEigenVals(eigenVal1, eigenVal2, eigenVal3, maxDist, normalizedEvals);
	
	//scale by eigenvals
	eigenVecA = eigenVecA * normalizedEvals[0];
	eigenVecB = eigenVecB * normalizedEvals[1];
	eigenVecC = eigenVecC * normalizedEvals[2];

	glm::dvec3 centroid = mesh.MeshCentroid;

	//Need to translate eigenvectors to mesh centroid
	eigenVecA += centroid;
	eigenVecB += centroid;
	eigenVecC += centroid;

	GLfloat frameVertices[] = {
		eigenVecA.x, eigenVecA.y, eigenVecA.z, 1.0, 1.0, 1.0,
		centroid.x, centroid.y, centroid.z, 1.0, 1.0, 1.0,
		//-eigenVecA.x, -eigenVecA.y, -eigenVecA.z, 1.0, 1.0, 1.0,

		eigenVecB.x, eigenVecB.y, eigenVecB.z, 1.0, 1.0, 1.0,
		centroid.x, centroid.y, centroid.z, 1.0, 1.0, 1.0,
		//-eigenVecB.x, -eigenVecB.y, -eigenVecB.z, 1.0, 1.0, 1.0,

		eigenVecC.x, eigenVecC.y, eigenVecC.z, 1.0, 1.0, 1.0,
		centroid.x, centroid.y, centroid.z, 1.0, 1.0, 1.0
		//-eigenVecC.x, -eigenVecC.y, -eigenVecC.z, 1.0, 1.0, 1.0,
	};

	//Arrows (tetrahedra for now):
	TetrahedralMesh arrows;
	//A:
	const double TH = .05f;
	glm::dvec3 pfA_Pt = eigenVecA * (1.0 / (1.0- 2.0*TH));//(eigenVecA.x + TH, eigenVecA.y + TH, eigenVecA.z + TH);
	glm::dvec3 orthogEigenVecAxy = glm::dvec3(eigenVecA.y, -eigenVecA.x, eigenVecA.z) * TH;
	glm::dvec3 orthogEigenVecAxz = glm::dvec3(-eigenVecA.z, eigenVecA.y, eigenVecA.x) * TH;
	if (abs(glm::dot(glm::normalize(eigenVecA), glm::normalize(orthogEigenVecAxy)) - 1.0) < .00001){
		int ok = 0;
		//not orthog
		orthogEigenVecAxy = glm::dvec3(eigenVecA.x, -eigenVecA.z, eigenVecA.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecA), glm::normalize(orthogEigenVecAxy)) - 1.0) < .00001){
			int ok = 0;
		}
	}
	if (abs(glm::dot(glm::normalize(eigenVecA), glm::normalize(orthogEigenVecAxz)) - 1.0) < .00001){
		int ok = 0;
		//not orthog
		orthogEigenVecAxz = glm::dvec3(eigenVecA.x, -eigenVecA.z, eigenVecA.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecA), glm::normalize(orthogEigenVecAxz)) - 1.0) < .00001){
			int ok = 0;
		}
	}
	
	glm::dvec3 pfA_a = eigenVecA + orthogEigenVecAxy;
	glm::dvec3 pfA_b = eigenVecA - orthogEigenVecAxy;
	glm::dvec3 pfA_c = eigenVecA + orthogEigenVecAxz;
	glm::dvec3 pfA_d = eigenVecA - orthogEigenVecAxz;
	
	/*glm::dvec3 pfA_b(eigenVecA.x + orthogEigenVecA.x, eigenVecA.y, eigenVecA.z);
	glm::dvec3 pfA_c(eigenVecA.x, eigenVecA.y + orthogEigenVecA.y, eigenVecA.z);
	glm::dvec3 pfA_d(eigenVecA.x, eigenVecA.y, eigenVecA.z + orthogEigenVecA.z);*/
	glm::dvec3 arrowNorm(1.0, 1.0, 1.0); //TODO
	Tetrahedron arrowA_t1(pfA_Pt, pfA_a, pfA_b, pfA_c);
	Tetrahedron arrowA_t2(pfA_Pt, pfA_a, pfA_b, pfA_d);
	arrows.AddTetrahedron(arrowA_t1);
	arrows.AddTetrahedron(arrowA_t2);

	//B:
	glm::dvec3 pfB_Pt = eigenVecB * (1.0f / (1.0f - 2.0f*TH));//(eigenVecA.x + TH, eigenVecA.y + TH, eigenVecA.z + TH);
	glm::dvec3 orthogEigenVecBxy = glm::dvec3(eigenVecB.y, -eigenVecB.x, eigenVecB.z) * TH;
	if (abs(glm::dot(glm::normalize(eigenVecB), glm::normalize(orthogEigenVecBxy)) - 1.0f) < .00001f){
		int ok = 0;
		//not orthog
		orthogEigenVecBxy = glm::dvec3(eigenVecB.x, -eigenVecB.z, eigenVecB.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecB), glm::normalize(orthogEigenVecBxy)) - 1.0f) < .00001f){
			int ok = 0;
		}
	}
	glm::dvec3 orthogEigenVecBxz = glm::dvec3(-eigenVecB.z, eigenVecB.y, eigenVecB.x) * TH;
	if (abs(glm::dot(glm::normalize(eigenVecB), glm::normalize(orthogEigenVecBxz)) - 1.0f) < .00001f){
		int ok = 0;
		//not orthog
		orthogEigenVecBxz = glm::dvec3(eigenVecB.x, -eigenVecB.z, eigenVecB.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecB), glm::normalize(orthogEigenVecBxz)) - 1.0f) < .00001f){
			int ok = 0;
		}
	}

	glm::dvec3 pfB_a = eigenVecB + orthogEigenVecBxy;
	glm::dvec3 pfB_b = eigenVecB - orthogEigenVecBxy;
	glm::dvec3 pfB_c = eigenVecB + orthogEigenVecBxz;
	glm::dvec3 pfB_d = eigenVecB - orthogEigenVecBxz;

	Tetrahedron arrowB_t1(pfB_Pt, pfB_a, pfB_b, pfB_c);
	Tetrahedron arrowB_t2(pfB_Pt, pfB_a, pfB_b, pfB_d);
	arrows.AddTetrahedron(arrowB_t1);
	arrows.AddTetrahedron(arrowB_t2);

	//C:
	glm::dvec3 pfC_Pt = eigenVecC * (1.0f / (1.0f - 2.0f*TH));
	glm::dvec3 orthogEigenVecCxy = glm::dvec3(eigenVecC.y, -eigenVecC.x, eigenVecC.z) * TH;
	glm::dvec3 orthogEigenVecCxz = glm::dvec3(-eigenVecC.z, eigenVecC.y, eigenVecC.x) * TH;
	if (abs(glm::dot(glm::normalize(eigenVecC), glm::normalize(orthogEigenVecCxy)) - 1.0f) < .00001f){
		int ok = 0;
		//not orthog
		orthogEigenVecCxy = glm::dvec3(eigenVecC.x, -eigenVecC.z, eigenVecC.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecC), glm::normalize(orthogEigenVecCxy)) - 1.0f) < .00001f){
			int ok = 0;
		}
	}
	if (abs(glm::dot(glm::normalize(eigenVecC), glm::normalize(orthogEigenVecCxz)) - 1.0f) < .00001f){
		int ok = 0;
		//not orthog
		orthogEigenVecCxz = glm::dvec3(eigenVecC.x, -eigenVecC.z, eigenVecC.y) * TH;
		if (abs(glm::dot(glm::normalize(eigenVecC), glm::normalize(orthogEigenVecCxz)) - 1.0f) < .00001f){
			int ok = 0;
		}
	}


	glm::dvec3 pfC_a = eigenVecC + orthogEigenVecCxy;
	glm::dvec3 pfC_b = eigenVecC - orthogEigenVecCxy;
	glm::dvec3 pfC_c = eigenVecC + orthogEigenVecCxz;
	glm::dvec3 pfC_d = eigenVecC - orthogEigenVecCxz;

	Tetrahedron arrowC_t1(pfC_Pt, pfC_a, pfC_b, pfC_c);
	Tetrahedron arrowC_t2(pfC_Pt, pfC_a, pfC_b, pfC_d);
	arrows.AddTetrahedron(arrowC_t1);
	arrows.AddTetrahedron(arrowC_t2);

	std::vector<GLfloat> arrowVertices;
	arrows.GetGlVertices(arrowVertices);

	//MatrixXd eigenVals = svd.singularValues();
	//GLfloat eigenValA(eigenVals(0, 0));
	//GLfloat eigenValB(eigenVals(1, 0));
	//GLfloat eigenValC(eigenVals(2, 0));
#pragma endregion

#pragma region VAO/VBOSetup
	//Tetrahedron vertices array setup
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO

	//Frame Vertices:
	GLuint frameVBO, frameVAO;
	glGenVertexArrays(1, &frameVAO);
	glGenBuffers(1, &frameVBO);
	glBindVertexArray(frameVAO);
	glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(frameVertices), frameVertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0); // Unbind frameVAO

	GLuint arrowsVBO, arrowsVAO;
	glGenVertexArrays(1, &arrowsVAO);
	glGenBuffers(1, &arrowsVBO);
	glBindVertexArray(arrowsVAO);
	glBindBuffer(GL_ARRAY_BUFFER, arrowsVBO);
	glBufferData(GL_ARRAY_BUFFER, arrowVertices.size() * sizeof(GLfloat), &arrowVertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0); // Unbind VAO
#pragma endregion 

	GLfloat phi = glm::pi<float>() / 8096.0f;
	GLfloat rotation = phi;

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.0f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw tetrahedron
		glm::mat4 model;
		//model = glm::translate(model, glm::dvec3(0.0, 0.0, -3.0));
		model = glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
		if (ELEPHANT){
			if (!cone)
				model = glm::scale(model, glm::vec3(.2, .2, .2));
				int ok = 0;
		}
		rotation += phi;
		bareShader.Use();
		glm::dvec3 lightPos(0.0f, 0.0f, -1.0f);
		glm::dvec3 objectColor(1.0f, 0.5f, 0.31f);
		GLint modelLoc = glGetUniformLocation(bareShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(bareShader.Program, "view");
		GLint projLoc = glGetUniformLocation(bareShader.Program, "projection");
		GLint lightPosLoc = glGetUniformLocation(bareShader.Program, "lightPos");
		GLint objectColorLoc = glGetUniformLocation(bareShader.Program, "objectColor");
		glUniform3f(objectColorLoc, objectColor.x, objectColor.y, objectColor.z);
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glBindVertexArray(VAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
		glDrawArrays(GL_TRIANGLES, 0, numTris * 3);

		//Draw Principal Frame:

		glUniform3f(objectColorLoc, 0.0f, 0.0f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindVertexArray(frameVAO);
		glDrawArrays(GL_LINES, 0, 6);

		//Draw arrows:
		glBindVertexArray(arrowsVAO);
		glDrawArrays(GL_TRIANGLES, 0, arrowVertices.size());

		glBindVertexArray(0);

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Properly de-allocate all resources once they've outlived their purpose
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

//	delete[] tetrahedron;

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();

	return 0;
}

void convertGlmMatToEigen(glm::dmat3 &glmMat, Eigen::MatrixXd &eigenMatOut){
	//eigenMatOut(row, col)
	eigenMatOut(0, 0) = glmMat[0][0];
	eigenMatOut(0, 1) = glmMat[0][1];
	eigenMatOut(0, 2) = glmMat[0][2];
	eigenMatOut(1, 0) = glmMat[1][0];
	eigenMatOut(1, 1) = glmMat[1][1];
	eigenMatOut(1, 2) = glmMat[1][2];
	eigenMatOut(2, 0) = glmMat[2][0];
	eigenMatOut(2, 1) = glmMat[2][1];
	eigenMatOut(2, 2) = glmMat[2][2];
}

bool verifyOrthogonalVecs(glm::dvec3 a, glm::dvec3 b, glm::dvec3 c){
	float dotab = glm::dot(a, b);
	float dotac = glm::dot(a, c);
	float dotbc = glm::dot(b, c);
	return true;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void normalizeEigenVals(float eigenVal1, float eigenVal2, float eigenVal3, float minNormalization, std::vector<double> &normalizedEvals) {
	float max = (eigenVal1 > eigenVal2 ? (eigenVal1 > eigenVal3 ? eigenVal1 : eigenVal3) : (eigenVal2 > eigenVal3 ? eigenVal2 : eigenVal3));
	float remaining = abs(1.0f - minNormalization);
	float a = (eigenVal1 / max) * remaining;
	float b = (eigenVal2 / max) * remaining;
	float c = (eigenVal3 / max) * remaining;
	normalizedEvals.push_back(minNormalization + a);
	normalizedEvals.push_back(minNormalization + b);
	normalizedEvals.push_back(minNormalization + c);
}

float maxDistanceFromCentroid(TetrahedralMesh &mesh){
	float max = -1.0f;
	glm::dvec3 centroid = mesh.MeshCentroid;
	typedef std::vector<Tetrahedron>::iterator iter;
	for (iter it = mesh.Tetrahedra.begin(); it != mesh.Tetrahedra.end(); it++){
		float distA = glm::distance(centroid, it->a);
		float distB = glm::distance(centroid, it->b);
		float distC = glm::distance(centroid, it->c);
		float distD = glm::distance(centroid, it->d);
		if (distA > max){
			max = distA;
		}
		if (distB > max){
			max = distB;
		}
		if (distC > max){
			max = distC;
		}
		if (distD > max){
			max = distD;
		}
	}
	return max;
}