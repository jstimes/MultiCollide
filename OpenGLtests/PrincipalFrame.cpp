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

// Function prototypes
void convertGlmMatToEigen(glm::mat3 &glmMat, Eigen::MatrixXd &eigenMatOut);
bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c);
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

    // Build and compile our shader program
    Shader bareShader("vertexShaderBare.glsl", "fragmentShaderBare.glsl");

#pragma endregion

#pragma region MeshSetup
	//Mesh vertices
	glm::vec3 a1(-.3f, 0.0f, 0.0f);
	glm::vec3 a2( .3f, 0.0f, 0.0f);
	glm::vec3 b(0.0, 0.0, 0.0);
	glm::vec3 c1(0.0f, 0.0f, -.3f);
	glm::vec3 c2(0.0f, 0.0f,  .3f);
	glm::vec3 d1(0.0f,  .3f, 0.0f);
	glm::vec3 d2(0.0f, -.3f, 0.0f);

	Tetrahedron t1(a1, b, c1, d1);
    t1.normalABD = glm::vec3(0.0f,  0.0f, 1.0f);
    t1.normalABC = glm::vec3(0.0f, -1.0f, 0.0f);
    t1.normalCDB = glm::vec3(1.0f,  0.0f, 0.0f);
    t1.normalACD = glm::vec3(-.333f, .333f, -.333f);

	Tetrahedron t2(a2, b, c1, d1);
    t2.normalABD = glm::vec3( 0.0f,  0.0f,  1.0f);
    t2.normalABC = glm::vec3( 0.0f, -1.0f,  0.0f);
    t2.normalCDB = glm::vec3(-1.0f,  0.0f,  0.0f);
    t2.normalACD = glm::vec3(.333f, .333f, -.333f);

	Tetrahedron t3(a1, b, c2, d1);
    t3.normalABD = glm::vec3(0.0f,  0.0f, -1.0f);
    t3.normalABC = glm::vec3(0.0f, -1.0f,  0.0f);
    t3.normalCDB = glm::vec3(1.0f,  0.0f,  0.0f);
    t3.normalACD = glm::vec3(-.333f, .333f, .333f);

	Tetrahedron t4(a2, b, c2, d1);
    t4.normalABD = glm::vec3( 0.0f,  0.0f, -1.0f);
    t4.normalABC = glm::vec3( 0.0f, -1.0f,  0.0f);
    t4.normalCDB = glm::vec3(-1.0f,  0.0f,  0.0f);
    t4.normalACD = glm::vec3(.333f, .333f, .333f);

	TetrahedralMesh mesh;
	mesh.AddTetrahedron(t1);
	mesh.AddTetrahedron(t2);
	mesh.AddTetrahedron(t3);
	mesh.AddTetrahedron(t4);
	mesh.ComputeMeshAttributes();

	int numTris = mesh.GetTetrahedraCount() * 4; // Each tetrahedron consists of 4 triangles
	std::vector<float> vertices;
	mesh.GetGlVertices(vertices);
	GLfloat *tetrahedron = new GLfloat[vertices.size()];
	for(int vert = 0; vert<vertices.size(); vert++){
		tetrahedron[vert] = vertices[vert];
	}
#pragma endregion

#pragma region PrincipalFrameComputation
    MatrixXd angularInertia(3, 3);
	convertGlmMatToEigen(mesh.AngularInertia, angularInertia);
	Eigen::ComplexEigenSolver<MatrixXd> ces(angularInertia);
	for(int i=0; i<3; i++)
		std::cout << angularInertia.col(i) << std::endl;
	typedef Eigen::ComplexEigenSolver<MatrixXd>::EigenvectorType EigenVec;
	EigenVec eigenVec1 = ces.eigenvectors().col(0);
	EigenVec eigenVec2 = ces.eigenvectors().col(1);
	EigenVec eigenVec3 = ces.eigenvectors().col(2);

	using namespace std;
	cout << eigenVec1 << endl << eigenVec2 << endl << eigenVec3 << endl;

    //Eigen::JacobiSVD<MatrixXd> svd(angularInertia, Eigen::DecompositionOptions::ComputeFullU);
   // MatrixXd eigenVectors = svd.matrixU();
    glm::vec3 eigenVecA(eigenVec1.row(0).real().value(), eigenVec1.row(1).real().value(), eigenVec1.row(2).real().value());
    glm::vec3 eigenVecB(eigenVec2.row(0).real().value(), eigenVec2.row(1).real().value(), eigenVec2.row(2).real().value());
    glm::vec3 eigenVecC(eigenVec3.row(0).real().value(), eigenVec3.row(1).real().value(), eigenVec3.row(2).real().value());

	verifyOrthogonalVecs(eigenVecA, eigenVecB, eigenVecC);

	//Eigen::JacobiSVD<MatrixXd>::SingularValuesType singVals = svd.singularValues();
	//GLfloat sA = singVals(0, 0);
	//GLfloat sB = singVals(1, 0);
	//GLfloat sC = singVals(2, 0);

    //Normalize:
    eigenVecA = glm::normalize(eigenVecA);
    eigenVecB = glm::normalize(eigenVecB);
    eigenVecC = glm::normalize(eigenVecC);

	glm::vec3 centroid = mesh.MeshCentroid;

    GLfloat frameVertices[] = {
        eigenVecA.x, eigenVecA.y, eigenVecA.z, 0.0, 0.0, 0.0,
        centroid.x, centroid.y, centroid.z, 0.0, 0.0, 0.0,

        eigenVecB.x, eigenVecB.y, eigenVecB.z, 0.0, 0.0, 0.0,
        centroid.x, centroid.y, centroid.z, 0.0, 0.0, 0.0,

        eigenVecC.x, eigenVecC.y, eigenVecC.z, 0.0, 0.0, 0.0,
        centroid.x, centroid.y, centroid.z, 0.0, 0.0, 0.0
    };

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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), tetrahedron, GL_STATIC_DRAW);

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
#pragma endregion 

    GLfloat phi = glm::pi<float>() / 4096.0f;
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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Draw tetrahedron
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(0.0, 0.0, -3.0));
        model = glm::rotate(model, glm::sin(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::scale(model, glm::vec3(.2, .2, .2));
        rotation += phi;
        bareShader.Use();
        glm::vec3 lightPos(0.0f, 0.0f, -1.0f);
        glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
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
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
        glDrawArrays(GL_TRIANGLES, 0, numTris * 3);

        //Draw Principal Frame:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(frameVAO);
        glDrawArrays(GL_LINES, 0, 6);

        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

	delete[] tetrahedron;

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    return 0;
}

void convertGlmMatToEigen(glm::mat3 &glmMat, Eigen::MatrixXd &eigenMatOut){
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

bool verifyOrthogonalVecs(glm::vec3 a, glm::vec3 b, glm::vec3 c){
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