#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "ShapeSeparatingAxis.h"

class Tetrahedron {
public:

	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;

	//Assumed uniform density for now
	float density;

	//Set by TetrahedralMesh class:
	glm::vec3 centroid;
	glm::mat3 angularInertia;
	float volume;
	float mass;


	Tetrahedron(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd) : a(pa), b(pb), c(pc), d(pd) {
		density = 1.0;
	}

	void computeVolume() {
		glm::mat4 volMat;
		volMat[0] = glm::vec4(a.x, b.x, c.x, d.x);
		volMat[1] = glm::vec4(a.y, b.y, c.y, d.y);
		volMat[2] = glm::vec4(a.z, b.z, c.z, d.z);
		volMat[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		float det = ShapeUtils::absf(glm::determinant(volMat));
		this->volume = det / (6.0f); // volume formula divides by 3!

		this->mass = volume * density;
	}

	void Translate(glm::vec3 trans) {
		a += trans;
		b += trans;
		c += trans;
		d += trans;
	}

};

//Uses bounding box for collision detection
class TetrahedralMesh : public ShapeSeparatingAxis {
public:
	int TetrahedraCount;
	std::vector<Tetrahedron> Tetrahedra;
	std::vector<GLfloat> vertices;

	//glm::mat3 AngularInertia;
	float Volume;
	float Mass;
	float Density;

	TetrahedralMesh() : TetrahedraCount(0) {}

	virtual void InitVAOandVBO(Shader &shader) override {

		shader.Use();

		ComputeInertia();
		getVerticesFromTetrahedra();

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

		shader.Use();
		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glUniform4f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z, 1.0f);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

	virtual ~TetrahedralMesh() override {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	//Mesh's centroid should be centered at origin before calling
	void getVerticesFromTetrahedra() {

		//BB[0] = minX, [1] = maxX, [2] = minY, [3] = maxY, [4] = minZ, [5] = maxZ
		boundingBox[0] = boundingBox[2] = boundingBox[4] = 1000000.0f;
		boundingBox[1] = boundingBox[3] = boundingBox[5] = -boundingBox[0];

		/*if (TetrahedraCount == 0) {
			std::cout << "0 tetrahedra" << std::endl;
		}*/
		std::cout << "Tetrahedra count: " << TetrahedraCount << std::endl;
		for (int i = 0; i < TetrahedraCount; i++) {
			Tetrahedron t = this->Tetrahedra[i];

			testTetrahedronPtsForBoundingBox(t);

			//TODO how to ensure normal is correct direction
			glm::vec3 n1 = ShapeUtils::getNormalOfTriangle(t.a, t.b, t.c);
			glm::vec3 n2 = ShapeUtils::getNormalOfTriangle(t.a, t.b, t.d);
			glm::vec3 n3 = ShapeUtils::getNormalOfTriangle(t.d, t.b, t.c);
			glm::vec3 n4 = ShapeUtils::getNormalOfTriangle(t.a, t.d, t.c);

			if (glm::dot(n1, t.a) < 0.0f) {
				n1 *= -1.0f;
			}

			if (glm::dot(n2, t.a) < 0.0f) {
				n2 *= -1.0f;
			}

			if (glm::dot(n3, t.b) < 0.0f) {
				n3 *= -1.0f;
			}

			if (glm::dot(n4, t.a) < 0.0f) {
				n4 *= -1.0f;
			}

			ShapeUtils::addTriangleToVector(t.a, t.b, t.c, n1, this->vertices);
			ShapeUtils::addTriangleToVector(t.a, t.b, t.d, n2, this->vertices);
			ShapeUtils::addTriangleToVector(t.d, t.b, t.c, n3, this->vertices);
			ShapeUtils::addTriangleToVector(t.a, t.d, t.c, n4, this->vertices);
		}

		glm::vec3 c1(boundingBox[1], boundingBox[2], boundingBox[5]);
		glm::vec3 c2(boundingBox[1], boundingBox[2], boundingBox[4]);
		glm::vec3 c3(boundingBox[0], boundingBox[2], boundingBox[4]);
		glm::vec3 c4(boundingBox[0], boundingBox[2], boundingBox[5]);

		glm::vec3 c5(boundingBox[1], boundingBox[3], boundingBox[5]);
		glm::vec3 c6(boundingBox[1], boundingBox[3], boundingBox[4]);
		glm::vec3 c7(boundingBox[0], boundingBox[3], boundingBox[4]);
		glm::vec3 c8(boundingBox[0], boundingBox[3], boundingBox[5]);

		corners.push_back(c1);
		corners.push_back(c2);
		corners.push_back(c3);
		corners.push_back(c4);
		corners.push_back(c5);
		corners.push_back(c6);
		corners.push_back(c7);
		corners.push_back(c8);
		
		glm::vec3 n1(1.0f, 0.0f, 0.0f);
		glm::vec3 n2(0.0f, 1.0f, 0.0f);
		glm::vec3 n3(0.0f, 0.0f, 1.0f);

		normals.push_back(n1);
		normals.push_back(n2);
		normals.push_back(n3);
	}

	void testTetrahedronPtsForBoundingBox(Tetrahedron &t) {
		testPtForBB(t.a);
		testPtForBB(t.b);
		testPtForBB(t.c);
		testPtForBB(t.d);
	}

	void testPtForBB(glm::vec3 &pt) {
		if (pt.x < boundingBox[0]) {
			boundingBox[0] = pt.x;
		}
		if (pt.x > boundingBox[1]) {
			boundingBox[1] = pt.x;
		}
		if (pt.y < boundingBox[2]) {
			boundingBox[2] = pt.y;
		}
		if (pt.y > boundingBox[3]) {
			boundingBox[3] = pt.y;
		}
		if (pt.z < boundingBox[4]) {
			boundingBox[4] = pt.z;
		}
		if (pt.z > boundingBox[5]) {
			boundingBox[5] = pt.z;
		}

	}

	void SetMass(float mass) {
		this->Mass = mass;
	}

	void AddTetrahedron(Tetrahedron &th) {
		Tetrahedra.push_back(th);
		TetrahedraCount++;
	}

	void ComputeCentroid() {
		glm::vec3 tempCentroid(0.0, 0.0, 0.0);
		float vol = 0.0f;
		for (std::vector<Tetrahedron>::iterator it = Tetrahedra.begin(); it != Tetrahedra.end(); it++) {
			it->computeVolume();
			this->computeTetrahedronCentroid(*it);
			tempCentroid += it->centroid * it->volume;
			vol += it->volume;
		}

		//Take average of tetrahedra centroids as mesh centroid
		this->centroid = tempCentroid / vol;
		this->Volume = vol;
	}

	//Assumes mass has been set
	//First computes mesh centroid and volume of each tetrahedra
	//Then computes the density of all tetrahedra based on the 
	// total mass and volume
	//Finally, the mesh's angular inertia is computed by summing
	// the angular inertia of each tetrahedra, taken w.r.t. the mesh centroid
	virtual void ComputeInertia() override {
		typedef std::vector<Tetrahedron>::iterator iter;

		this->ComputeCentroid();

		this->Density = this->Mass / this->Volume;

		//Now have inertia matrix of each tetrahedra wrt its own centroid, compute
		// with respect to mesh centroid:
		glm::mat3 meshInertia(0.0);
		for (iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++) {

			float proportion = (it->volume / this->Volume);
			it->mass = proportion * this->Mass;
			it->density = this->Density;

			computeTetrahedronAngularInertia(*it, this->centroid);
			meshInertia += it->angularInertia;
		}
		this->angularInertia = meshInertia;
	}

	void Translate(glm::vec3 trans) {
		for (std::vector<Tetrahedron>::iterator it = Tetrahedra.begin(); it != Tetrahedra.end(); it++) {
			it->Translate(trans);
		}
		this->centroid += trans;
	}

	int GetTetrahedraCount() {
		return this->TetrahedraCount;
	}

	TetrahedralMesh(std::string filename) {

		this->name = filename;

		//Use pointer so polymorphism can be used in initFromStream
		std::ifstream *meshfile = new std::ifstream(filename);
		if (meshfile->is_open()) {

			initFromStream(meshfile);
			meshfile->close();
		}
		else {
			std::cout << "Failed opening " << filename << std::endl;
		}

		delete meshfile;
	}

	//Constructor to be used by Emscripten, since it can only pass char* and not string objects
	TetrahedralMesh(const char *name, const char *fileContents) {
		this->name = name;

		std::string file(fileContents);
		std::cout << file << std::endl;

		//Need pointer for initFromStream polymorphism to work
		std::istringstream *stringStream = new std::istringstream(file);
		initFromStream(stringStream);

		delete stringStream;
	}

	void initFromStream(std::istream *stream) {
		std::vector<glm::vec3> tetrahedraVertices;
		bool mass = false;
		bool vertex = false;
		bool tetrahedra = false;

		std::string line;

		while (std::getline(*stream, line)) {
			std::cout << line << std::endl;
			if (!mass) {
				if (line.find("Mass") != std::string::npos) {
					mass = true;
					std::cout << "Found mass" << std::endl;
					std::getline(*stream, line);
					this->Mass = stof(line);
				}
			}
			else if (!vertex) {
				if (line.find("Vertices") != std::string::npos) {
					vertex = true;
					std::cout << "Found Vertices" << std::endl;
				}
			}
			else if (vertex && !tetrahedra) {
				if (line.find("Tetrahedra") != std::string::npos) {
					std::cout << "Found Tetrahedra" << std::endl;
					tetrahedra = true;
					continue;
				}

				std::vector<std::string> splitLine = TetrahedralMesh::split(line, ' ');
				tetrahedraVertices.push_back(glm::vec3(stof(splitLine[0]), stof(splitLine[1]), stof(splitLine[2]))); //drop the homogeneous coord
			}
			else if (tetrahedra) {
				//Read in tetrahedra indices:
				if (line.find("End") != std::string::npos) {
					std::cout << "Found end" << std::endl;
					break;
				}
				std::vector<std::string> splitLine = TetrahedralMesh::split(line, ' ');
				int indexA = stoi(splitLine[0]) - 1;
				int indexB = stoi(splitLine[1]) - 1;
				int indexC = stoi(splitLine[2]) - 1;
				int indexD = stoi(splitLine[3]) - 1;
				Tetrahedron t(tetrahedraVertices[indexA], tetrahedraVertices[indexB], tetrahedraVertices[indexC], tetrahedraVertices[indexD]);
				this->AddTetrahedron(t);
			}
		}
	}

	virtual std::string getShapeCSVline1() override {
		std::ostringstream os;
		os << "5,TODO";
		/*for (int i = 0; i < corners.size(); i++) {
			os << "," << corners[i].x << "," << corners[i].y;
		}*/
		os << std::endl;
		return os.str();
	}

	static void computeTetrahedronCentroid(Tetrahedron &th) {
		float centroidX = (th.a.x + th.b.x + th.c.x + th.d.x) / 4.0f;
		float centroidY = (th.a.y + th.b.y + th.c.y + th.d.y) / 4.0f;
		float centroidZ = (th.a.z + th.b.z + th.c.z + th.d.z) / 4.0f;
		th.centroid = glm::vec3(centroidX, centroidY, centroidZ);
	}

	//Compute th's volume before calling

	//Computes A.I. wrt the mesh's centroid instead of tetrahedron's centroid,
	// then there's no need for the angularInertiaWithRespectToPoint conversion
	void computeTetrahedronAngularInertia(Tetrahedron &th, glm::vec3 &meshCentroid) {

		computeTetrahedronCentroid(th);

		//use vertices wrt centroid:
		glm::vec3 a = th.a - meshCentroid; //basically translates centroid to origin
		glm::vec3 b = th.b - meshCentroid;
		glm::vec3 c = th.c - meshCentroid;
		glm::vec3 d = th.d - meshCentroid;

		//th.computeVolume();

		//DET of Jacobian, see http://docsdrive.com/pdfs/sciencepublications/jmssp/2005/8-11.pdf
		float DETJ = 6 * th.volume;
		//inertia tensor matrix:
		// | a  -b' -c' |
		// |-b'  b  -a' |
		// |-c' -a'  c  |

		float density = th.density;
		if (density == 0.0f)
			density = 1.0f;
		float aa = (density * DETJ * (a.y*a.y + a.y*b.y + b.y*b.y + a.y*c.y + b.y*c.y + c.y*c.y + a.y * d.y
			+ b.y*d.y + c.y*d.y + d.y*d.y + a.z*a.z + a.z*b.z + b.z*b.z + a.z*c.z + b.z*c.z + c.z*c.z + a.z * d.z
			+ b.z*d.z + c.z*d.z + d.z*d.z)) / 60.0f;

		float bb = (density * DETJ * (a.x*a.x + a.x*b.x + b.x*b.x + a.x*c.x + b.x*c.x + c.x*c.x + a.x * d.x
			+ b.x*d.x + c.x*d.x + d.x*d.x + a.z*a.z + a.z*b.z + b.z*b.z + a.z*c.z + b.z*c.z + c.z*c.z + a.z * d.z
			+ b.z*d.z + c.z*d.z + d.z*d.z)) / 60.0f;

		float cc = (density * DETJ * (a.x*a.x + a.x*b.x + b.x*b.x + a.x*c.x + b.x*c.x + c.x*c.x + a.x * d.x
			+ b.x*d.x + c.x*d.x + d.x*d.x + a.y*a.y + a.y*b.y + b.y*b.y + a.y*c.y + b.y*c.y + c.y*c.y + a.y * d.y
			+ b.y*d.y + c.y*d.y + d.y*d.y)) / 60.0f;

		float aPrime = (density * DETJ * (2.0f*a.y*a.z + b.y*a.z + c.y*a.z + d.y*a.z + a.y*b.z + 2.0f*b.y*b.z
			+ c.y*b.z + d.y*b.z + a.y*c.z + b.y*c.z + 2.0f*c.y*c.z + d.y*c.z + a.y*d.z + b.y*d.z
			+ c.y*d.z + 2.0f*d.y*d.z)) / 120.0f;

		float bPrime = (density * DETJ * (2.0f*a.x*a.z + b.x*a.z + c.x*a.z + d.x*a.z + a.x*b.z + 2.0f*b.x*b.z
			+ c.x*b.z + d.x*b.z + a.x*c.z + b.x*c.z + 2.0f*c.x*c.z + d.x*c.z + a.x*d.z + b.x*d.z
			+ c.x*d.z + 2.0f*d.x*d.z)) / 120.0f;

		float cPrime = (density * DETJ * (2.0f*a.x*a.y + b.x*a.y + c.x*a.y + d.x*a.y + a.x*b.y + 2.0f*b.x*b.y
			+ c.x*b.y + d.x*b.y + a.x*c.y + b.x*c.y + 2.0f*c.x*c.y + d.x*c.y + a.x*d.y + b.x*d.y
			+ c.x*d.y + 2.0f*d.x*d.y)) / 120.0f;

		th.angularInertia = glm::mat3(aa, -bPrime, -cPrime, -bPrime, bb, -aPrime, -cPrime, -aPrime, cc);
	}

	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			elems.push_back(item);
		}
		return elems;
	}


	static std::vector<std::string> split(const std::string &s, char delim) {
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

};