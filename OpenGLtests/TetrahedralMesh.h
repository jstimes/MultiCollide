#pragma once
#include <glm\common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <fstream>
#include "ShapeUtils.h"

class Tetrahedron {
public:
	//Properties set by caller:
	glm::dvec3 a;
	glm::dvec3 b;
	glm::dvec3 c;
	glm::dvec3 d;
	//Normal vectors for lighting
	glm::dvec3 normalACD;   
    glm::dvec3 normalABC;
    glm::dvec3 normalCDB;
    glm::dvec3 normalABD;
	double volume;
	double mass;
	double density;

	//Set by TetrahedralMesh class:
	glm::dvec3 centroid;
	glm::dmat3 angularInertia;

	Tetrahedron() {
		volume = 1.0;
		mass = 1.0;
		density = 1.0;
	}

	Tetrahedron(glm::dvec3 pa, glm::dvec3 pb, glm::dvec3 pc, glm::dvec3 pd) : a(pa), b(pb), c(pc), d(pd) {
		computeVolume();
		mass = 1.0;
		density = 1.0;
		normalACD = ShapeUtils::getNormalOfTriangle(a, c, d);
		normalABC = ShapeUtils::getNormalOfTriangle(a, b, c);
		normalCDB = ShapeUtils::getNormalOfTriangle(c, d, b);
		normalABD = ShapeUtils::getNormalOfTriangle(a, b, d);
	}

	Tetrahedron(glm::dvec3 pa, glm::dvec3 pb, glm::dvec3 pc, glm::dvec3 pd, double pVolume, double pMass, double pDensity) : 
		a(pa), b(pb), c(pc), d(pd), volume(pVolume), mass(pMass), density(pDensity) {
		
		mass = 1.0;
		density = 1.0;
		normalACD = ShapeUtils::getNormalOfTriangle(a, c, d);
		normalABC = ShapeUtils::getNormalOfTriangle(a, b, c);
		normalCDB = ShapeUtils::getNormalOfTriangle(c, d, b);
		normalABD = ShapeUtils::getNormalOfTriangle(a, b, d);
	}

	Tetrahedron(glm::dvec3 pa, glm::dvec3 pb, glm::dvec3 pc, glm::dvec3 pd, glm::dvec3 pNormalACD, glm::dvec3 pNormalABC,
		glm::dvec3 pNormalCDB, glm::dvec3 pNormalABD) : a(pa), b(pb), c(pc), d(pd), normalACD(pNormalACD), 
		normalABC(pNormalABC), normalCDB(pNormalCDB), normalABD(pNormalABD) {
		computeVolume();
		mass = 1.0;
		density = 1.0;
	}

	Tetrahedron(glm::dvec3 pa, glm::dvec3 pb, glm::dvec3 pc, glm::dvec3 pd, glm::dvec3 pNormalACD, glm::dvec3 pNormalABC,
		glm::dvec3 pNormalCDB, glm::dvec3 pNormalABD, double pVolume, double pMass, double pDensity) : 
			a(pa), b(pb), c(pc), d(pd), normalACD(pNormalACD), normalABC(pNormalABC), normalCDB(pNormalCDB), 
			normalABD(pNormalABD), volume(pVolume), mass(pMass), density(pDensity) {

	}

	void computeVolume(){
		glm::mat4 volMat;
		volMat[0] = glm::vec4(a.x, b.x, c.x, d.x);
		volMat[1] = glm::vec4(a.y, b.y, c.y, d.y);
		volMat[2] = glm::vec4(a.z, b.z, c.z, d.z);
		volMat[3] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

		//Scale: 
		//volMat *= 100.0f;
		double det = abs(glm::determinant(volMat));
		//TODO need to divide by 10^4 ...?
		this->volume = det / (6.0f); // volume formula divides by 3!, and need to unscale by 10, done by 10^4

		this->mass = volume * density;
	}

	void Translate(glm::dvec3 trans){
		a += trans;
		b += trans;
		c += trans;
		d += trans;
	}

};

class TetrahedralMesh {
public:
	int TetrahedraCount;
	std::vector<Tetrahedron> Tetrahedra;
	glm::dvec3 MeshCentroid;
	glm::dmat3 AngularInertia;
	double Volume;
	double Mass;
	double Density;

	TetrahedralMesh() : TetrahedraCount(0) {}

	void AddTetrahedron(Tetrahedron &th){
		Tetrahedra.push_back(th);
		TetrahedraCount++;
	}

	//First aggregates mass & volumes of tetrahedra, while computing their 
	// centroids & individual angular inertia matrices
	//Then averages centroids for mesh centroid and 
	// sums the tetrahedra inertia w.r.t. the mesh centroid
	// to get the mesh's angular inertia matrix
	void ComputeMeshAttributes(){
		typedef std::vector<Tetrahedron>::iterator iter;

		glm::dvec3 centroid(0.0, 0.0, 0.0);
		double vol = 0.0f;
		double mass = 0.0f;
		double EPSILON = 0.0000000000001;
		for (iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			this->computeTetrahedronAngularInertia(*it);
			if (abs(it->centroid.x) < EPSILON){
				it->centroid.x = 0.0;
			}
			if (abs(it->centroid.y) < EPSILON){
				it->centroid.y = 0.0;
			}
			if (abs(it->centroid.z) < EPSILON){
				it->centroid.z = 0.0;
			}
			for (int col = 0; col < 3; col++){
				for (int row = 0; row < 3; row++){
					if (abs(it->angularInertia[col][row]) < EPSILON){
						it->angularInertia[col][row] = 0.0;
					}
				}
			}
			centroid += it->centroid * it->volume;
			vol += it->volume;
			mass += it->mass;
		}
		//Take average of tetrahedra centroids as mesh centroid
		this->MeshCentroid = centroid / vol;
		this->Volume = vol;
		this->Mass = mass;

		//Now have inertia matrix of each tetrahedra wrt its own centroid, compute
		// with respect to mesh centroid:
		glm::mat3 meshInertia(0.0);
		for (iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			glm::mat3 inertiaWrtMesh;
			this->angularInertiaWithRespectToPoint(this->MeshCentroid, *it, inertiaWrtMesh);
			meshInertia += inertiaWrtMesh;
		}
		this->AngularInertia = meshInertia;
	}

	void doubleTetrahedra(){
		typedef std::vector<Tetrahedron>::iterator iter;
		std::vector<Tetrahedron> splitTetrahedra;
		for (iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			bool maxEdge[6];
			glm::dvec3 longestA, longestB, otherA, otherB;
			for (int i = 0; i < 6; i++) maxEdge[i] = false;
			double max = 0.0f;
			double ab = glm::length(it->a - it->b);
			max = ab;
			maxEdge[0] = true;
			longestA = it->a;
			longestB = it->b;
			otherA = it->c;
			otherB = it->d;
			double ac = glm::length(it->a - it->b);
			if (ac > max){
				max = ac;
				maxEdge[1] = true;
				longestA = it->a;
				longestB = it->c;
				otherA = it->b;
				otherB = it->d;
			}
			double ad = glm::length(it->a - it->b);
			if (ad > max){
				max = ad;
				maxEdge[2] = true;
				longestA = it->a;
				longestB = it->d;
				otherA = it->b;
				otherB = it->c;
			}
			double bc = glm::length(it->a - it->b);
			if (bc > max){
				max = bc;
				maxEdge[3] = true;
				longestA = it->c;
				longestB = it->b;
				otherA = it->a;
				otherB = it->d;
			}
			double bd = glm::length(it->a - it->b);
			if (bd > max){
				max = bd;
				maxEdge[4] = true;
				longestA = it->d;
				longestB = it->b;
				otherA = it->a;
				otherB = it->c;
			}
			double cd = glm::length(it->a - it->b);
			if (cd > max){
				max = cd;
				maxEdge[5] = true;
				longestA = it->c;
				longestB = it->d;
				otherA = it->b;
				otherB = it->a;
			}
			glm::dvec3 midPt = (longestA + longestB) / 2.0;
			Tetrahedron new1(midPt, longestA, otherA, otherB);
			Tetrahedron new2(midPt, longestB, otherA, otherB);
			splitTetrahedra.push_back(new1);
			splitTetrahedra.push_back(new2);

		}
		this->TetrahedraCount = 0;
		this->Tetrahedra.clear();
		for (iter it = splitTetrahedra.begin(); it != splitTetrahedra.end(); it++){
			//if(this->TetrahedraCount != 0){
			//	it->Translate(glm::dvec3(-.25f, -0.25f, -.25f)); //debugging
			//}
			//else{
			//	it->Translate(glm::dvec3(.25f, 0.25f, .25f));
			//}
			this->AddTetrahedron(*it);
		}
	}

	void Translate(glm::dvec3 trans){
		for (std::vector<Tetrahedron>::iterator it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			it->Translate(trans);
		}
	}

	//TODO USE INDEX BUFFER 
	void GetGlVertices(std::vector<GLfloat> &verts){
		for(int tet=0; tet<TetrahedraCount; tet++){
			Tetrahedron th = Tetrahedra[tet];
			verts.push_back((GLfloat) th.a.x);
			verts.push_back((GLfloat)th.a.y);
			verts.push_back((GLfloat) th.a.z);
			verts.push_back((GLfloat) th.normalACD.x);
			verts.push_back((GLfloat) th.normalACD.y);
			verts.push_back((GLfloat) th.normalACD.z);
			verts.push_back((GLfloat) th.c.x);
			verts.push_back((GLfloat) th.c.y);
			verts.push_back((GLfloat) th.c.z);
			verts.push_back((GLfloat) th.normalACD.x);
			verts.push_back((GLfloat) th.normalACD.y);
			verts.push_back((GLfloat) th.normalACD.z);
			verts.push_back((GLfloat) th.d.x);
			verts.push_back((GLfloat) th.d.y);
			verts.push_back((GLfloat) th.d.z);
			verts.push_back((GLfloat) th.normalACD.x);
			verts.push_back((GLfloat) th.normalACD.y);
			verts.push_back((GLfloat) th.normalACD.z);

			verts.push_back((GLfloat) th.a.x);
			verts.push_back((GLfloat) th.a.y);
			verts.push_back((GLfloat) th.a.z);
			verts.push_back((GLfloat) th.normalABC.x);
			verts.push_back((GLfloat) th.normalABC.y);
			verts.push_back((GLfloat) th.normalABC.z);
			verts.push_back((GLfloat) th.b.x);
			verts.push_back((GLfloat) th.b.y);
			verts.push_back((GLfloat) th.b.z);
			verts.push_back((GLfloat) th.normalABC.x);
			verts.push_back((GLfloat) th.normalABC.y);
			verts.push_back((GLfloat) th.normalABC.z);
			verts.push_back((GLfloat) th.c.x);
			verts.push_back((GLfloat) th.c.y);
			verts.push_back((GLfloat) th.c.z);
			verts.push_back((GLfloat) th.normalABC.x);
			verts.push_back((GLfloat) th.normalABC.y);
			verts.push_back((GLfloat) th.normalABC.z);

			verts.push_back((GLfloat) th.c.x);
			verts.push_back((GLfloat) th.c.y);
			verts.push_back((GLfloat) th.c.z);
			verts.push_back((GLfloat) th.normalCDB.x);
			verts.push_back((GLfloat) th.normalCDB.y);
			verts.push_back((GLfloat) th.normalCDB.z);
			verts.push_back((GLfloat) th.d.x);
			verts.push_back((GLfloat) th.d.y);
			verts.push_back((GLfloat) th.d.z);
			verts.push_back((GLfloat) th.normalCDB.x);
			verts.push_back((GLfloat) th.normalCDB.y);
			verts.push_back((GLfloat) th.normalCDB.z);
			verts.push_back((GLfloat) th.b.x);
			verts.push_back((GLfloat) th.b.y);
			verts.push_back((GLfloat) th.b.z);
			verts.push_back((GLfloat) th.normalCDB.x);
			verts.push_back((GLfloat) th.normalCDB.y);
			verts.push_back((GLfloat) th.normalCDB.z);

			verts.push_back((GLfloat) th.a.x);
			verts.push_back((GLfloat) th.a.y);
			verts.push_back((GLfloat) th.a.z);
			verts.push_back((GLfloat) th.normalABD.x);
			verts.push_back((GLfloat) th.normalABD.y);
			verts.push_back((GLfloat) th.normalABD.z);
			verts.push_back((GLfloat) th.b.x);
			verts.push_back((GLfloat) th.b.y);
			verts.push_back((GLfloat) th.b.z);
			verts.push_back((GLfloat) th.normalABD.x);
			verts.push_back((GLfloat) th.normalABD.y);
			verts.push_back((GLfloat) th.normalABD.z);
			verts.push_back((GLfloat) th.d.x);
			verts.push_back((GLfloat) th.d.y);
			verts.push_back((GLfloat) th.d.z);
			verts.push_back((GLfloat) th.normalABD.x);
			verts.push_back((GLfloat) th.normalABD.y);
			verts.push_back((GLfloat) th.normalABD.z);
		}
	}

	int GetTetrahedraCount(){
		return this->TetrahedraCount;
	}

	static void BuildFromFile(std::string filename, TetrahedralMesh &outRef){
		std::string line;
		std::ifstream meshfile(filename);
		if (meshfile.is_open()) {
			std::vector<glm::dvec3> vertices;
			bool vertex = false;
			bool tetrahedra = false;
			while (getline(meshfile, line)){
				if (!vertex){
					if (line.compare("Vertices") == 0){
						//vertex count:
						getline(meshfile, line);
						//vertexx count = line
						vertex = true;
					}
				}
				else if (vertex && !tetrahedra){
					if (line.compare("Triangles") == 0){
						while (line != "Tetrahedra")
							getline(meshfile, line);
						getline(meshfile, line); //count
						tetrahedra = true;
						continue;
					}
					std::vector<std::string> splitLine;
					TetrahedralMesh::splitStringOnSpace(line, splitLine);
					vertices.push_back(glm::dvec3(stof(splitLine[0]), stof(splitLine[1]), stof(splitLine[2]))); //drop the homogeneous coord
				}
				else if(tetrahedra){
					//Read in tetrahedra indices:
					if (line.compare("End") == 0)
						break;
					std::vector<std::string> splitLine;
					TetrahedralMesh::splitStringOnSpace(line, splitLine);
					int indexA = stoi(splitLine[0]) - 1;
					int indexB = stoi(splitLine[1]) - 1;
					int indexC = stoi(splitLine[2]) - 1;
					int indexD = stoi(splitLine[3]) - 1;
					Tetrahedron t(vertices[indexA], vertices[indexB], vertices[indexC], vertices[indexD]);
					outRef.AddTetrahedron(t);
				}
			}
			meshfile.close();
		}
		else {
			std::cout << "Failed opening " << filename << std::endl;
		}
	}

private:

	void angularInertiaWithRespectToPoint(glm::dvec3 &point, Tetrahedron &th, glm::mat3 &inertiaWrtPoint) {

		glm::dmat3 I3; // 3x3 identity
		glm::dvec3 ptToCentroid = th.centroid - point; // vector from point to centroid
		glm::dmat3 ptToCtimesTranspose = glm::outerProduct(ptToCentroid, ptToCentroid); // ptToC * (ptToC)^T

		inertiaWrtPoint = th.angularInertia + th.mass * (glm::dot(ptToCentroid,ptToCentroid) * I3 - ptToCtimesTranspose);
	}

public:

	static void computeTetrahedronCentroid(Tetrahedron &th){
		double centroidX = (th.a.x + th.b.x + th.c.x + th.d.x) / 4.0;
		double centroidY = (th.a.y + th.b.y + th.c.y + th.d.y) / 4.0;
		double centroidZ = (th.a.z + th.b.z + th.c.z + th.d.z) / 4.0;
		th.centroid = glm::dvec3(centroidX, centroidY, centroidZ);
	}

	private:
	void computeTetrahedronAngularInertia(Tetrahedron &th) {
		
		computeTetrahedronCentroid(th);

		//use vertices wrt centroid:
		glm::dvec3 a = th.a - th.centroid; //basically translates centroid to origin
		glm::dvec3 b = th.b - th.centroid;
		glm::dvec3 c = th.c - th.centroid;
		glm::dvec3 d = th.d - th.centroid;

		th.computeVolume();

		//DET of Jacobian, see http://docsdrive.com/pdfs/sciencepublications/jmssp/2005/8-11.pdf
		double DETJ = 6 * th.volume;
		//inertia tensor matrix:
		// | a  -b' -c' |
		// |-b'  b  -a' |
		// |-c' -a'  c  |

		double density = th.density;
		if(density == 0.0)
			density = 1.0;
		double aa = (density * DETJ * (a.y*a.y + a.y*b.y + b.y*b.y + a.y*c.y + b.y*c.y + c.y*c.y + a.y * d.y
			+ b.y*d.y + c.y*d.y + d.y*d.y + a.z*a.z + a.z*b.z + b.z*b.z + a.z*c.z + b.z*c.z + c.z*c.z + a.z * d.z
			+ b.z*d.z + c.z*d.z + d.z*d.z)) / 60.0;

		double bb = (density * DETJ * (a.x*a.x + a.x*b.x + b.x*b.x + a.x*c.x + b.x*c.x + c.x*c.x + a.x * d.x
			+ b.x*d.x + c.x*d.x + d.x*d.x + a.z*a.z + a.z*b.z + b.z*b.z + a.z*c.z + b.z*c.z + c.z*c.z + a.z * d.z
			+ b.z*d.z + c.z*d.z + d.z*d.z)) / 60.0;

		double cc = (density * DETJ * (a.x*a.x + a.x*b.x + b.x*b.x + a.x*c.x + b.x*c.x + c.x*c.x + a.x * d.x
			+ b.x*d.x + c.x*d.x + d.x*d.x + a.y*a.y + a.y*b.y + b.y*b.y + a.y*c.y + b.y*c.y + c.y*c.y + a.y * d.y
			+ b.y*d.y + c.y*d.y + d.y*d.y)) / 60.0;

		double aPrime = (density * DETJ * (2.0*a.y*a.z + b.y*a.z + c.y*a.z + d.y*a.z + a.y*b.z + 2.0*b.y*b.z
			+ c.y*b.z + d.y*b.z + a.y*c.z + b.y*c.z + 2.0*c.y*c.z + d.y*c.z + a.y*d.z + b.y*d.z
			+ c.y*d.z + 2.0*d.y*d.z)) / 120.0;

		double bPrime = (density * DETJ * (2.0*a.x*a.z + b.x*a.z + c.x*a.z + d.x*a.z + a.x*b.z + 2.0*b.x*b.z
			+ c.x*b.z + d.x*b.z + a.x*c.z + b.x*c.z + 2.0*c.x*c.z + d.x*c.z + a.x*d.z + b.x*d.z
			+ c.x*d.z + 2.0*d.x*d.z)) / 120.0;

		double cPrime = (density * DETJ * (2.0*a.x*a.y + b.x*a.y + c.x*a.y + d.x*a.y + a.x*b.y + 2.0*b.x*b.y
			+ c.x*b.y + d.x*b.y + a.x*c.y + b.x*c.y + 2.0*c.x*c.y + d.x*c.y + a.x*d.y + b.x*d.y
			+ c.x*d.y + 2.0*d.x*d.y)) / 120.0;

		th.angularInertia = glm::mat3(aa, -bPrime, -cPrime, -bPrime, bb, -aPrime, -cPrime, -aPrime, cc);
	}

	static void splitStringOnSpace(std::string str, std::vector<std::string>& args){
		std::istringstream iss(str);
		std::copy(std::istream_iterator<std::string>(iss),
			std::istream_iterator<std::string>(),
			back_inserter(args));
	}

};