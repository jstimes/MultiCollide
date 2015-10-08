#include <glm\common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

//Should probably be a class....
class Tetrahedron {
public:
	//Properties set by caller:
	glm::vec3 a;
	glm::vec3 b;
	glm::vec3 c;
	glm::vec3 d;
	//Normal vectors for lighting
	glm::vec3 normalACD;   
    glm::vec3 normalABC;
    glm::vec3 normalCDB;
    glm::vec3 normalABD;
	float volume;
	float mass;
	float density;

	Tetrahedron() {
		volume = 1.0;
		mass = 1.0;
		density = 1.0;
	}

	Tetrahedron(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd) : a(pa), b(pa), c(pc), d(pd) {
		volume = 1.0;
		mass = 1.0;
		density = 1.0;
		//TODO compute normals of triangles
	}

	Tetrahedron(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd, float pVolume, float pMass, float pDensity) : 
		a(pa), b(pa), c(pc), d(pd), volume(pVolume), mass(pMass), density(pDensity) {
		volume = 1.0;
		mass = 1.0;
		density = 1.0;
		//TODO compute normals of triangles
	}

	Tetrahedron(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd, glm::vec3 pNormalACD, glm::vec3 pNormalABC,
		glm::vec3 pNormalCDB, glm::vec3 pNormalABD) : a(pa), b(pa), c(pc), d(pd), normalACD(pNormalACD), 
		normalABC(pNormalABC), normalCDB(pNormalCDB), normalABD(pNormalABD) {
		volume = 1.0;
		mass = 1.0;
		density = 1.0;
	}

	Tetrahedron(glm::vec3 pa, glm::vec3 pb, glm::vec3 pc, glm::vec3 pd, glm::vec3 pNormalACD, glm::vec3 pNormalABC,
		glm::vec3 pNormalCDB, glm::vec3 pNormalABD, float pVolume, float pMass, float pDensity) : 
			a(pa), b(pa), c(pc), d(pd), normalACD(pNormalACD), normalABC(pNormalABC), normalCDB(pNormalCDB), 
			normalABD(pNormalABD), volume(pVolume), mass(pMass), density(pDensity) {

	}

	//Set by TetrahedralMesh class:
	glm::vec3 centroid;
	glm::mat3 angularInertia;
};

class TetrahedralMesh {
private:
	int TetrahedraCount;
	std::vector<Tetrahedron> Tetrahedra;

public:

	TetrahedralMesh() : TetrahedraCount(0) {}

	glm::vec3 MeshCentroid;
	glm::mat3 AngularInertia;
	float Volume;
	float Mass;
	float Density;

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
		
		glm::vec3 centroid(0.0, 0.0, 0.0);
		float vol = 0.0f;
		float mass = 0.0f;
		for(iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			this->computeTetrahedronCentroid(*it);
			centroid += it->centroid / it->volume;
			this->computeTetrahedronAngularInertia(*it);
			vol += it->volume;
			mass += it->mass;
		}
		//Take average of tetrahedra centroids as mesh centroid
		this->MeshCentroid = glm::vec3(centroid.x / this->TetrahedraCount, centroid.y / this->TetrahedraCount, centroid.z / this->TetrahedraCount);
		this->Volume = vol;
		this->Mass = mass;

		//Now have inertia matrix of each tetrahedra wrt its own centroid, compute
		// with respect to mesh centroid:
		glm::mat3 meshInertia(0.0);
		for(iter it = Tetrahedra.begin(); it != Tetrahedra.end(); it++){
			glm::mat3 inertiaWrtMesh;
			this->angularInertiaWithRespectToPoint(this->MeshCentroid, *it, inertiaWrtMesh);
			meshInertia += inertiaWrtMesh;
		}
		this->AngularInertia = meshInertia;
	}

	//TODO USE INDEX BUFFER
	void GetGlVertices(std::vector<float> &verts){
		for(int tet=0; tet<TetrahedraCount; tet++){
			Tetrahedron th = Tetrahedra[tet];
			verts.push_back(th.a.x);
			verts.push_back(th.a.y);
			verts.push_back(th.a.z);
			verts.push_back(th.normalACD.x);
			verts.push_back(th.normalACD.y);
			verts.push_back(th.normalACD.z);
			verts.push_back(th.c.x);
			verts.push_back(th.c.y);
			verts.push_back(th.c.z);
			verts.push_back(th.normalACD.x);
			verts.push_back(th.normalACD.y);
			verts.push_back(th.normalACD.z);
			verts.push_back(th.d.x);
			verts.push_back(th.d.y);
			verts.push_back(th.d.z);
			verts.push_back(th.normalACD.x);
			verts.push_back(th.normalACD.y);
			verts.push_back(th.normalACD.z);

			verts.push_back(th.a.x);
			verts.push_back(th.a.y);
			verts.push_back(th.a.z);
			verts.push_back(th.normalABC.x);
			verts.push_back(th.normalABC.y);
			verts.push_back(th.normalABC.z);
			verts.push_back(th.b.x);
			verts.push_back(th.b.y);
			verts.push_back(th.b.z);
			verts.push_back(th.normalABC.x);
			verts.push_back(th.normalABC.y);
			verts.push_back(th.normalABC.z);
			verts.push_back(th.c.x);
			verts.push_back(th.c.y);
			verts.push_back(th.c.z);
			verts.push_back(th.normalABC.x);
			verts.push_back(th.normalABC.y);
			verts.push_back(th.normalABC.z);

			verts.push_back(th.c.x);
			verts.push_back(th.c.y);
			verts.push_back(th.c.z);
			verts.push_back(th.normalCDB.x);
			verts.push_back(th.normalCDB.y);
			verts.push_back(th.normalCDB.z);
			verts.push_back(th.d.x);
			verts.push_back(th.d.y);
			verts.push_back(th.d.z);
			verts.push_back(th.normalCDB.x);
			verts.push_back(th.normalCDB.y);
			verts.push_back(th.normalCDB.z);
			verts.push_back(th.b.x);
			verts.push_back(th.b.y);
			verts.push_back(th.b.z);
			verts.push_back(th.normalCDB.x);
			verts.push_back(th.normalCDB.y);
			verts.push_back(th.normalCDB.z);

			verts.push_back(th.a.x);
			verts.push_back(th.a.y);
			verts.push_back(th.a.z);
			verts.push_back(th.normalABD.x);
			verts.push_back(th.normalABD.y);
			verts.push_back(th.normalABD.z);
			verts.push_back(th.b.x);
			verts.push_back(th.b.y);
			verts.push_back(th.b.z);
			verts.push_back(th.normalABD.x);
			verts.push_back(th.normalABD.y);
			verts.push_back(th.normalABD.z);
			verts.push_back(th.d.x);
			verts.push_back(th.d.y);
			verts.push_back(th.d.z);
			verts.push_back(th.normalABD.x);
			verts.push_back(th.normalABD.y);
			verts.push_back(th.normalABD.z);
		}
	}

	int GetTetrahedraCount(){
		return this->TetrahedraCount;
	}

private:

	void angularInertiaWithRespectToPoint(glm::vec3 &point, Tetrahedron &th, glm::mat3 &inertiaWrtPoint) {

		glm::mat3 I3; // 3x3 identity
		glm::vec3 ptToCentroid = th.centroid - point; // vector from point to centroid
		glm::mat3 ptToCtimesTranspose = glm::outerProduct(ptToCentroid, ptToCentroid); // ptToC * (ptToC)^T

		inertiaWrtPoint = th.angularInertia + th.mass * (glm::dot(ptToCentroid,ptToCentroid) * I3 - ptToCtimesTranspose);
	}

	void computeTetrahedronCentroid(Tetrahedron &th){
		double centroidX = (th.a.x + th.b.x + th.c.x + th.d.x) / 4.0;
		double centroidY = (th.a.y + th.b.y + th.c.y + th.d.y) / 4.0;
		double centroidZ = (th.a.z + th.b.z + th.c.z + th.d.z) / 4.0;
		th.centroid = glm::vec3(centroidX, centroidY, centroidZ);
	}

	void computeTetrahedronAngularInertia(Tetrahedron &th) {
		
		computeTetrahedronCentroid(th);

		//use vertices wrt centroid:
		glm::vec3 a = th.a - th.centroid;
		glm::vec3 b = th.b - th.centroid;
		glm::vec3 c = th.c - th.centroid;
		glm::vec3 d = th.d - th.centroid;

		glm::vec3 centroid(0.0, 0.0, 0.0); //essentially sets centroid to be origin

		glm::mat4 volMat(a.x, a.y, a.z, 1.0,
			b.x, b.y, b.z, 1.0,
			c.x, c.y, c.z, 1.0,
			d.x, d.y, d.z, 1.0);
		double det = abs(glm::determinant(volMat));
		double volume = det / 6.0; // (det * 1/3!) 
		th.volume = volume;

		//DET of Jacobian, see http://docsdrive.com/pdfs/sciencepublications/jmssp/2005/8-11.pdf
		double DETJ = 6 * volume;
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

};