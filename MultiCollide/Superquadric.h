#pragma once

#include <math.h>
#include <vector>
#include <map>

#include "Quaternion.h"
#include "ShapeSeparatingAxis.h"

const float SQ_EPSILON = 0.001f;

//float PI = glm::pi<float>();
//float PI_OVER_2 = PI / 2.0f;
//float PI_OVER_4 = PI_OVER_2 / 2.0f;

//class tuple2 {
//public: 
//	float v;
//	glm::vec3 pt;
//
//	tuple2(float vArg, glm::vec3 ptArg) : v(vArg), pt(ptArg) {}
//};
//
//class tuple1 {
//public: 
//	float u;
//	std::vector<tuple2> vs;
//
//	tuple1(float uArg, std::vector<tuple2> vsArg) : u(uArg) {
//		vs = vsArg;
//	}
//};

//class Points {
//public:
//	std::vector<tuple1> pts;
//
//	Points() {}
//
//	Points(std::map<float, std::map<float, glm::vec3>> map) {
//		for (std::map<float, std::map<float, glm::vec3>>::iterator iter = map.begin(); iter != map.end(); iter++) {
//			float u = iter->first;
//			std::map<float, glm::vec3> vs = iter->second;
//			
//			std::vector<tuple2> vVector;
//
//
//			for (std::map<float, glm::vec3>::iterator iter = vs.begin(); iter != vs.end(); iter++) {
//				tuple2 pt(iter->first, iter->second);
//				vVector.push_back(pt);
//			}
//			//std::cout << vVector.size() << std::endl;
//			
//			//sort vVector:
//			// Selection sort cuz I'm feeling lame: TODO use better alg 
//			for (int i = 0; i < vVector.size(); i++) {
//				float minV = vVector[i].v;
//				int minIndex = i;
//				float val;
//				for (int j = i + 1; j < vVector.size(); j++) {
//					if ( (val = vVector[j].v) < minV) {
//						minV = val;
//						minIndex = j;
//					}
//				}
//
//				tuple2 temp = vVector[i];
//				vVector[i] = vVector[minIndex];
//				vVector[minIndex] = temp;
//			}
//
//			pts.push_back(tuple1(u, vVector));
//		}
//
//		//And finally sort by u:
//		for (int i = 0; i < pts.size(); i++) {
//			float minU = pts[i].u;
//			int minIndex = i;
//			float val;
//			for (int j = i + 1; j < pts.size(); j++) {
//				if ((val = pts[j].u) < minU) {
//					minU = val;
//					minIndex = j;
//				}
//			}
//
//			tuple1 temp = pts[i];
//			pts[i] = pts[minIndex];
//			pts[minIndex] = temp;
//		}
//	}
//};

// Vertex generation adapted from: http://www.gamedev.net/page/resources/_/technical/opengl/superquadric-ellipsoids-and-toroids-opengl-lig-r1172
class Superquadric : public ShapeSeparatingAxis {

	static const bool print = false;

public:

	virtual glm::vec3 GetNormalAtPoint(ParamPoint &pt) override {
		return this->NormalFromSurfaceParams(pt.u, pt.v);
	}

	virtual bool DispatchCollisionDetection(Shape *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SQ DD 1" << std::endl;
		return other->DispatchCollisionDetection(this, closestPt2, closestPt1, pp2, pp1);
	}

	virtual bool DispatchCollisionDetection(Superquadric *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SQ DD 2" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

	virtual bool DispatchCollisionDetection(ShapeSeparatingAxis *other, glm::vec3 &closestPt1, glm::vec3 &closestPt2, ParamPoint &pp1, ParamPoint &pp2) override {
		//std::cout << "SQ DD 3" << std::endl;
		return CollisionDetector::Detect(*this, *other, closestPt1, closestPt2, pp1, pp2);
	}

	//From "Segmentation and Recovery of Superquadrics"
	virtual void ComputeInertia() override {
		float Ixx, Iyy, Izz;

		float betas = Beta(1.5f * e2, .5f * e2) * Beta(.5f * e1, 2.0f*e1 + 1.0f);

		float pre = .5f * a1 * a2 * a3 * e1 * e2;
		float end = 4.0f * a3*a3 *
			Beta(.5f * e2, .5f * e2 + 1.0f) * Beta(1.5f * e1, e1 + 1.0f);

		Ixx = pre * (a2 * a2 * betas + end);

		Iyy = pre * (a1 * a1 * betas + end);

		Izz = pre * (a1*a1 + a2*a2) * betas;

		angularInertia = glm::mat3();
		angularInertia[0][0] = Ixx;
		angularInertia[1][1] = Iyy;
		angularInertia[1][1] = Izz;
	}

	float Beta(float x, float y) {
		return 1.0f;
	}

	float a1, a2, a3;       		 /* Scaling factors for x, y, and z */
									 //float alpha;      				 /* For generating toroids. This is the inner radius */
	float e1, e2;             		 /* North-South/East-West Roundness/Squareness Factors */
	float u1, u2;     				 /* Initial and Final U values */
	float v1, v2;     				 /* Initial and Final V values */
	int u_segs;       				 /* Number of segments for U */
	int v_segs;       				 /* Number of segments for V */
	std::vector<GLfloat> vertices;   /* Stores points & normals */

									 //Created in uniform surface area && used in collision detection:
	std::vector<ParamPoint> points;

	std::map<float, std::map<float, glm::vec3>> pointsMap;

	int csvCode;

	std::vector<float> allTriangleAreas;

	static const int COORDS_PER_VERTEX = 6;

	const float APPROX_DERIVATIVE_H = sqrtf(FLT_EPSILON);
	std::vector<glm::vec2> nanAdjustments;
	float adjustment = .05f;

	glm::vec3 afterVelocity;

	std::vector<glm::vec3> nonsingulars;

	Superquadric() {
		nanAdjustments.push_back(glm::vec2(adjustment, 0.0f));
		nanAdjustments.push_back(glm::vec2(0.0f, adjustment));
		nanAdjustments.push_back(glm::vec2(adjustment, adjustment));
		nanAdjustments.push_back(glm::vec2(adjustment, -adjustment));
		nanAdjustments.push_back(glm::vec2(-adjustment, 0.0f));
		nanAdjustments.push_back(glm::vec2(0.0f, -adjustment));
		nanAdjustments.push_back(glm::vec2(-adjustment, -adjustment));
		nanAdjustments.push_back(glm::vec2(-adjustment, adjustment));
	}

	virtual void InitVAOandVBO(Shader &shader) override {

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

		glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(this->model));
		glUniform4f(shader.getUniform("objectColor"), objectColor.x, objectColor.y, objectColor.z, 1.0f);
		glBindVertexArray(this->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
		glBindVertexArray(0);

	}

	virtual ~Superquadric() override {
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &VAO);
	}

	void CreateSphere() {
		this->a1 = 1.0f;
		this->a2 = 1.0f;
		this->a3 = 1.0f;
		this->e2 = 1.0f;
		this->e1 = 1.0f;
		this->u1 = -glm::pi<float>() / 2.0f;
		this->u2 = glm::pi<float>() / 2.0f;
		this->v1 = -glm::pi<float>();
		this->v2 = glm::pi<float>();
		this->u_segs = 23;
		this->v_segs = 23;
	}

	glm::mat3 getPointBasis(glm::vec3 localPt) {
		//basis C for localPt is C = (a, b, c), where c is the normal to the surface, a, b span tangential plane
		glm::vec3 c = this->unitnormal(localPt);

		//get 2 orthogonal vectors:
		glm::vec3 a = glm::vec3(-c.y, c.x, 0.0f);
		if (MathUtils::abs(c.x) < .0001f && MathUtils::abs(c.y) < .0001f) {
			a = glm::vec3(0.0f, c.z, 0.0f);
		}

		glm::vec3 b = glm::cross(c, a);

		return glm::mat3(glm::normalize(a), glm::normalize(b), glm::normalize(c));
	}

#pragma region Derivatives

	// (0, 0, 0) at u = +/- PI/2
	glm::vec3 dSigmaDv(float u, float v) {
		glm::vec3 out;
		float cosuToTheE1, sinv, cosv, cosu;

		cosu = MathUtils::cos(u);

		cosuToTheE1 = MathUtils::sgnf(cosu) * (MathUtils::abs(cosu), e1);
		sinv = MathUtils::sin(v);
		cosv = MathUtils::cos(v);

		if (this->isSphere() || (e1 == 1.0f && e2 == 1.0f)) {
			return glm::vec3(a1 * cosu *-sinv, a2 * cosu * cosv, 0.0f);
		}

		out.x = MathUtils::sgnf(cosv) * a1 * cosuToTheE1 * (-1.0f * e2 * powf(MathUtils::abs(cosv), e2 - 1.0f) * sinv);
		out.y = MathUtils::sgnf(sinv) * a2 * cosuToTheE1 * (e2 * powf(MathUtils::abs(sinv), e2 - 1.0f) * cosv);
		out.z = 0.0f;

		return out;
	}

	glm::vec3 dSigmaDu(float u, float v) {
		glm::vec3 out;
		float cosv, cosuToTheE1Minus1, sinv, end, sinu, cosu;
		cosu = MathUtils::cos(u);
		sinu = MathUtils::sin(u);
		sinv = MathUtils::sin(v);
		cosv = MathUtils::cos(v);

		cosuToTheE1Minus1 = powf(cosu, e1 - 1.0f);

		if (this->isSphere() || (e1 == 1.0f && e2 == 1.0f)) {
			return glm::vec3(a1 * cosv * -sinu, a2 * -sinu * sinv, a3 * -cosu);
		}

		end = (-1.0f * e1 * cosuToTheE1Minus1 * sinu);

		out.x = MathUtils::sgnf(cosv) * a1 * powf(MathUtils::abs(cosv), e2) * end;
		out.y = MathUtils::sgnf(sinv) * a2 * powf(MathUtils::abs(sinv), e2) * end;
		out.z = MathUtils::sgnf(sinu) * a3 * e1 * powf(MathUtils::abs(sinu), e1 - 1.0f) * cosu;

		if (IsZeroVector(out)) {
			out.z = 1.0f;
		}

		return out;
	}

	glm::vec3 ddSigmaDvDv(float u, float v) {
		glm::vec3 out;
		float cosv, cosuToTheE1, sinv;

		cosv = MathUtils::cos(v);
		sinv = MathUtils::sin(v);
		cosuToTheE1 = powf(MathUtils::cos(u), e1);

		float e2Minus1 = e2 - 1.0f;

		out.x = MathUtils::sgnf(cosv) * -e2 * a1 * cosuToTheE1 * (-1.0f * (e2Minus1) * powf(MathUtils::abs(cosv), e2 - 2.0f) * sinv * sinv + cosv * powf(MathUtils::abs(cosv), e2Minus1));
		out.y = MathUtils::sgnf(sinv) *  e2 * a1 * cosuToTheE1 * ((e2Minus1) * powf(MathUtils::abs(sinv), e2 - 2.0f) * cosv * cosv - sinv * powf(MathUtils::abs(sinv), e2Minus1));
		out.z = 0.0f;

		return out;
	}

	glm::vec3 ddSigmaDuDv(float u, float v) {
		glm::vec3 out;
		float sinu, sinv, cosv, cosuToTheE1Minus1, end;

		sinv = MathUtils::sin(v);
		cosv = MathUtils::cos(v);
		sinu = MathUtils::sin(u);
		cosuToTheE1Minus1 = powf(MathUtils::cos(u), e1 - 1.0f);
		end = e1 * cosuToTheE1Minus1 * sinu;

		out.x = MathUtils::sgnf(cosv) * e2 * a1 * sinv * powf(MathUtils::abs(cosv), e2 - 1.0f) * end;
		out.y = MathUtils::sgnf(sinv) * e2 * a2 * cosv * powf(MathUtils::abs(sinv), e2 - 1.0f) * -1.0f * end;
		out.z = 0.0f;

		return out;
	}

	glm::vec3 ddSigmaDuDu(float u, float v) {
		glm::vec3 out;
		float sinv, cosv, cosuToTheE1Minus2, cosuToTheE1Minus1, cosu, sinu;
		float e1Minus1 = e1 - 1.0f;
		sinu = MathUtils::sin(u);
		cosu = MathUtils::cos(u);
		cosv = MathUtils::cos(v);
		sinv = MathUtils::sin(v);
		cosuToTheE1Minus1 = powf(cosu, e1Minus1);
		cosuToTheE1Minus2 = powf(cosu, e1 - 2.0f);

		float inner = e1 * (-1.0f * (e1Minus1) * cosuToTheE1Minus2 * sinu * sinu + cosu * cosuToTheE1Minus1);
		out.x = MathUtils::sgnf(cosv) * -a1 * powf(MathUtils::abs(cosv), e2) * inner;
		out.y = MathUtils::sgnf(sinv) * -a2 * powf(MathUtils::abs(sinv), e2) * inner;

		out.z = a3 * e1 * (e1Minus1 * powf(sinv, e1 - 2.0f) * cosv * cosv - sinv * powf(sinv, e1Minus1));

		return out;
	}

#pragma endregion 

	static float SurfaceArea(Superquadric &sq, float u, float v) {
		glm::vec3 fff = FirstFundamentalForm(sq, u, v);
		return sqrtf(fff.x * fff.z - fff.y * fff.y);
	}

	//Returns <E,F,G>, where E = sigmaU * sigmaU, F = sigmaU*sigmaV, and G = sigmaV * sigmaV
	static glm::vec3 FirstFundamentalForm(Superquadric &sq, float u, float v) {
		glm::vec3 sigmaU, sigmaV;

		sigmaU = sq.dSigmaDu(u, v);
		sigmaV = sq.dSigmaDv(u, v);

		float E = glm::dot(sigmaU, sigmaU);
		float F = glm::dot(sigmaV, sigmaU);
		float G = glm::dot(sigmaV, sigmaV);

		return glm::vec3(E, F, G);
	}

	static glm::vec3 SlideUpdate(Superquadric &sq, ParamPoint &contactPt, glm::vec3 slideDirection) {

		glm::vec2 inputUV = glm::vec2(contactPt.u, contactPt.v);
		glm::vec2 curUV = inputUV;

		float slideMagnitude = sqrtf(slideDirection.x * slideDirection.x + slideDirection.y * slideDirection.y + slideDirection.z  * slideDirection.z);
		float movedMagnitude = 0.0f;

		//put contact pt in local space: 
		glm::vec3 globalContactOriginal(contactPt.pt);
		glm::mat4 rotMat = sq.getRotationMatrix();
		glm::vec3 localContactPt = ShapeUtils::getLocalCoordinates(contactPt.pt, sq.translation, rotMat, sq.scaling);
		glm::vec3 origLocalContactPt = localContactPt;

		float origInsideOutsideValue = sq.f(localContactPt);
		if (origInsideOutsideValue < .98f) {
			std::cout << "input Stationary IO value: " << origInsideOutsideValue << std::endl;
		}

		glm::vec3 slidePt;

		glm::vec2 sysVal;
		float totSysVal;

		float deltaT = .01f;
		int i = 0;

		glm::vec2 lastUpdate;

		while (i < 100) {
			glm::vec3 previousPt = localContactPt;
			glm::vec3 fff = FirstFundamentalForm(sq, curUV.x, curUV.y);

			//Trying to solve system: 
			// | E F |   | u. |  =  | sigmaU * vp |
			// | F G |   | v. |  =  | sigmaV * vp |

			// for (u., v.)

			// | E F |
			// | F G |
			float EFFG[2][2] = { { fff.x, fff.y },
			{ fff.y, fff.z } };

			float EFFGdet = EFFG[0][0] * EFFG[1][1] - EFFG[0][1] * EFFG[1][0];

			// | u. |
			// | v. |
			float uDotvDot[2];

			glm::vec3 sigmaU = glm::normalize(sq.dSigmaDu(curUV.x, curUV.y));
			glm::vec3 sigmaV = glm::normalize(sq.dSigmaDv(curUV.x, curUV.y));

			//glm::vec3 vp = projection(vp, sigmaU) + projection(vp, )

			////Slide direction needs to be in sq's local space:
			//glm::vec3 slide = contactPt.pt + Superquadric::getLocalCoordinates(slideDirection, sq.translation, sq.getRotationMatrix());
			glm::mat4 transpose = glm::transpose(sq.getRotationMatrix());
			slideDirection = glm::normalize(Superquadric::applyRotation(slideDirection, transpose));

			// | sigmaU * slideDir |
			// | sigmaV * slideDir |

			glm::vec3 vProj = (glm::dot(sigmaU, slideDirection)*sigmaU + glm::dot(sigmaV, slideDirection)*sigmaV);
			//glm::vec3 tangNormal = glm::normalize(glm::cross(sigmaU, sigmaV));
			//glm::vec3 vProj = slideDirection - (glm::dot(tangNormal, slideDirection) * tangNormal);

			float sigmaUsigmaVdotprods[2] = { glm::dot(vProj, sigmaU), glm::dot(vProj, sigmaV) };

			//Solve (Cramer's rule):
			uDotvDot[0] = (sigmaUsigmaVdotprods[0] * EFFG[1][1] - sigmaUsigmaVdotprods[1] * EFFG[0][1]) / EFFGdet;
			uDotvDot[1] = (sigmaUsigmaVdotprods[1] * EFFG[0][0] - sigmaUsigmaVdotprods[0] * EFFG[1][0]) / EFFGdet;

			//Check if this solution is valid:
			sysVal.x = fff.x * uDotvDot[0] + fff.y * uDotvDot[1] - sigmaUsigmaVdotprods[0];
			sysVal.y = fff.y * uDotvDot[0] + fff.z * uDotvDot[1] - sigmaUsigmaVdotprods[1];

			totSysVal = MathUtils::abs(sysVal.x) + MathUtils::abs(sysVal.y);

			//Update by constant distance
			// deltaS = sqrt( E*uDot^2 + 2*F*uDot*vDot + G*vDot^2 ) * deltaT
			//Fix deltaS, since it represents the change of curve length along the surface,
			// and solve for deltaT using the calculated values of uDot, vDot, E,F,G

			float deltaS = .015f;
			float sqrtVal = sqrtf(fff.x * uDotvDot[0] * uDotvDot[0] + 2.0f * fff.y * uDotvDot[0] * uDotvDot[1] + fff.z * uDotvDot[1] * uDotvDot[1]);

			float deltaT = deltaS / sqrtVal;

			///std::cout << "Delta T: " << deltaT << std::endl;

			lastUpdate.x = deltaT * uDotvDot[0];
			lastUpdate.y = deltaT * uDotvDot[1];
			curUV = curUV + lastUpdate;
			glm::vec3 i3 = glm::vec3();
			sq.evalParams(curUV.x, curUV.y, localContactPt, i3);

			glm::vec3 diffVec = origLocalContactPt - localContactPt;
			if (MathUtils::abs(diffVec.x) > .1f || MathUtils::abs(diffVec.y) > .1f || MathUtils::abs(diffVec.z) > .1f) {
				curUV = curUV - (.5f * lastUpdate);
				sq.evalParams(curUV.x, curUV.y, localContactPt, i3);

				glm::vec3 diffVec = origLocalContactPt - localContactPt;
				if (MathUtils::abs(diffVec.x) > .1f || MathUtils::abs(diffVec.y) > .1f || MathUtils::abs(diffVec.z) > .1f) {
					curUV = curUV - (.25f * lastUpdate);
					sq.evalParams(curUV.x, curUV.y, localContactPt, i3);
				}
			}

			if (totSysVal < .01f) {
				break;
			}

			

			i++;

			movedMagnitude += glm::distance(origLocalContactPt, localContactPt);

			if (movedMagnitude > .1f) {
				///std::cout << "Moved mag: " << std::to_string(movedMagnitude) << std::endl;
			}
		}

		//std::cout << "Solved system in " << i << " iterations" << std::endl;

		/*

		//  v = mouse movement (slide direction)
		// vp = projection of V onto tangent plane

		//Holds value of system (left side - rightside)
		glm::vec2 systemVal(0.0f, 0.0f);

		//Holds summed absolute value of system (goal is 0)
		float totSysVal = 100.0f;

		float deltaT = .1f;
		int i = 0;
		while (i < 100) {

			glm::vec3 previousPt = localContactPt;
			glm::vec3 fff = FirstFundamentalForm(sq, curUV.x, curUV.y);

			//Trying to solve system: 
			// | E F |   | u. |  =  | sigmaU * vp |
			// | F G |   | v. |  =  | sigmaV * vp |

			// for (u., v.)

			// | E F |
			// | F G |
			float EFFG[2][2] = { { fff.x, fff.y },
			{ fff.y, fff.z } };

			float EFFGdet = EFFG[0][0] * EFFG[1][1] - EFFG[0][1] * EFFG[1][0];

			// | u. |
			// | v. |
			float uDotvDot[2];

			glm::vec3 sigmaU = glm::normalize(sq.dSigmaDu(curUV.x, curUV.y));
			glm::vec3 sigmaV = glm::normalize(sq.dSigmaDv(curUV.x, curUV.y));

			//glm::vec3 vp = projection(vp, sigmaU) + projection(vp, )

			////Slide direction needs to be in sq's local space:
			//glm::vec3 slide = contactPt.pt + Superquadric::getLocalCoordinates(slideDirection, sq.translation, sq.getRotationMatrix());
			glm::mat4 transpose = glm::transpose(sq.getRotationMatrix());
			slideDirection = Superquadric::applyRotation(slideDirection, transpose);

			// | sigmaU * slideDir |
			// | sigmaV * slideDir |

			glm::vec3 vProj = (glm::dot(sigmaU, slideDirection)*sigmaU + glm::dot(sigmaV, slideDirection)*sigmaV);
			float sigmaUsigmaVdotprods[2] = { glm::dot(vProj, sigmaU), glm::dot(vProj, sigmaV) };

			//Solve (Cramer's rule):
			uDotvDot[0] = (sigmaUsigmaVdotprods[0] * EFFG[1][1] - sigmaUsigmaVdotprods[1] * EFFG[0][1]) / EFFGdet;
			uDotvDot[1] = (sigmaUsigmaVdotprods[1] * EFFG[0][0] - sigmaUsigmaVdotprods[0] * EFFG[1][0]) / EFFGdet;

			//Check if this solution is valid:
			systemVal.x = fff.x * uDotvDot[0] + fff.y * uDotvDot[1] - sigmaUsigmaVdotprods[0];
			systemVal.y = fff.y * uDotvDot[0] + fff.z * uDotvDot[1] - sigmaUsigmaVdotprods[1];

			totSysVal = MathUtils::abs(systemVal.x) + MathUtils::abs(systemVal.y);

			if (totSysVal < .01f) {
				break;
			}

			curUV.x = curUV.x + deltaT * uDotvDot[0];
			curUV.y = curUV.y + deltaT * uDotvDot[1];

			glm::vec3 i3 = glm::vec3();
			sq.evalParams(curUV.x, curUV.y, localContactPt, i3);

			i++;

			//movedMagnitude += glm::distance(contactPt.pt, previousPt);

			//std::cout << "Moved mag: " << std::to_string(movedMagnitude) << std::endl;

		}
		

		*/
		/*

		// | u. |
		// | v. |
		float uDotvDot[2];

		//Vectors spanning the tangent plane at contact pt:
		glm::vec3 sigmaU = glm::normalize(sq.dSigmaDu(inputUV.x, inputUV.y));
		glm::vec3 sigmaV = glm::normalize(sq.dSigmaDv(inputUV.x, inputUV.y));

		//Slide direction needs to be in sq's local space: TODO ALSO UNDO VIEW MATRIX
		glm::mat4 transpose = glm::transpose(sq.getRotationMatrix()); //transpose of rotation mat is inverse
		slideDirection = Superquadric::applyRotation(slideDirection, transpose);

		// | sigmaU * slideDir |
		// | sigmaV * slideDir |

		glm::vec3 projOntoSigmaU = glm::dot(sigmaU, slideDirection)*sigmaU;
		glm::vec3 projOntoSigmaV = glm::dot(sigmaV, slideDirection)*sigmaV;
		float sigmaUupdate = sqrtf(projOntoSigmaU.x * projOntoSigmaU.x + projOntoSigmaU.y * projOntoSigmaU.y + projOntoSigmaU.z * projOntoSigmaU.z);
		float sigmaVupdate = sqrtf(projOntoSigmaV.x * projOntoSigmaV.x + projOntoSigmaV.y * projOntoSigmaV.y + projOntoSigmaV.z * projOntoSigmaV.z);

		inputUV.x = inputUV.x + sigmaUupdate;
		inputUV.y = inputUV.y + sigmaVupdate;

		glm::vec3 i3 = glm::vec3();
		sq.evalParams(inputUV.x, inputUV.y, localContactPt, i3);

		movedMagnitude = MathUtils::magnitude(origLocalContactPt - localContactPt);
		if (movedMagnitude > .01f) {
			std::cout << "Big slide update " << std::endl;

			inputUV.x = inputUV.x - .5f * sigmaUupdate;
			inputUV.y = inputUV.y - .5f * sigmaVupdate;

			sq.evalParams(inputUV.x, inputUV.y, localContactPt, i3);

			movedMagnitude = MathUtils::magnitude(origLocalContactPt - localContactPt);

			if (movedMagnitude > .01f) {
				std::cout << "still big after adjustment" << std::endl;

				inputUV.x = inputUV.x - .25f * sigmaUupdate;
				inputUV.y = inputUV.y - .25f * sigmaVupdate;

				sq.evalParams(inputUV.x, inputUV.y, localContactPt, i3);
				movedMagnitude = MathUtils::magnitude(origLocalContactPt - localContactPt);

				if (movedMagnitude > .01f) {
					inputUV.x = contactPt.u + .01f * sigmaUupdate;
					inputUV.y = contactPt.v + .01f * sigmaVupdate;
					sq.evalParams(inputUV.x, inputUV.y, localContactPt, i3);
					movedMagnitude = MathUtils::magnitude(origLocalContactPt - localContactPt);
				}
			}
		}
		*/

		float insideOutsideValue = sq.f(localContactPt);
		if (insideOutsideValue < .98f) { 
			///std::cout << "Stationary IO value: " << insideOutsideValue << std::endl;
		}

		// Reapply model transformation:
		slidePt = ShapeUtils::getGlobalCoordinates(localContactPt, sq.translation, rotMat, sq.scaling);

		if (checkVecForNaN(slidePt)) {
			///std::cout << " bad derivatives" << std::endl;
		}

		//Return vector from contact pt to slide pt
		glm::vec3 update = slidePt - globalContactOriginal;
		if (IsZeroVector(update)) {
			///std::cout << "No update" << std::endl;
		}

		contactPt.u = curUV.x;
		contactPt.v = curUV.y;
		contactPt.pt = slidePt;
		
		return update;
	}

	//Returns point in local space
	static glm::vec3 RotatingSurfaceUpdate(Superquadric &rotatingSq, Superquadric &stationarySq, ParamPoint &rotatingSurfaceContactPt, MultiCollide::Quaternion update) {

		// Let p be rotatingSq's contact point (in parametrization domain)
		// Let q be rotatingSq's current orientation quaternion
		// Let n be the normal at p in the world frame (q applied)
		// Let r be  the quaternion update to be applied to q
		// Want to find the point b, s.t.  n = r q (normal(b)) q* r*
		// or normal(b) = q* r* (n) r q

		//Approach is to find point that minifies q* r* (n) r q - normal(b)

		glm::vec2 p(rotatingSurfaceContactPt.u, rotatingSurfaceContactPt.v);
		glm::mat4 rotMat = rotatingSq.getRotationMatrix();
		MultiCollide::Quaternion q(rotMat);
		MultiCollide::Quaternion r = update;
		glm::vec3 localNormal = rotatingSq.NormalFromSurfaceParams(rotatingSurfaceContactPt.u, rotatingSurfaceContactPt.v);
		glm::vec3 n = q.Rotate(localNormal);

		//glm::quat q(rotatingSq.getRotationMatrix());

		MultiCollide::Quaternion rq = r.Multiply(q);
		MultiCollide::Quaternion rqConj = rq.Conjugate();

		glm::vec3 rqConjOnN = glm::normalize(rqConj.Rotate(n));

		glm::vec2 curB(p.x, p.y);

		float PI_OVER_8 = glm::pi<float>() / 4.0f;
		float start = -PI_OVER_8;
		float end = PI_OVER_8;
		float uvUpdate = PI_OVER_8 / 64.0f;

		glm::vec2 bestB = curB;
		glm::vec3 rqConjOnNminusCurB = rqConjOnN - rotatingSq.NormalFromSurfaceParams(curB);
		float best = glm::dot(rqConjOnNminusCurB, rqConjOnNminusCurB);

		for (float uoffset = start; uoffset <= end; uoffset += uvUpdate) {
			for (float voffset = start; voffset <= end; voffset += uvUpdate) {
				curB = glm::vec2(p.x + uoffset, p.y + voffset);

				rqConjOnNminusCurB = rqConjOnN - rotatingSq.NormalFromSurfaceParams(curB);

				float dist = glm::dot(rqConjOnNminusCurB, rqConjOnNminusCurB);
				if (dist < best) {
					best = dist;
					bestB = curB;
				}
			}
		}

		glm::vec3 inputNormal = n;
		glm::vec3 outputNormal = rq.Rotate(rotatingSq.NormalFromSurfaceParams(bestB.x, bestB.y));
		return rotatingSq.PointsFromSurfaceParams(bestB.x, bestB.y);
	}

	//Returns point in global space
	// Can try solving with cross product and just using R3 (xyz points and gradient)

	//TODO not considering rotation of objects

	//slidePt is new contact pt on stationarySq after applying slide
	static glm::vec3 SlidingSurfaceUpdate(Superquadric &slidingSq, Superquadric &stationarySq, ParamPoint &slidingSurfaceContactPt, ParamPoint &slidePt) {

		glm::mat4 slidingSqRotMat = slidingSq.getRotationMatrix();
		glm::vec3 localContactSlidePt = ShapeUtils::getLocalCoordinates(slidingSurfaceContactPt.pt, slidingSq.translation, slidingSqRotMat, slidingSq.scaling);



		glm::mat4 stationaryRotMat = stationarySq.getRotationMatrix();
		glm::vec3 slidePtLocal = ShapeUtils::getLocalCoordinates(slidePt.pt, stationarySq.translation, stationaryRotMat, stationarySq.scaling);

		glm::vec3 localStationaryNormal;// = stationarySq.unitnormal(slidePtLocal);
		glm::vec3 temp;
		stationarySq.evalParams(slidePt.u, slidePt.v, temp, localStationaryNormal);

		glm::vec3 globalStationaryNormal = glm::vec3(stationaryRotMat * glm::vec4(localStationaryNormal.x, localStationaryNormal.y, localStationaryNormal.z, 1.0f));

		//Trying to find point on slidingSq whose normal vector is opposite direction of slidePt's normal on stationarySq
		glm::vec3 globalSlidingNorm = -globalStationaryNormal;
		//Need to search for pt in local space with this normal:
		glm::vec3 localSlidingNorm = glm::vec3(glm::transpose(slidingSqRotMat) * glm::vec4(globalSlidingNorm.x, globalSlidingNorm.y, globalSlidingNorm.z, 1.0f));
		glm::vec3 goalNorm = glm::normalize(localSlidingNorm);

		///std::cout << "Goal norm: " << goalNorm.x << ", " << goalNorm.y << ", " << goalNorm.z << std::endl;

		//slidingSq.n

		//Want to find pt on slidingSq with normal opposite to slidePtNorm
		// i.e. pt whose tangent plane has that same normal, i.e. sigmaU dot N = 0 & sigmaV dot N = 0
		// This gives the system: 
		//  |sigmaU dot N| = |0|
		//  |sigmaV dot N|   |0|

		//use newton's method and Jacobian to get system:
		//  |sigmaUU dot N   sigmaUV dot N| |u.| = | - sigmaU dot N |
		//  |sigmaUV dot N   sigmaVV dot N| |v.|   | - sigmaV dot N |

		/*
		int i = 0;
		int maxIterations = 50;
		float deltaT = .06f;

		glm::mat2 J;

		glm::vec2 origUV = glm::vec2(slidingSurfaceContactPt.u, slidingSurfaceContactPt.v);
		glm::vec2 UiVi = glm::vec2(slidingSurfaceContactPt.u, slidingSurfaceContactPt.v);
		//Right side of first system. Stops when both elements are 0:
		glm::vec2 Yi = evalSSU(UiVi, slidingSq, goalNorm);

		while (!IsZeroVector(Yi) && i < maxIterations) {
			J = evalJacobianSSU(UiVi, slidingSq, goalNorm);

			//TODO eval more efficiently:

			glm::mat2 Jinverse = glm::inverse(J);
			glm::vec2 deltaUV = Jinverse * glm::vec2(-Yi.x, -Yi.y);

			UiVi += deltaT * deltaUV;

			Yi = evalSSU(UiVi, slidingSq, goalNorm);

			i++;
		}

		if (i == maxIterations) {
			//return nan's. 
			//TODO better practice:
			return glm::vec3(powf(0.0, -2.0f), powf(0.0, -2.0f), powf(0.0, -2.0f));
		}
		

		glm::vec3 pt = slidingSq.PointsFromSurfaceParams(UiVi.x, UiVi.y);

		if (checkVecForNaN(pt)) {
			std::cout << "Bad point" << std::endl;
		}

		//Pt is orthogonal to normal, but going wrong direction
		// if this condition is true, the normals are parallel, we want the opposite direction normal
		if (glm::dot(slidingSq.unitnormal(pt), goalNorm) < 0.0f) {
			return glm::vec3(powf(0.0, -2.0f), powf(0.0, -2.0f), powf(0.0, -2.0f));
		}

		//glm::mat4 slidingSqRotMat = slidingSq.getRotationMatrix();
		return Superquadric::getGlobalCoordinates(pt, slidingSq.translation, slidingSqRotMat);

		//brute force attempt:

		*/

		float u, v;
		u = slidingSurfaceContactPt.u;
		v = slidingSurfaceContactPt.v;

		float origU = u;
		float origV = v;

		float delta = .001f;
		//glm::vec3 bestNorm = slidingSq.unitnormal(localContactSlidePt);//slidingSq.NormalFromSurfaceParams(u, v);
		glm::vec3 bestNorm;
		slidingSq.evalParams(u, v, localContactSlidePt, bestNorm);

		int iterations = 0;

		glm::vec3 diffVec = goalNorm - bestNorm;

		//while (checkVecs(goalNorm, bestNorm) > .005f) {
		while(diffVec.x > .001f && diffVec.y > .001f && diffVec.z > .001f){
			if (iterations == 100) {
				delta *= 10.0f;
			}
			if (iterations == 200) {
				delta /= 10.0f;
			}
			if (iterations == 300) {
				delta *= 10.0f;
			}
			if (iterations == 400) {
				delta *= 10.0f;
			}
			if (iterations == 500) {
				delta /= 10.0f;
			}
			if (iterations == 600) {
				delta /= 10.0f;
			}
			if (iterations > 1000) {
				std::cout << "> 1000 iters to find normal" << std::endl;
				break;
			}
			glm::vec3 pt, extra;
			slidingSq.evalParams(u + delta, v, pt, extra);
			glm::vec3 addU = glm::normalize(extra);// slidingSq.unitnormal(pt);

			slidingSq.evalParams(u - delta, v, pt, extra);
			glm::vec3 subU = glm::normalize(extra);//slidingSq.unitnormal(pt);

			slidingSq.evalParams(u, v + delta, pt, extra);
			glm::vec3 addV = glm::normalize(extra);//slidingSq.unitnormal(pt);

			slidingSq.evalParams(u, v - delta, pt, extra);
			glm::vec3 subV = glm::normalize(extra);//slidingSq.unitnormal(pt);

			int bestOf4 = findClosestOf4(addU, subU, addV, subV, goalNorm);
			if (bestOf4 == 1) {
				u = u + delta;
				bestNorm = addU;
			}
			else if (bestOf4 == 2) {
				u = u - delta;
				bestNorm = subU;
			}
			else if (bestOf4 == 3) {
				v = v + delta;
				bestNorm = addV;
			}
			else {
				v = v - delta;
				bestNorm = subV;
			}

			if (checkVecForNaN(bestNorm)) {
				///std::cout << "nan" << std::endl;
			}

			diffVec = goalNorm - bestNorm;

			iterations++;
		}

		glm::vec3 pt, norm;
		slidingSq.evalParams(u, v, pt, norm);
		float insideOutsideValue = slidingSq.f(pt);
		///std::cout << "Sliding IO value: " << insideOutsideValue << std::endl;

		float deltaU = MathUtils::abs(u - origU);
		float deltaV = MathUtils::abs(v - origV);

		///std::cout << "DeltaU SSU: " << deltaU << std::endl;
		///std::cout << "DeltaV SSU: " << deltaV << std::endl;

		return ShapeUtils::getGlobalCoordinates(pt, slidingSq.translation, slidingSqRotMat, slidingSq.scaling);
		
	}

	//Evals first system in SlidingSurfaceUpdate (above)
	//  |sigmaU dot N| = |0|
	//  |sigmaV dot N|   |0|
	static glm::vec2 evalSSU(glm::vec2 uv, Superquadric &sq, glm::vec3 norm) {
		glm::vec3 sigU, sigV;
		sigU = glm::normalize(sq.dSigmaDu(uv.x, uv.y));
		sigV = glm::normalize(sq.dSigmaDv(uv.x, uv.y));

		return glm::vec2(glm::dot(sigU, norm), glm::dot(sigV, norm));
	}

	static glm::mat2 evalJacobianSSU(glm::vec2 uv, Superquadric &sq, glm::vec3 norm) {
		glm::vec3 sigUU, sigUV, sigVV, sigUUhat, sigUVhat, sigVVhat;
		sigUU = sq.ddSigmaDuDu(uv.x, uv.y);
		sigUV = sq.ddSigmaDuDv(uv.x, uv.y);
		sigVV = sq.ddSigmaDvDv(uv.x, uv.y);

		sigUUhat = glm::normalize(sigUU);
		sigUVhat = glm::normalize(sigUV);
		sigVVhat = glm::normalize(sigVV);

		if (checkVecForNaN(sigUU) || checkVecForNaN(sigVV) || checkVecForNaN(sigUV)) {
			std::cout << "nan 2nd derivs" << std::endl;
		}
		return glm::mat2(glm::vec2(glm::dot(sigUUhat, norm), glm::dot(sigUVhat, norm)), glm::vec2(glm::dot(sigUVhat, norm), glm::dot(sigVVhat, norm)));
	}

	static bool IsZeroVector(glm::vec2 vec) {
		return MathUtils::abs(vec.x) < .001f && MathUtils::abs(vec.y) < .001f;
	}

	static bool IsZeroVector(glm::vec3 vec) {
		return MathUtils::abs(vec.x) < .0001f && MathUtils::abs(vec.y) < .0001f && MathUtils::abs(vec.z) < .0001f;
	}

	static void InitializeClosestPoints(Superquadric &sq) {
		sq.points.clear();

		float u, v;
		glm::vec3 vec, norm;
		float PI = glm::pi<float>();

		u = 0.0f;
		v = 0.0f;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });

		sq.pointsMap[u][v] = vec;

		u = 0.0f;
		v = PI / 2.0f;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		u = 0.0f;
		v = -PI;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		u = 0.0f;
		v = -PI / 2.0f;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		u = -PI / 2.0f;
		v = -PI / 2.0f;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		u = PI / 2.0f;
		v = PI / 2.0f;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		//Halfways:

		u = PI / 4.0f;
		v = u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = -u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = 3 * u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = -3 * u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		u = -PI / 4.0f;
		v = u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = -u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = 3 * u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		v = -3 * u;
		sq.evalParams(u, v, vec, norm);
		sq.corners.push_back(norm);
		sq.normals.push_back(norm);
		sq.points.push_back({ u, v, vec });
		sq.pointsMap[u][v] = vec;

		//u = PI / 4.0f;
		//v = PI;
		//sq.evalParams(u, v, vec, norm);
		//sq.points.push_back({ u, v, vec });
		//sq.pointsMap[u][v] = vec;
	}

	static void ClosestPointFrameworkTryPrevious(Superquadric &sq1, Superquadric &sq2, glm::vec3 &sq1closest, glm::vec3 &sq2closest, ParamPoint &prev1, ParamPoint &prev2) {
		float offset = glm::pi<float>() / 4.0f;

		glm::mat4 sq1RotMat = sq1.getRotationMatrix();
		glm::mat4 sq2RotMat = sq2.getRotationMatrix();

		if (sq1.isSphere() && sq2.isSphere()) {
			glm::vec3 sq1ToSq2 = sq2.translation - sq1.translation;
			glm::vec2 param1 = sq1.SurfaceParamValuesFromSurfacePoint(glm::normalize(sq1ToSq2));
			sq1closest = ShapeUtils::getGlobalCoordinates(glm::normalize(sq1ToSq2), sq1.translation, sq1RotMat, sq1.scaling);

			glm::vec3 sq2ToSq1 = sq1.translation - sq2.translation;
			glm::vec2 param2 = sq2.SurfaceParamValuesFromSurfacePoint(glm::normalize(sq2ToSq1));
			sq2closest = ShapeUtils::getGlobalCoordinates(glm::normalize(sq2ToSq1), sq2.translation, sq2RotMat, sq2.scaling);

			prev1.pt = sq1closest;
			prev1.u = param1.x;
			prev1.v = param1.y;

			prev2.pt = sq2closest;
			prev2.u = param2.x;
			prev2.v = param2.y;

			return;
		}

		if (MathUtils::abs(prev1.u) > 7.0f || MathUtils::abs(prev1.v) > 7.0f || MathUtils::abs(prev2.u) > 7.0f || MathUtils::abs(prev2.v) > 7.0f) {
			return ClosestPointFramework(sq1, sq2, sq1closest, sq2closest, prev1, prev2);
		}
		glm::vec2 closestParams1(prev1.u, prev1.v);
		glm::vec2 closestParams2(prev2.u, prev2.v);

		glm::vec2 closestParams1Try(prev1.u, prev1.v);
		glm::vec2 closestParams2Try(prev2.u, prev2.v);

		glm::vec2 closestParamsTry1(prev1.u + MathUtils::PI_OVER_4, prev1.v);
		glm::vec2 closestParamsTry2(prev1.u, prev1.v + MathUtils::PI_OVER_4);
		glm::vec2 closestParamsTry3(prev1.u - MathUtils::PI_OVER_4, prev1.v);
		glm::vec2 closestParamsTry4(prev1.u, prev1.v - MathUtils::PI_OVER_4);


		glm::vec2 closestParamsTry11(prev2.u + MathUtils::PI_OVER_4, prev2.v);
		glm::vec2 closestParamsTry22(prev2.u, prev2.v + MathUtils::PI_OVER_4);
		glm::vec2 closestParamsTry33(prev2.u - MathUtils::PI_OVER_4, prev2.v);
		glm::vec2 closestParamsTry44(prev2.u, prev2.v - MathUtils::PI_OVER_4);

		std::vector<glm::vec2> s1;
		std::vector<glm::vec2> s2;

		s1.push_back(closestParams1Try);
		s1.push_back(closestParamsTry1);
		s1.push_back(closestParamsTry2);
		s1.push_back(closestParamsTry3);


		s2.push_back(closestParams2Try);
		s2.push_back(closestParamsTry11);
		s2.push_back(closestParamsTry22);
		s2.push_back(closestParamsTry33);
		s2.push_back(closestParamsTry44);


		glm::vec2 closestParamsTry5(prev1.u + MathUtils::PI_OVER_2, prev1.v);
		glm::vec2 closestParamsTry6(prev1.u, prev1.v + MathUtils::PI_OVER_2);
		glm::vec2 closestParamsTry7(prev1.u - MathUtils::PI_OVER_2, prev1.v);
		glm::vec2 closestParamsTry8(prev1.u, prev1.v - MathUtils::PI_OVER_2);
		glm::vec2 closestParamsTry55(prev2.u + MathUtils::PI_OVER_2, prev2.v);
		glm::vec2 closestParamsTry66(prev2.u, prev2.v + MathUtils::PI_OVER_2);
		glm::vec2 closestParamsTry77(prev2.u - MathUtils::PI_OVER_2, prev2.v);
		glm::vec2 closestParamsTry88(prev2.u, prev2.v - MathUtils::PI_OVER_2);
		s2.push_back(closestParamsTry55);
		s2.push_back(closestParamsTry66);
		s2.push_back(closestParamsTry77);
		s2.push_back(closestParamsTry88);
		s1.push_back(closestParamsTry4);
		s1.push_back(closestParamsTry5);
		s1.push_back(closestParamsTry6);
		s1.push_back(closestParamsTry7);
		s1.push_back(closestParamsTry8);

		float closest = ShapeUtils::squaredDistance(prev1.pt, prev2.pt);

		for (unsigned int i = 0; i < s1.size(); i++) {
			glm::vec2 p1 = s1[i];
			glm::vec3 pt1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(p1.x, p1.y), sq1.translation, sq1RotMat, sq1.scaling);
			glm::vec2 p2;
			for (unsigned int j = 0; j < s2.size(); j++) {
				/*if ((sq2.e1 - 1.0f) < .001f && (sq2.e2 - 1.0f) < .001f){
				glm::vec3 sq2CentroidToPt1 = pt1 - sq2.translation;
				p2 = sq2.SurfaceParamValuesFromNormal(glm::normalize(sq2CentroidToPt1));
				}
				else {*/
				p2 = s2[j];
				//}

				glm::vec3 pt2 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(p2.x, p2.y), sq2.translation, sq2RotMat, sq2.scaling);
				float dist = ShapeUtils::squaredDistance(pt1, pt2);
				if (dist < closest) {
					closest = dist;
					closestParams1.x = prev1.u = p1.x;
					closestParams1.y = prev1.v = p1.y;
					prev1.pt = pt1;



					closestParams2.x = prev2.u = p2.x;
					closestParams2.y = prev2.v = p2.y;
					prev2.pt = pt2;
				}

			}
		}

		//if (!ClosestPoint(sq1, sq2, closestParams1, closestParams2, sq1closest, sq2closest, offset, prev1, prev2)) {
		//	ClosestPointFramework(sq1, sq2, sq1closest, sq2closest, prev1, prev2);
		//}
		ClosestPoint(sq1, sq2, closestParams1, closestParams2, sq1closest, sq2closest, offset, prev1, prev2);

		if (print) {

			ParamPoint actual1, actual2;
			offset = MathUtils::PI / 5.0f;
			float accuracy = .01f;
			float uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2;
			uStart1 = -MathUtils::PI_OVER_2;
			uEnd1 = MathUtils::PI_OVER_2;
			vStart1 = -MathUtils::PI;
			vEnd1 = MathUtils::PI;

			uStart2 = -MathUtils::PI_OVER_2;
			uEnd2 = MathUtils::PI_OVER_2;
			vStart2 = -MathUtils::PI;
			vEnd2 = MathUtils::PI;

			ClosestPointBruteForce(sq1, sq2, actual1, actual2, uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2, offset, accuracy);

			if ((checkVecs(actual1.pt, sq1closest) > .1f || checkVecs(actual2.pt, sq2closest) > .1f) && ShapeUtils::squaredDistance(actual1.pt, actual2.pt) < ShapeUtils::squaredDistance(sq1closest, sq2closest)) {

				std::cout << "Uh oh - " << checkVecs(actual1.pt, sq1closest) << "   " << checkVecs(actual2.pt, sq2closest) << std::endl;

				//sq1closest = actual1.pt;
				//sq2closest = actual2.pt;

			}
		}

	}

	bool isSphere() {
		return this->a1 == 1.0f && this->a2 == 1.0f && this->a3 == 1.0f && this->e1 == 1.0f && this->e2 == 1.0f;
	}

	bool testPoint(ParamPoint pp) {
		glm::mat4 rotMat = this->getRotationMatrix();
		glm::vec3 localPt = ShapeUtils::getLocalCoordinates(pp.pt, this->translation, rotMat, scaling);

		glm::vec3 pt, norm;
		this->evalParams(pp.u, pp.v, pt, norm);

		glm::vec3 diffVec = pt - localPt;
		if (!IsZeroVector(diffVec)) {
			std::cout << "evalParams doesn't give same point" << std::endl;
			return false;
		}

		float fVal = this->f(pt);
		if (fVal < .98f) {
			std::cout << "Not a surface point" << std::endl;
			return false;
		}

		return true;
	}

	//TODO need to incorporate scaling
	static void ClosestPointFramework(Superquadric &sq1, Superquadric &sq2, glm::vec3 &sq1closest, glm::vec3 &sq2closest, ParamPoint &p1, ParamPoint &p2) {
		const float PI = glm::pi<float>();

		if (sq1.isSphere() && sq2.isSphere()) {
			glm::vec3 sq1ToSq2 = sq2.translation - sq1.translation;
			glm::vec2 param1 = sq1.SurfaceParamValuesFromSurfacePoint(glm::normalize(sq1ToSq2));
			glm::mat4 sq1RotMat = sq1.getRotationMatrix();

			//11/20/2016:
			sq1ToSq2 = glm::vec3(glm::transpose(sq1RotMat) * glm::vec4(sq1ToSq2.x, sq1ToSq2.y, sq1ToSq2.z, 1.0f));
			
			sq1closest = ShapeUtils::getGlobalCoordinates(glm::normalize(sq1ToSq2), sq1.translation, sq1RotMat, sq1.scaling);

			glm::vec3 sq2ToSq1 = sq1.translation - sq2.translation;
			glm::vec2 param2 = sq2.SurfaceParamValuesFromSurfacePoint(glm::normalize(sq2ToSq1));
			glm::mat4 sq2RotMat = sq2.getRotationMatrix();

			//11/20/2016:
			sq2ToSq1 = glm::vec3(glm::transpose(sq2RotMat) * glm::vec4(sq2ToSq1.x, sq2ToSq1.y, sq2ToSq1.z, 1.0f));

			sq2closest = ShapeUtils::getGlobalCoordinates(glm::normalize(sq2ToSq1), sq2.translation, sq2RotMat, sq2.scaling);

			p1.pt = sq1closest;
			p1.u = param1.x;
			p1.v = param1.y;

			p2.pt = sq2closest;
			p2.u = param2.x;
			p2.v = param2.y;

			sq1.testPoint(p1);
			sq2.testPoint(p2);

			return;
		}

		ParamPoint closest1, closest2, closestP1, closestP2;

		float PI_OVER_4 = PI / 4.0f;
		ClosestPointBruteForcePoints(sq1, sq2, closestP1, closestP2);
		float uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2;
		uStart1 = closestP1.u - PI_OVER_4;
		uEnd1 = closestP1.u + PI_OVER_4;
		vStart1 = closestP1.v - PI_OVER_4;
		vEnd1 = closestP1.v + PI_OVER_4;

		uStart2 = closestP2.u - PI_OVER_4;
		uEnd2 = closestP2.u + PI_OVER_4;
		vStart2 = closestP2.v - PI_OVER_4;
		vEnd2 = closestP2.v + PI_OVER_4;

		//Discretization 
		float offset = PI / 8.0f;

		//Precision of accuracy of closest points:
		float accuracy = 0.2f;

		ClosestPointBruteForce(sq1, sq2, closest1, closest2, uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2, offset, accuracy);
		glm::vec2 closestParams1(closest1.u, closest1.v);
		glm::vec2 closestParams2(closest2.u, closest2.v);

		/*float offset = PI / 16.0f;
		ClosestPointBruteForcePoints(sq1, sq2, closestP1, closestP2);
		glm::vec2 closestParams1(closestP1.u, closestP1.v);
		glm::vec2 closestParams2(closestP2.u, closestP2.v);*/

		//Polish using Newton's method:
		ClosestPoint(sq1, sq2, closestParams1, closestParams2, sq1closest, sq2closest, offset = .25f, p1, p2);

		//If Newton's diverges, it will continue the brute force method where it previously ended with greater accuracy (ResumeBruteForce)


		if (print) {
			//Find actual closest by pure brute force for debug checking:
			ParamPoint actual1, actual2;
			offset = PI / 5.0f;
			accuracy = .01f;
			ClosestPointBruteForce(sq1, sq2, actual1, actual2, uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2, offset, accuracy);

			if ((checkVecs(actual1.pt, sq1closest) > .1f || checkVecs(actual2.pt, sq2closest) > .1f) && ShapeUtils::squaredDistance(actual1.pt, actual2.pt) < ShapeUtils::squaredDistance(sq1closest, sq2closest)) {
				std::cout << "Uh oh - " << checkVecs(actual1.pt, sq1closest) << "   " << checkVecs(actual2.pt, sq2closest) << std::endl;

				//sq1closest = actual1.pt;
				//sq2closest = actual2.pt;

			}
		}
	}

	static float checkVecs(glm::vec3 v1, glm::vec3 v2) {
		return MathUtils::abs(v1.x - v2.x) + MathUtils::abs(v1.y - v2.y) + MathUtils::abs(v1.z - v2.z);
	}

	static void ResumeBruteForce(Superquadric &sq1, Superquadric &sq2, ParamPoint &pp1, ParamPoint &pp2, glm::vec3 &sq1closest, glm::vec3 &sq2closest, float offset, float bruteForceAccuracy) {

		float uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2;
		uStart1 = pp1.u - offset;
		uEnd1 = pp1.u + offset;
		uStart2 = pp2.u - offset;
		uEnd2 = pp2.u + offset;

		vStart1 = pp1.v - offset;
		vEnd1 = pp1.v + offset;
		vStart2 = pp2.v - offset;
		vEnd2 = pp2.v + offset;

		ParamPoint closest1, closest2;
		closest1 = { pp1.u, pp1.v, sq1closest };
		closest2 = { pp2.u, pp2.v, sq2closest };

		ClosestPointBruteForce(sq1, sq2, closest1, closest2, uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2, offset, bruteForceAccuracy);

		sq1closest = closest1.pt;
		sq2closest = closest2.pt;

		pp1.u = closest1.u;
		pp1.v = closest1.v;
		pp1.pt = closest1.pt;

		pp2.u = closest2.u;
		pp2.v = closest2.v;
		pp2.pt = closest2.pt;
	}

	//Assumes sq1closest is to stay fixed 
	static void ResumeBruteForceOne(Superquadric &sq1, Superquadric &sq2, glm::vec3 &sq1closest, glm::vec3 &sq2closest, ParamPoint &pp2, float offset, float bruteForceAccuracy) {

		float uStart2, uEnd2, vStart2, vEnd2;
		uStart2 = pp2.u - offset;
		uEnd2 = pp2.u + offset;

		vStart2 = pp2.v - offset;
		vEnd2 = pp2.v + offset;

		ParamPoint closest2 = { pp2.u, pp2.v, sq2closest };
		float closestDistance = ShapeUtils::squaredDistance(sq1closest, sq2closest);

		glm::mat4 sq2RotMat = sq2.getRotationMatrix();

		while (offset > bruteForceAccuracy) {

			//Get all discretized points on sq2
			for (float u2 = uStart2; u2 < uEnd2; u2 += offset) {
				for (float v2 = vStart2; v2 < vEnd2; v2 += offset) {

					glm::vec3 pt = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(u2, v2), sq2.translation, sq2RotMat, sq2.scaling);
					float distance = ShapeUtils::squaredDistance(ShapeUtils::getGlobalCoordinates(pt, sq2.translation, sq2RotMat, sq2.scaling), sq1closest);
					if (distance < closestDistance) {
						closestDistance = distance;
						closest2.pt = pt;
						closest2.u = u2;
						closest2.v = v2;
					}
				}
			}

			//Update search domain, and refine the discretization offset:
			uStart2 = closest2.u - offset;
			uEnd2 = closest2.u + offset;
			vStart2 = closest2.v - offset;
			vEnd2 = closest2.v + offset;

			offset = offset / 2.0f;

		}

		pp2.u = closest2.u;
		pp2.v = closest2.v;
		pp2.pt = closest2.pt;

		sq2closest = closest2.pt;
	}

	static void ClosestPointBruteForce(Superquadric &sq1, Superquadric &sq2, ParamPoint &sq1closest, ParamPoint &sq2closest,
		float uStart1, float uStart2, float uEnd1, float uEnd2, float vStart1, float vStart2, float vEnd1, float vEnd2,
		float &offset, float accuracy) {

		ParamPoint closest1, closest2;
		float closestDistance = 1000000.0f;

		while (offset > accuracy) {

			BruteForceSearch(sq1, sq2, uStart1, uStart2, uEnd1, uEnd2, vStart1, vStart2, vEnd1, vEnd2,
				offset, closestDistance, closest1, closest2);

			//Update search domain, and refine the discretization offset:
			uStart1 = closest1.u - offset;
			uEnd1 = closest1.u + offset;
			uStart2 = closest2.u - offset;
			uEnd2 = closest2.u + offset;

			vStart1 = closest1.v - offset;
			vEnd1 = closest1.v + offset;
			vStart2 = closest2.v - offset;
			vEnd2 = closest2.v + offset;

			offset = offset / 2.0f;

		}

		sq1closest = closest1;
		sq2closest = closest2;
	}

	static void BruteForceTest1(Superquadric &sq1, Superquadric &sq2) {
		
		glm::vec3 sq1Tosq2Local = getLocalDirectionFrom1to2(sq1, sq2);
		glm::vec3 sq2Tosq1Local = getLocalDirectionFrom1to2(sq2, sq1);


	}

	static glm::vec3 getLocalDirectionFrom1to2(Superquadric &sq1, Superquadric &sq2) {
		glm::mat4 sq1Rot = sq1.getRotationMatrix();
		glm::mat4 sq1RotInverse = glm::transpose(sq1Rot);

		glm::vec3 sq1Tosq2 = sq2.translation - sq1.translation;
		glm::vec3 sq1Tosq2Local = glm::vec3(sq1RotInverse * glm::vec4(sq1Tosq2.x, sq1Tosq2.y, sq1Tosq2.z, 1.0f));

		return sq1Tosq2Local;
	}

	struct UVinterval {
		float u0;
		float v0;
		float uF;
		float vF;
	};

	/*
	1 = x axis
	2 = y axis
	3 = z axis
	4 = -x axis
	5 = -y axis
	6 = -z axis
	*/
	static int getOctanctForVector(glm::vec3 vec) {

		//First check special cases where vec is aligned along an axis:


		if (vec.x > 0.0f) {
			if (vec.y > 0.0f) {

			}
		}
	}

	static UVinterval getSearchIntervalForOctant(int octant) {
		//**********************************
		//Will this actually be robust?? Counterexamples?
		//**********************************


	}

	static void BruteForceSearch(Superquadric &sq1, Superquadric &sq2, float uStart1, float uStart2, float uEnd1, float uEnd2,
		float vStart1, float vStart2, float vEnd1, float vEnd2, float offset, float closestDistance,
		ParamPoint &closest1, ParamPoint &closest2) {

		//These will store all points at each discretization of each superquadric 
		std::vector<ParamPoint> points1;
		std::vector<ParamPoint> points2;

		float PI = glm::pi<float>();

		glm::mat4 sq1RotMat = sq1.getRotationMatrix();
		glm::mat4 sq2RotMat = sq2.getRotationMatrix();

		//Get all discretized points on sq1
		for (float u1 = uStart1; u1 < uEnd1; u1 += offset) {
			glm::vec3 pt = glm::vec3();
			for (float v1 = vStart1; v1 < vEnd1; v1 += offset) {
				pt = sq1.PointsFromSurfaceParams(u1, v1);
				ParamPoint pp = { u1, v1, ShapeUtils::getGlobalCoordinates(pt, sq1.translation, sq1RotMat, sq1.scaling) };
				points1.push_back(pp);
			}
		}

		//Get all discretized points on sq2
		for (float u2 = uStart2; u2 < uEnd2; u2 += offset) {
			glm::vec3 pt = glm::vec3();
			for (float v2 = vStart2; v2 < vEnd2; v2 += offset) {
				pt = sq2.PointsFromSurfaceParams(u2, v2);
				ParamPoint pp = { u2, v2, ShapeUtils::getGlobalCoordinates(pt, sq2.translation, sq2RotMat, sq1.scaling) };
				points2.push_back(pp);
			}
		}

		//Find closest pair of points
		for (std::vector<ParamPoint>::iterator it1 = points1.begin(); it1 != points1.end(); it1++) {
			for (std::vector<ParamPoint>::iterator it2 = points2.begin(); it2 != points2.end(); it2++) {

				//float dist = glm::distance(it1->pt, it2->pt);

				//Use squared distance to avoid extra computation:
				float dist = ShapeUtils::squaredDistance(it1->pt, it2->pt);

				if (dist < closestDistance) {
					closestDistance = dist;
					closest1 = { it1->u, it1->v, it1->pt };
					closest2 = { it2->u, it2->v, it2->pt };
				}
			}
		}
	}

	//Uses the stored superquad Points objects
	static void ClosestPointBruteForcePoints(Superquadric &sq1, Superquadric &sq2, ParamPoint &sq1closest, ParamPoint &sq2closest) {

		std::vector<ParamPoint> sq1pts = sq1.points;
		std::vector<ParamPoint> sq2pts = sq2.points;

		ParamPoint closest1, closest2;
		float closestDistance = 1000000.0f;

		glm::mat4 sq1Rot = sq1.getRotationMatrix();
		glm::mat4 sq2Rot = sq2.getRotationMatrix();

		for (std::vector<ParamPoint>::iterator it1 = sq1pts.begin(); it1 != sq1pts.end(); it1++) {
			ParamPoint sq1pp = *it1;
			for (std::vector<ParamPoint>::iterator it2 = sq2pts.begin(); it2 != sq2pts.end(); it2++) {
				ParamPoint sq2pp = *it2;
				float distance = ShapeUtils::squaredDistance(ShapeUtils::getGlobalCoordinates(sq1pp.pt, sq1.translation, sq1Rot, sq1.scaling),
					ShapeUtils::getGlobalCoordinates(sq2pp.pt, sq2.translation, sq2Rot, sq2.scaling));
				if (distance < closestDistance) {
					closest1 = *it1;
					closest2 = *it2;
					closestDistance = distance;
				}
			}
		}

		sq1closest = { closest1.u, closest1.v, ShapeUtils::getGlobalCoordinates(closest1.pt, sq1.translation, sq1Rot, sq1.scaling) };
		sq2closest = { closest2.u, closest2.v, ShapeUtils::getGlobalCoordinates(closest2.pt, sq2.translation, sq2Rot, sq2.scaling) };
	}

	static bool testIntersection(Superquadric &sq1, Superquadric &sq2, glm::vec3 &sq1pt, glm::vec3 &sq2pt) {

		//glm::vec3 pt = applyRotation(sq1pt, glm::transpose(sq1.getRotationMatrix()));
		glm::mat4 sq2Rot = sq2.getRotationMatrix();
		glm::vec3 pt1InSq2ModelSpace = ShapeUtils::getLocalCoordinates(sq1pt, sq2.translation, sq2Rot, sq2.scaling);

		if (sq2.f(pt1InSq2ModelSpace) < 1.0f) {
			float f = sq2.f(pt1InSq2ModelSpace);
			if (print)
				std::cout << "Intersection found" << std::endl;
			return true;
		}
		return false;
	}

	static bool tryClosestPoint(Superquadric &sq1, Superquadric &sq2, glm::vec2 p1, glm::vec2 p2,
		glm::vec3 &sq1closest, glm::vec3 &sq2closest, float offset, ParamPoint &pp1, ParamPoint &pp2) {

		// sigma(u, v) = sq1,
		// gamma(s, t) = sq2

		//Then get the points in parametrization coordinates:
		float xi[4];

		xi[0] = p1.x; // params.x;
		xi[1] = p1.y; // params.y;

					  //params = sq2.SurfaceParamValuesFromNormal(glm::normalize(localp2));
		xi[2] = p2.x;
		xi[3] = p2.y;

		float yi[4];
		float deltaX[4];

		// yi is evaluation of each element of xi at 4 corresponding functions
		//	 f1 is SigmaU DOT (p2-p1)
		//   f2 is SigmaV DOT (p2-p1)
		//   f3 is GammaU DOT (p2-p1)
		//   f4 is GammaV DOT (p2-p1)

		float closestDistance = 10000.0f;
		glm::vec2 closest1(xi[0], xi[1]);
		glm::vec2 closest2(xi[2], xi[3]);

		evalSystem(sq1, sq2, xi, yi, closestDistance);

		//To determine divergence:
		int maxIterations = 8;
		int curIterations = 0;

		glm::vec3 p, q, pMinusQ;

		glm::vec3 prevGradient1;// = sq1.unitnormal(p);
		glm::vec3 prevGradient2;// = sq2.unitnormal(q);
		glm::vec2 prevDx1;
		glm::vec2 prevDx2;
		int gradient1NAN = 0;
		int gradient2NAN = 0;

		glm::mat4 sq1Rot = sq1.getRotationMatrix();
		glm::mat4 sq2Rot = sq2.getRotationMatrix();

		while (!IsZeroVector(yi)) {

			curIterations++;

			if (curIterations >= maxIterations) {
				//Probably diverging if it's taken this many iterations
				// use brute force to refine closest points found yet
				// points

				/*float bruteForceAccuracy = .01f;
				pp1.u = closest1.x;
				pp1.v = closest1.y;

				pp2.u = closest1.x;
				pp2.v = closest2.y;*/

				//ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);  //pp1, pp2

				static int divergences = 0;
				divergences++;
				//if (print)
				//	std::cout << "Diverged (try)" << divergences << std::endl;

				return false;
			}

			//Construct Jacobian:
			float Ji[4][4];
			glm::vec3 sigu, sigv, siguu, siguv, sigvv;
			sigu = sq1.dSigmaDu(xi[0], xi[1]);
			int i = -1;
			bool doubled = false;
			while (checkVecForNaN(sigu)) {
				i++;
				if (i > 7) {
					if (doubled)
						break;
					else {
						sq1.doubleNanAdjustsments();
						i = -1;
						doubled = true;
						continue;
					}
				}
				sigu = sq1.dSigmaDu(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq1.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[0] += sq1.nanAdjustments[i].x;
				xi[1] += sq1.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			i = -1;
			sigv = sq1.dSigmaDv(xi[0], xi[1]);
			while (checkVecForNaN(sigv)) {
				i++;
				if (i > 7)
					break;
				sigv = sq1.dSigmaDv(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);

			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq1.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[0] += sq1.nanAdjustments[i].x;
				xi[1] += sq1.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			i = -1;
			siguu = sq1.ddSigmaDuDu(xi[0], xi[1]);
			while (checkVecForNaN(siguu)) {
				i++;
				if (i > 7)
					break;
				siguu = sq1.ddSigmaDuDu(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);

			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq1.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[0] += sq1.nanAdjustments[i].x;
				xi[1] += sq1.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			i = -1;
			siguv = sq1.ddSigmaDuDv(xi[0], xi[1]);
			while (checkVecForNaN(siguv)) {
				i++;
				if (i > 7)
					break;
				siguv = sq1.ddSigmaDuDv(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq1.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[0] += sq1.nanAdjustments[i].x;
				xi[1] += sq1.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			sigvv = sq1.ddSigmaDvDv(xi[0], xi[1]);
			i = -1;
			while (checkVecForNaN(sigvv)) {
				i++;
				if (i > 7)
					break;
				sigvv = sq1.ddSigmaDvDv(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq1.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[0] += sq1.nanAdjustments[i].x;
				xi[1] += sq1.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			glm::vec3 gams, gamt, gamss, gamst, gamtt;
			gams = sq2.dSigmaDu(xi[2], xi[3]);
			i = -1;
			while (checkVecForNaN(gams)) {
				i++;
				if (i > 7)
					break;
				gams = sq2.dSigmaDu(xi[2] + sq2.nanAdjustments[i].x, xi[3] + sq2.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq2.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[2] += sq2.nanAdjustments[i].x;
				xi[3] += sq2.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			gamt = sq2.dSigmaDv(xi[2], xi[3]);
			i = -1;
			while (checkVecForNaN(gamt)) {
				i++;
				if (i > 7)
					break;
				gamt = sq2.dSigmaDv(xi[2] + sq2.nanAdjustments[i].x, xi[3] + sq2.nanAdjustments[i].y);

			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq2.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[2] += sq2.nanAdjustments[i].x;
				xi[3] += sq2.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			gamss = sq2.ddSigmaDuDu(xi[2], xi[3]);
			i = -1;
			while (checkVecForNaN(gamss)) {
				i++;
				if (i > 7)
					break;
				gamss = sq2.ddSigmaDuDu(xi[2] + sq2.nanAdjustments[i].x, xi[3] + sq2.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq2.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[2] += sq2.nanAdjustments[i].x;
				xi[3] += sq2.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			gamst = sq2.ddSigmaDuDv(xi[2], xi[3]);
			i = -1;
			while (checkVecForNaN(gamst)) {
				i++;
				if (i > 7)
					break;
				gamst = sq2.ddSigmaDuDv(xi[2] + sq2.nanAdjustments[i].x, xi[3] + sq2.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq2.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[2] += sq2.nanAdjustments[i].x;
				xi[3] += sq2.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			gamtt = sq2.ddSigmaDvDv(xi[2], xi[3]);
			i = -1;
			while (checkVecForNaN(gamtt)) {
				i++;
				if (i > 7)
					break;
				gamtt = sq2.ddSigmaDvDv(xi[2] + sq2.nanAdjustments[i].x, xi[3] + sq2.nanAdjustments[i].y);
			}
			//Still not a number, stop newtons and continue brute force method
			if (i > 7) {
				curIterations = maxIterations;
				sq2.doubleNanAdjustsments();
				continue;
			}
			else if (i > -1) {
				xi[2] += sq2.nanAdjustments[i].x;
				xi[3] += sq2.nanAdjustments[i].y;
				//Want other derivatives to be evaluated at the same (u,v) point so iterate again at this point
				// that didn't produce NaN values
				continue;
			}

			// apply just rotation? TODO
			sigu = applyRotation(sigu, sq1Rot);
			sigv = applyRotation(sigv, sq1Rot);
			siguu = applyRotation(siguu, sq1Rot);
			siguv = applyRotation(siguv, sq1Rot);
			sigvv = applyRotation(sigvv, sq1Rot);

			gams = applyRotation(gams, sq2Rot);
			gamt = applyRotation(gamt, sq2Rot);
			gamss = applyRotation(gamss, sq2Rot);
			gamst = applyRotation(gamst, sq2Rot);
			gamtt = applyRotation(gamtt, sq2Rot);

			glm::vec3 localp = sq1.PointsFromSurfaceParams(xi[0], xi[1]);
			glm::vec3 localq = sq2.PointsFromSurfaceParams(xi[2], xi[3]);
			p = ShapeUtils::getGlobalCoordinates(localp, sq1.translation, sq1Rot, sq1.scaling);
			q = ShapeUtils::getGlobalCoordinates(localq, sq2.translation, sq2Rot, sq2.scaling);

			glm::vec3 gradient1 = sq1.unitnormal(localp);
			glm::vec3 gradient2 = sq2.unitnormal(localq);



			if (curIterations > 1) {
				if (checkVecForNaN(gradient1)) {
					gradient1NAN++;

					/*if (gradient1NAN > 2) {
					pp2.u = xi[2];
					pp2.v = xi[3];
					pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());

					pp1.u = xi[0];
					pp1.v = xi[1];
					pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());
					ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);

					return false;
					}*/
					i = -1;
					while (checkVecForNaN(gradient1)) {
						i++;
						if (i > 7) {
							curIterations = maxIterations;
							break;
						}
						localp = sq2.PointsFromSurfaceParams(xi[0] + sq2.nanAdjustments[i].x, xi[1] + sq2.nanAdjustments[i].y);
						gradient1 = sq1.unitnormal(localp);
					}
					continue;

					//if (gradient1NAN > 2) {
					//ResumeBruteForceOne(sq1, sq2, p, q, pp2, offset, .01f);
					//ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					//std::cout << "Resumed" << std::endl;

					//pp1.u = xi[0];
					//pp1.v = xi[1];
					//pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());

					//return false;
					//}
				}
				if (checkVecForNaN(gradient2)) {
					gradient2NAN++;
					if (gradient2NAN > 2) {
						//ResumeBruteForceOne(sq2, sq1, q, p, pp1, offset, .01f);
						//ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
						//std::cout << "Resumed" << std::endl;

						//pp2.u = xi[2];
						//pp2.v = xi[3];
						//pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());

						return false;
					}
				}
				if (glm::dot(gradient1, prevGradient1) < 0.0f) {
					i = 100;
					//ResumeBruteForceOne(sq1, sq2, p, q, pp2, offset, .01f);
					/*ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					sq1closest = p;
					sq2closest = q;
					std::cout << "Resumed" << std::endl;

					pp1.u = xi[0];
					pp1.v = xi[1];
					pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());*/


					return false;
				}
				if (glm::dot(gradient2, prevGradient2) < 0.0f) {
					//ResumeBruteForceOne(sq2, sq1, q, p, pp1, offset, .01f);
					/*ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					sq1closest = p;
					sq2closest = q;

					pp2.u = xi[2];
					pp2.v = xi[3];
					pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());

					std::cout << "Resumed" << std::endl;*/
					return false;
				}
			}

			prevGradient1 = gradient1;
			prevGradient2 = gradient2;

			pMinusQ = (p - q);

			//TODO 0,1 = 1,0 and 2,3 = 3,2, just reuse value instead of computing again
			Ji[0][0] = glm::dot(siguu, pMinusQ) + glm::dot(sigu, sigu);
			Ji[0][1] = glm::dot(siguv, pMinusQ) + glm::dot(sigu, sigv);
			Ji[0][2] = glm::dot(sigu, -gams);
			Ji[0][3] = glm::dot(sigu, -gamt);

			Ji[1][0] = glm::dot(siguv, pMinusQ) + glm::dot(sigv, sigu);
			Ji[1][1] = glm::dot(sigvv, pMinusQ) + glm::dot(sigv, sigv);
			Ji[1][2] = glm::dot(sigv, -gams);
			Ji[1][3] = glm::dot(sigv, -gamt);

			Ji[2][0] = glm::dot(gams, sigu);
			Ji[2][1] = glm::dot(gams, sigv);
			Ji[2][2] = glm::dot(gamss, pMinusQ) + glm::dot(gams, -gams);
			Ji[2][3] = glm::dot(gamst, pMinusQ) + glm::dot(gams, -gamt);

			Ji[3][0] = glm::dot(gamt, sigu);
			Ji[3][1] = glm::dot(gamt, sigv);
			Ji[3][2] = glm::dot(gamst, pMinusQ) + glm::dot(gamt, -gams);
			Ji[3][3] = glm::dot(gamtt, pMinusQ) + glm::dot(gamt, -gamt);

			// Need to solve J * deltax = -y to get deltax, so negate yi:
			for (int i = 0; i < 4; i++) {
				yi[i] *= -1.0f;
			}

			solve(Ji, yi, deltaX); //0,0 and 2,2 are nan

								   //update xi by deltax:
			for (int i = 0; i < 4; i++) {
				xi[i] += deltaX[i];
			}

			float curDist = 0.0f;
			//check if solution is found:
			evalSystem(sq1, sq2, xi, yi, curDist);

			glm::vec2 dx1(deltaX[0], deltaX[1]);
			glm::vec2 dx2(deltaX[2], deltaX[3]);

			/*if (curDist < closestDistance) {
			closest1.x = xi[0];
			closest1.y = xi[1];
			closest2.x = xi[2];
			closest2.y = xi[3];


			}*/

			/*if (curIterations > 1) {
			if (smallVecDiff(dx1, prevDx1, .001f) && smallVecDiff(dx2, prevDx2, .001f)) {
			std::cout << "diff close enough" << std::endl;
			break;
			}
			}*/

			prevDx1 = dx1;
			prevDx2 = dx2;


		}

		//Return the 3d points in their world space coordinates

		sq1closest = sq1.PointsFromSurfaceParams(xi[0], xi[1]);
		sq1closest = ShapeUtils::getGlobalCoordinates(sq1closest, sq1.translation, sq1Rot, sq1.scaling);

		sq2closest = sq2.PointsFromSurfaceParams(xi[2], xi[3]);
		sq2closest = ShapeUtils::getGlobalCoordinates(sq2closest, sq2.translation, sq2Rot, sq2.scaling);

		pp1.u = xi[0];
		pp1.v = xi[1];
		pp2.u = xi[2];
		pp2.v = xi[3];
		pp1.pt = sq1closest;
		pp2.pt = sq2closest;
		//if (print)
		//	std::cout << "Iterations: " << curIterations << std::endl;
		return true;
	}

	//Offset is the current offest that was used before calling this method from the 
	// brute force method. If this method diverges, stop iterating and return to brute force 
	// at that offset
	//p1 and p2 are the (u,v) parameters for the closest point estimates
	static bool ClosestPoint(Superquadric &sq1, Superquadric &sq2, glm::vec2 p1, glm::vec2 p2,
		glm::vec3 &sq1closest, glm::vec3 &sq2closest, float offset, ParamPoint &pp1, ParamPoint &pp2) {

		// sigma(u, v) = sq1,
		// gamma(s, t) = sq2

		//Then get the points in parametrization coordinates:
		float xi[4];

		xi[0] = p1.x; // params.x;
		xi[1] = p1.y; // params.y;

					  //params = sq2.SurfaceParamValuesFromNormal(glm::normalize(localp2));
		xi[2] = p2.x;
		xi[3] = p2.y;

		float yi[4];
		float deltaX[4];

		// yi is evaluation of each element of xi at 4 corresponding functions
		//	 f1 is SigmaU DOT (p2-p1)
		//   f2 is SigmaV DOT (p2-p1)
		//   f3 is GammaU DOT (p2-p1)
		//   f4 is GammaV DOT (p2-p1)

		float closestDistance = 10000.0f;
		glm::vec2 closest1(xi[0], xi[1]);
		glm::vec2 closest2(xi[2], xi[3]);

		evalSystem(sq1, sq2, xi, yi, closestDistance);

		//To determine divergence:
		int maxIterations = 9;
		int curIterations = 0;

		glm::vec3 p, q, pMinusQ;

		glm::vec3 prevGradient1;// = sq1.unitnormal(p);
		glm::vec3 prevGradient2;// = sq2.unitnormal(q);
		glm::vec2 prevDx1;
		glm::vec2 prevDx2;
		int gradient1NAN = 0;
		int gradient2NAN = 0;

		float prevY[4];

		bool sq1uLeft = false;
		bool sq1uRight = false;
		bool sq1vLeft1 = false;
		bool sq1vRight1 = false;
		bool sq1vLeft2 = false;
		bool sq1vRight2 = false;

		bool sq1u4Left = false;
		bool sq1u4Right = false;
		bool sq1v4Left = false;
		bool sq1v4Right = false;

		bool sq2uLeft = false;
		bool sq2uRight = false;
		bool sq2vLeft1 = false;
		bool sq2vRight1 = false;
		bool sq2vLeft2 = false;
		bool sq2vRight2 = false;

		bool sq2u4Left = false;
		bool sq2u4Right = false;
		bool sq2v4Left = false;
		bool sq2v4Right = false;

		float bruteForceAccuracy = .02f;

		glm::mat4 sq1Rot = sq1.getRotationMatrix();
		glm::mat4 sq2Rot = sq2.getRotationMatrix();

		while (!IsZeroVector(yi)) {

			prevY[0] = yi[0];
			prevY[1] = yi[1];
			prevY[2] = yi[2];
			prevY[3] = yi[3];

			curIterations++;

			if (curIterations >= maxIterations) {
				//Probably diverging if it's taken this many iterations
				// use brute force to refine closest points found yet
				// points

				pp1.u = closest1.x;
				pp1.v = closest1.y;

				pp2.u = closest2.x;
				pp2.v = closest2.y;

				ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset * 2.0f, bruteForceAccuracy);  //pp1, pp2

				static int divergences = 0;
				divergences++;
				if (print)
					std::cout << "Diverged, started at: " << closest1.x << " , " << closest1.y << "  -  " << closest2.x << " , " << closest2.y << divergences << std::endl;

				return false;
			}

			//Construct Jacobian:
			float Ji[4][4];
			glm::vec3 sigu, sigv, siguu, siguv, sigvv;
			sigu = sq1.dSigmaDu(xi[0], xi[1]);

			sigv = sq1.dSigmaDv(xi[0], xi[1]);

			siguu = sq1.ddSigmaDuDu(xi[0], xi[1]);

			siguv = sq1.ddSigmaDuDv(xi[0], xi[1]);

			sigvv = sq1.ddSigmaDvDv(xi[0], xi[1]);

			if (checkAllVecs(sigu, sigv, siguu, siguv, sigvv)) {
				if (MathUtils::abs(xi[0]) < .05f) {
					// u is approximately 0 
					if (!sq1uLeft) {
						//Try adjusting to the left side of the u=0 line:
						xi[0] -= sq1.adjustment;
						sq1uLeft = true;
						continue;
					}
					else if (!sq1uRight) {
						//Try adjusting to the left side of the u=0 line:
						xi[0] += sq1.adjustment;
						sq1uRight = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line u=0
						// by adjusting v:

						//float adjust = .05f;

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1] + sq1.adjustment), sq1.translation, sq1Rot, sq1.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1] - sq1.adjustment), sq1.translation, sq1Rot, sq1.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p1;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float update = .05f; // sq1.adjustment * 3;
							if (result == 3) {
								//negative direction is the closer direction:
								update = -update;
								closest = p3;
							}

							searchOneDirection(sq1, closest, p, &xi[0], &xi[1], &xi[1], update);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[1]) < .05f || MathUtils::abs(xi[1] - MathUtils::PI) < .05f) {
					//v is approximately 0 or pi
					if (!sq1vLeft1) {
						//Try adjusting to the left side of the line v1= 0 || PI:
						xi[1] -= sq1.adjustment;
						sq1vLeft1 = true;
						continue;
					}
					else if (!sq1vRight1) {
						//Try adjusting to the left side of the line v1= 0 || PI:
						xi[1] += sq1.adjustment;
						sq1vRight1 = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line v1= 0 || PI
						// by adjusting u:

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] + sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] - sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p1;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							ResumeBruteForceOne(sq1, sq2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), pp2, offset, bruteForceAccuracy);

							if (print)
								std::cout << "ResumeOne ing on V" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float offset = sq1.adjustment;
							if (result == 3) {
								//negative direction is the closer direction:
								offset = -offset;
								closest = p3;
							}
							searchOneDirection(sq1, closest, p, &xi[0], &xi[1], &xi[0], offset);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];

							ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on v" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[1] - MathUtils::PI_OVER_2) < .05f || MathUtils::abs(xi[1] + MathUtils::PI_OVER_2) < .05f) {
					//v is approximately pi/2 or -pi/2
					if (!sq1vLeft2) {
						//Try adjusting to the left side of the line v1 = +/- PI/2:
						xi[1] -= sq1.adjustment;
						sq1vLeft2 = true;
						continue;
					}
					else if (!sq1vRight2) {
						//Try adjusting to the left side of the line v1 = +/- PI/2:
						xi[1] += sq1.adjustment;
						sq1vRight2 = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line v1 = +/- PI/2:
						// by adjusting u:

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] + sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] - sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p1;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							ResumeBruteForceOne(sq1, sq2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), pp2, offset, bruteForceAccuracy);

							if (print)
								std::cout << "ResumeOne ing on V" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float offset = sq1.adjustment;
							if (result == 3) {
								//negative direction is the closer direction:
								offset = -offset;
								closest = p3;
							}
							searchOneDirection(sq1, closest, p, &xi[0], &xi[1], &xi[0], offset);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];

							ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on v" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[1] - MathUtils::PI_OVER_4) < .05f || MathUtils::abs(xi[1] + MathUtils::PI_OVER_4) < .05f) {
					// v is approximately +/- PI/4 
					if (!sq1v4Left) {
						//Try adjusting to the left side of the line:
						xi[1] -= sq1.adjustment;
						sq1v4Left = true;
						continue;
					}
					else if (!sq1v4Right) {
						//Try adjusting to the left side of the line:
						xi[1] += sq1.adjustment;
						sq1v4Right = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line u=0
						// by adjusting v:

						//float adjust = .05f;

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] + sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0] - sq1.adjustment, xi[1]), sq1.translation, sq1Rot, sq1.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p1;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);

							if (print)
								std::cout << "Resuming on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float update = .05f; // sq1.adjustment * 3;
							if (result == 3) {
								//negative direction is the closer direction:
								update = -update;
								closest = p3;
							}

							searchOneDirection(sq1, closest, p, &xi[0], &xi[1], &xi[0], update);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[0] - MathUtils::PI_OVER_4) < .05f || MathUtils::abs(xi[0] + MathUtils::PI_OVER_4) < .05f) {

					// u is approximately +/- PI/4 

					if (!sq1u4Left) {
						//Try adjusting to the left side of the line:
						xi[0] -= sq1.adjustment;
						sq1u4Left = true;
						continue;
					}
					else if (!sq1u4Right) {
						//Try adjusting to the left side of the line:
						xi[0] += sq1.adjustment;
						sq1u4Right = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line u=0
						// by adjusting v:

						//float adjust = .05f;

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1Rot, sq1.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1] + sq1.adjustment), sq1.translation, sq1Rot, sq1.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1] - sq1.adjustment), sq1.translation, sq1Rot, sq1.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p1;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);

							if (print)
								std::cout << "Resuming on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float update = .05f; // sq1.adjustment * 3;
							if (result == 3) {
								//negative direction is the closer direction:
								update = -update;
								closest = p3;
							}

							searchOneDirection(sq1, closest, p, &xi[0], &xi[1], &xi[1], update);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];
							sq1closest = pp1.pt;
							sq2closest = pp2.pt;
							ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else {
					if (print)
						std::cout << "Unknown  problem..." << std::endl;
					pp1.u = xi[0];
					pp1.v = xi[1];
					pp1.pt = p;
					pp2.pt = q;
					pp2.u = xi[2];
					pp2.v = xi[3];
					sq1closest = pp1.pt;
					sq2closest = pp2.pt;
					ResumeBruteForce(sq1, sq2, pp1, pp2, sq1closest, sq2closest, offset, bruteForceAccuracy);
					return false;
				}
			}


			glm::vec3 gams, gamt, gamss, gamst, gamtt;
			gams = sq2.dSigmaDu(xi[2], xi[3]);

			gamt = sq2.dSigmaDv(xi[2], xi[3]);

			gamss = sq2.ddSigmaDuDu(xi[2], xi[3]);

			gamst = sq2.ddSigmaDuDv(xi[2], xi[3]);

			gamtt = sq2.ddSigmaDvDv(xi[2], xi[3]);

			if (checkAllVecs(gams, gamt, gamss, gamst, gamtt)) {
				if (MathUtils::abs(xi[2]) < .05f) {
					// u is approximately 0 
					if (!sq2uLeft) {
						//Try adjusting to the left side of the u=0 line:
						xi[2] -= sq2.adjustment;
						sq2uLeft = true;
						continue;
					}
					else if (!sq2uRight) {
						//Try adjusting to the left side of the u=0 line:
						xi[2] += sq2.adjustment;
						sq2uRight = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line u=0
						// by adjusting v:

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3] + sq2.adjustment), sq2.translation, sq2Rot, sq2.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3] - sq2.adjustment), sq2.translation, sq2Rot, sq2.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[1], xi[2]), sq1.translation, sq1Rot, sq1.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p;
							pp2.pt = p1;
							pp2.u = xi[2];
							pp2.v = xi[3];
							ResumeBruteForceOne(sq2, sq1, (sq2closest = pp2.pt), (sq1closest = pp1.pt), pp1, offset, bruteForceAccuracy);

							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float offset = sq2.adjustment;
							if (result == 3) {
								//negative direction is the closer direction:
								offset = -offset;
								closest = p3;
							}

							searchOneDirection(sq2, closest, p, &xi[2], &xi[3], &xi[3], offset);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];

							ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[3]) < .05f || MathUtils::abs(xi[3] - MathUtils::PI) < .05f) {
					//v is approximately 0 or pi
					if (!sq1vLeft1) {
						//Try adjusting to the left side of the line v1= 0 || PI:
						xi[3] -= sq2.adjustment;
						sq2vLeft1 = true;
						continue;
					}
					else if (!sq1vRight1) {
						//Try adjusting to the left side of the line v1= 0 || PI:
						xi[3] += sq2.adjustment;
						sq2vRight1 = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line v1= 0 || PI
						// by adjusting u:

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] + sq2.adjustment, xi[3]), sq2.translation, sq2Rot, sq2.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] - sq2.adjustment, xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[1], xi[2]), sq1.translation, sq1Rot, sq1.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p;
							pp2.pt = p1;
							pp2.u = xi[2];
							pp2.v = xi[3];
							ResumeBruteForceOne(sq2, sq1, (sq2closest = pp2.pt), (sq1closest = pp1.pt), pp1, offset, bruteForceAccuracy);

							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float offset = sq2.adjustment;
							if (result == 3) {
								//negative direction is the closer direction:
								offset = -offset;
								closest = p3;
							}

							searchOneDirection(sq2, closest, p, &xi[2], &xi[3], &xi[2], offset);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];

							ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else if (MathUtils::abs(xi[3] - MathUtils::PI_OVER_2) < .05f || MathUtils::abs(xi[3] + MathUtils::PI_OVER_2) < .05f) {
					//v is approximately pi/2 or -pi/2
					if (!sq2vLeft2) {
						//Try adjusting to the left side of the line v1 = +/- PI/2:
						xi[3] -= sq2.adjustment;
						sq2vLeft2 = true;
						continue;
					}
					else if (!sq2vRight2) {
						//Try adjusting to the left side of the line v1 = +/- PI/2:
						xi[3] += sq2.adjustment;
						sq2vRight2 = true;
						continue;
					}
					else {
						//Already tried left and right, try going up and down on the line v1 = +/- PI/2:
						// by adjusting u:

						//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
						glm::vec3 p1, p2, p3;
						p1 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2Rot, sq2.scaling);
						p2 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] + sq2.adjustment, xi[3]), sq2.translation, sq2Rot, sq2.scaling);
						p3 = ShapeUtils::getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] - sq2.adjustment, xi[3]), sq2.translation, sq2Rot, sq2.scaling);

						//Current point on other surface to use for evaluating closeness
						p = ShapeUtils::getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[1], xi[2]), sq1.translation, sq1Rot, sq1.scaling);

						int result = findClosest(p1, p2, p3, p);

						if (result == 1) {
							//Current point is closer than adjusting u each direction
							// Good chance this is the actual closest point
							// ...if p is close to sq2's actual closest point
							// ......so hope it is and resume one:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = p;
							pp2.pt = p1;
							pp2.u = xi[2];
							pp2.v = xi[3];
							ResumeBruteForceOne(sq2, sq1, (sq2closest = pp2.pt), (sq1closest = pp1.pt), pp1, offset, bruteForceAccuracy);

							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
						else {
							glm::vec3 prevClosest, closest;
							closest = p2;
							float offset = sq2.adjustment;
							if (result == 3) {
								//negative direction is the closer direction:
								offset = -offset;
								closest = p3;
							}

							searchOneDirection(sq2, closest, p, &xi[2], &xi[3], &xi[2], offset);

							//Now Resume Brute force search:
							pp1.u = xi[0];
							pp1.v = xi[1];
							pp1.pt = closest;
							pp2.pt = p;
							pp2.u = xi[2];
							pp2.v = xi[3];

							ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), offset, bruteForceAccuracy);
							if (print)
								std::cout << "ResumeOne ing on u" << std::endl;
							return false;
						}
					}
				}
				else {
					if (print)
						std::cout << "Unknown  problem..." << std::endl;
				}
			}

			// apply just rotation
			sigu = applyRotation(sigu, sq1Rot);
			sigv = applyRotation(sigv, sq1Rot);
			siguu = applyRotation(siguu, sq1Rot);
			siguv = applyRotation(siguv, sq1Rot);
			sigvv = applyRotation(sigvv, sq1Rot);

			gams = applyRotation(gams, sq2Rot);
			gamt = applyRotation(gamt, sq2Rot);
			gamss = applyRotation(gamss, sq2Rot);
			gamst = applyRotation(gamst, sq2Rot);
			gamtt = applyRotation(gamtt, sq2Rot);

			glm::vec3 localp = sq1.PointsFromSurfaceParams(xi[0], xi[1]);
			glm::vec3 localq = sq2.PointsFromSurfaceParams(xi[2], xi[3]);
			p = ShapeUtils::getGlobalCoordinates(localp, sq1.translation, sq1Rot, sq1.scaling);
			q = ShapeUtils::getGlobalCoordinates(localq, sq2.translation, sq2Rot, sq2.scaling);

			glm::vec3 gradient1 = sq1.unitnormal(localp);
			glm::vec3 gradient2 = sq2.unitnormal(localq);

			if (curIterations > 1) {
				if (checkVecForNaN(gradient1)) {
					gradient1NAN++;

					/*if (gradient1NAN > 2) {
					pp2.u = xi[2];
					pp2.v = xi[3];
					pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());

					pp1.u = xi[0];
					pp1.v = xi[1];
					pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());
					ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);

					return false;
					}*/
					int i = -1;
					bool doubley = false;
					while (checkVecForNaN(gradient1)) {
						i++;
						if (i > 7) {
							if (doubley) {
								curIterations = maxIterations;
								break;
							}
							else {
								i = -1;
								sq1.doubleNanAdjustsments();
								doubley = true;
								continue;
							}
						}
						localp = sq1.PointsFromSurfaceParams(xi[0] + sq1.nanAdjustments[i].x, xi[1] + sq1.nanAdjustments[i].y);
						gradient1 = sq1.unitnormal(localp);
					}
					continue;

					//if (gradient1NAN > 2) {
					//ResumeBruteForceOne(sq1, sq2, p, q, pp2, offset, .01f);
					//ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					//std::cout << "Resumed" << std::endl;

					//pp1.u = xi[0];
					//pp1.v = xi[1];
					//pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());

					//return false;
					//}
				}
				if (checkVecForNaN(gradient2)) {
					gradient2NAN++;
					if (gradient2NAN > 2) {
						//ResumeBruteForceOne(sq2, sq1, q, p, pp1, offset, .01f);

						pp1.u = closest1.x; // xi[0];
						pp1.v = closest1.y; // xi[1];
						pp2.u = closest2.x; // xi[2];
						pp2.v = closest2.y; // xi[3];
						ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
						//std::cout << "Resumed" << std::endl;

						//pp2.u = xi[2];
						//pp2.v = xi[3];
						//pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());

						return false;
					}
				}
				if (glm::dot(gradient1, prevGradient1) < 0.0f) {
					//ResumeBruteForceOne(sq1, sq2, p, q, pp2, offset, .01f);
					pp1.u = closest1.x; // xi[0];
					pp1.v = closest1.y; // xi[1];
					pp2.u = closest2.x; // xi[2];
					pp2.v = closest2.y; // xi[3];
					ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					sq1closest = p;
					sq2closest = q;
					if (print)
						std::cout << "Resumed at " << pp1.u << ", " << pp1.v << "   -    " << pp2.u << " , " << pp2.v << std::endl;

					/*pp1.u = xi[0];
					pp1.v = xi[1];
					pp1.pt = getGlobalCoordinates(sq1.PointsFromSurfaceParams(pp1.u, pp1.v), sq1.translation, sq1.getRotationMatrix());*/


					return false;
				}
				if (glm::dot(gradient2, prevGradient2) < 0.0f) {
					//ResumeBruteForceOne(sq2, sq1, q, p, pp1, offset, .01f);

					pp1.u = closest1.x; // xi[0];
					pp1.v = closest1.y; // xi[1];
					pp2.u = closest2.x; // xi[2];
					pp2.v = closest2.y; // xi[3];
					ResumeBruteForce(sq1, sq2, pp1, pp2, p, q, offset * 2.0f, .01f);
					sq1closest = p;
					sq2closest = q;

					/*pp2.u = xi[2];
					pp2.v = xi[3];
					pp2.pt = getGlobalCoordinates(sq2.PointsFromSurfaceParams(pp2.u, pp2.v), sq2.translation, sq2.getRotationMatrix());*/

					if (print)
						std::cout << "Resumed at " << pp1.u << ", " << pp1.v << "   -    " << pp2.u << " , " << pp2.v << std::endl;
					return false;
				}
			}

			prevGradient1 = gradient1;
			prevGradient2 = gradient2;

			pMinusQ = (p - q);

			//TODO 0,1 = 1,0 and 2,3 = 3,2, just reuse value instead of computing again
			Ji[0][0] = glm::dot(siguu, pMinusQ) + glm::dot(sigu, sigu);
			Ji[0][1] = glm::dot(siguv, pMinusQ) + glm::dot(sigu, sigv);
			Ji[0][2] = glm::dot(sigu, -gams);
			Ji[0][3] = glm::dot(sigu, -gamt);

			Ji[1][0] = glm::dot(siguv, pMinusQ) + glm::dot(sigv, sigu);
			Ji[1][1] = glm::dot(sigvv, pMinusQ) + glm::dot(sigv, sigv);
			Ji[1][2] = glm::dot(sigv, -gams);
			Ji[1][3] = glm::dot(sigv, -gamt);

			Ji[2][0] = glm::dot(gams, sigu);
			Ji[2][1] = glm::dot(gams, sigv);
			Ji[2][2] = glm::dot(gamss, pMinusQ) + glm::dot(gams, -gams);
			Ji[2][3] = glm::dot(gamst, pMinusQ) + glm::dot(gams, -gamt);

			Ji[3][0] = glm::dot(gamt, sigu);
			Ji[3][1] = glm::dot(gamt, sigv);
			Ji[3][2] = glm::dot(gamst, pMinusQ) + glm::dot(gamt, -gams);
			Ji[3][3] = glm::dot(gamtt, pMinusQ) + glm::dot(gamt, -gamt);

			// Need to solve J * deltax = -y to get deltax, so negate yi:
			for (int i = 0; i < 4; i++) {
				yi[i] *= -1.0f;
			}

			solve(Ji, yi, deltaX); //0,0 and 2,2 are nan

								   //update xi by deltax:
			for (int i = 0; i < 4; i++) {
				xi[i] += deltaX[i];
			}

			float curDist = 0.0f;
			//check if solution is found:
			evalSystem(sq1, sq2, xi, yi, curDist);

			glm::vec2 dx1(deltaX[0], deltaX[1]);
			glm::vec2 dx2(deltaX[2], deltaX[3]);

			if (compareYs(yi, prevY)) {
				curIterations = maxIterations;
				continue;
			}

			if (curDist < closestDistance) {
				closest1.x = xi[0];
				closest1.y = xi[1];
				closest2.x = xi[2];
				closest2.y = xi[3];


			}

			if (curIterations > 1) {
				if (smallVecDiff(dx1, prevDx1, .001f) && smallVecDiff(dx2, prevDx2, .001f)) {

					if (print)
						std::cout << "diff close enough" << std::endl;
					break;
				}
			}

			prevDx1 = dx1;
			prevDx2 = dx2;

			//if (((xi[2] - PI_OVER_2) < .01f || absf(xi[2] + PI_OVER_2) < .01f)) {
			//	//v is approximately pi/2 or -pi/2
			//	if (!sq2vLeft2) {
			//		//Try adjusting to the left side of the line v1 = +/- PI/2:
			//		xi[2] -= PI_OVER_4;
			//		sq2vLeft2 = true;
			//		continue;
			//	}
			//	else if (!sq2vRight2) {
			//		//Try adjusting to the left side of the line v1 = +/- PI/2:
			//		xi[2] += PI_OVER_4;
			//		sq2vRight2 = true;
			//		continue;
			//	}
			//	else {
			//		//Already tried left and right, try going up and down on the line v1 = +/- PI/2:
			//		// by adjusting u:

			//		//p1 is current point, p2 is positively adjusted, p3 is negatively adjusted 
			//		glm::vec3 p1, p2, p3;
			//		p1 = getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2], xi[3]), sq2.translation, sq2.getRotationMatrix());
			//		p2 = getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] + PI_OVER_4, xi[3]), sq2.translation, sq2.getRotationMatrix());
			//		p3 = getGlobalCoordinates(sq2.PointsFromSurfaceParams(xi[2] - PI_OVER_4, xi[3]), sq2.translation, sq2.getRotationMatrix());

			//		//Current point on other surface to use for evaluating closeness
			//		p = getGlobalCoordinates(sq1.PointsFromSurfaceParams(xi[0], xi[1]), sq1.translation, sq1.getRotationMatrix());

			//		int result = findClosest(p1, p2, p3, p);

			//		if (result == 1) {
			//			//Current point is closer than adjusting u each direction
			//			// Good chance this is the actual closest point
			//			// ...if p is close to sq2's actual closest point
			//			// ......so hope it is and resume one:
			//			pp1.u = xi[0];
			//			pp1.v = xi[1];
			//			pp1.pt = p1;
			//			pp2.pt = p;
			//			pp2.u = xi[2];
			//			pp2.v = xi[3];
			//			ResumeBruteForceOne(sq1, sq2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), pp2, PI_OVER_4, bruteForceAccuracy);

			//			if (print)
			//				std::cout << "ResumeOne ing on V" << std::endl;
			//			return false;
			//		}
			//		else {
			//			glm::vec3 prevClosest, closest;
			//			closest = p2;
			//			float update = PI_OVER_4;
			//			if (result == 3) {
			//				//negative direction is the closer direction:
			//				offset = -offset;
			//				closest = p3;
			//			}
			//			searchOneDirection(sq2, closest, p, &xi[2], &xi[3], &xi[3], update);

			//			//Now Resume Brute force search:
			//			pp1.u = xi[0];
			//			pp1.v = xi[1];
			//			pp1.pt = closest;
			//			pp2.pt = p;
			//			pp2.u = xi[2];
			//			pp2.v = xi[3];

			//			ResumeBruteForce(sq1, sq2, pp1, pp2, (sq1closest = pp1.pt), (sq2closest = pp2.pt), update, bruteForceAccuracy);
			//			if (print)
			//				std::cout << "ResumeOne ing on v" << std::endl;
			//			return false;
			//		}
			//	}
			//}

			if (isnan(xi[0]) || isnan(xi[1]) || isnan(xi[2]) || isnan(xi[3]) ||
				isnan(yi[0]) || isnan(yi[1]) || isnan(yi[2]) || isnan(yi[3])) {
				yi[0] = 100.0f;
				curIterations = maxIterations; //force to diverge
			}
		}

		//Return the 3d points in their world space coordinates

		sq1closest = sq1.PointsFromSurfaceParams(xi[0], xi[1]);
		sq1closest = ShapeUtils::getGlobalCoordinates(sq1closest, sq1.translation, sq1Rot, sq1.scaling);

		sq2closest = sq2.PointsFromSurfaceParams(xi[2], xi[3]);
		sq2closest = ShapeUtils::getGlobalCoordinates(sq2closest, sq2.translation, sq2Rot, sq2.scaling);

		pp1.u = xi[0];
		pp1.v = xi[1];
		pp2.u = xi[2];
		pp2.v = xi[3];
		pp1.pt = sq1closest;
		pp2.pt = sq2closest;

		if (print)
			std::cout << "Converged at " << pp1.u << " , " << pp1.v << "  -  " << pp2.u << " , " << pp2.v << std::endl;
		//if (print)
		//	std::cout << "Iterations: " << curIterations << std::endl;
		return true;
	}

	static void searchOneDirection(Superquadric &sq, glm::vec3 &closestPt, glm::vec3 &refPt, float *closestParamU, float *closestParamV, float *closestParamVariable, float update) {
		glm::vec3 prevClosest;

		float prevDist, dist;
		prevDist = 1000.0f;

		dist = ShapeUtils::squaredDistance(closestPt, refPt);

		glm::mat4 rotMat = sq.getRotationMatrix();

		while (dist < prevDist) {
			prevDist = dist;
			prevClosest = closestPt;

			*closestParamVariable = *closestParamVariable + update;
			closestPt = ShapeUtils::getGlobalCoordinates(sq.PointsFromSurfaceParams(*closestParamU, *closestParamV), sq.translation, rotMat, sq.scaling);
			dist = ShapeUtils::squaredDistance(closestPt, refPt);
		}
		closestPt = prevClosest;

		//Try half distance
		/*while (dist < prevDist) {
		prevDist = dist;
		prevClosest = closestPt;

		*closestParamVariable = *closestParamVariable + update;
		closestPt = getGlobalCoordinates(sq.PointsFromSurfaceParams(*closestParamU, *closestParamV), sq.translation, sq.getRotationMatrix());
		dist = ShapeUtils::squaredDistance(closestPt, refPt);
		}*/
	}

	//Returns 1 if p1 closest to ref, 2 if p2 is closest to ref, and 3 if p3
	static int findClosest(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &ref) {
		float dist1, dist2, dist3;
		dist1 = ShapeUtils::squaredDistance(p1, ref);
		dist2 = ShapeUtils::squaredDistance(p2, ref);
		dist3 = ShapeUtils::squaredDistance(p3, ref);

		if (dist2 < dist1) {
			if (dist2 < dist3) {
				return 2;
			}
			else {
				return 3;
			}
		}
		else if (dist3 < dist1) {
			if (dist3 < dist2) {
				return 3;
			}
			else {
				return 2;
			}
		}
		else {
			return 1;
		}
	}

	//Returns 1 if p1 closest to ref, 2 if p2 is closest to ref, and 3 if p3
	static int findClosestOf4(glm::vec3 &p1, glm::vec3 &p2, glm::vec3 &p3, glm::vec3 &p4, glm::vec3 &ref) {
		float dist1, dist2, dist3, dist4;
		dist1 = ShapeUtils::squaredDistance(p1, ref);
		dist2 = ShapeUtils::squaredDistance(p2, ref);
		dist3 = ShapeUtils::squaredDistance(p3, ref);
		dist4 = ShapeUtils::squaredDistance(p4, ref);

		int bestOf3 = findClosest(p1, p2, p3, ref);
		if (bestOf3 == 1) {
			if (dist4 < dist1) {
				return 4;
			}
			return 1;
		}
		else if (bestOf3 == 2) {
			if (dist4 < dist2) {
				return 4;
			}
			return 2;
		}
		else {
			if (dist4 < dist3) {
				return 4;
			}
			return 3;
		}
	}

	static bool checkAllVecs(glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &v3, glm::vec3 &v4, glm::vec3 &v5) {
		return checkVecForNaN(v1) || checkVecForNaN(v2) || checkVecForNaN(v3) || checkVecForNaN(v4) || checkVecForNaN(v5);
	}

	static bool compareYs(float y1[4], float y2[4]) {
		return MathUtils::abs(y1[0]) > MathUtils::abs(y2[0]) && MathUtils::abs(y1[1]) > MathUtils::abs(y2[1]) && MathUtils::abs(y1[2]) > MathUtils::abs(y2[2]) && MathUtils::abs(y1[3]) > MathUtils::abs(y2[3]);
	}

	static bool smallVecDiff(glm::vec2 v1, glm::vec2 v2, float diff) {
		return MathUtils::abs(v1.x - v2.x) < diff && MathUtils::abs(v1.y - v2.y) < diff;
	}

	static bool checkVecForNaN(glm::vec3 vec) {
		return isnan(vec.x) || isnan(vec.y) || isnan(vec.z) || isinf(vec.x) || isinf(vec.y) || isinf(vec.z);
	}

	static void evalSystem(Superquadric &sq1, Superquadric &sq2, float xi[4], float yi[4], float &closestDist) {
		glm::vec3 p, q, pMinusQ, sigu, sigv, gams, gamt;

		glm::mat4 sq1Rot = sq1.getRotationMatrix();
		glm::mat4 sq2Rot = sq2.getRotationMatrix();

		//p is point on sq1 in 3d coordinates, transformed to position in world space.
		// q is the point on sq2
		p = sq1.PointsFromSurfaceParams(xi[0], xi[1]);
		q = sq2.PointsFromSurfaceParams(xi[2], xi[3]);
		p = ShapeUtils::getGlobalCoordinates(p, sq1.translation, sq1Rot, sq1.scaling);
		q = ShapeUtils::getGlobalCoordinates(q, sq2.translation, sq2Rot, sq2.scaling);

		sigu = sq1.dSigmaDu(xi[0], xi[1]);
		sigv = sq1.dSigmaDv(xi[0], xi[1]);
		gams = sq2.dSigmaDu(xi[2], xi[3]);
		gamt = sq2.dSigmaDv(xi[2], xi[3]);

		//TODO apply trans/rot?
		// i think just rotation:...

		sigu = applyRotation(sigu, sq1Rot);
		sigv = applyRotation(sigv, sq1Rot);
		gams = applyRotation(gams, sq2Rot);
		gamt = applyRotation(gamt, sq2Rot);

		//trying with translation:
		/*sigu = getGlobalCoordinates(sigu, sq1trans, sq1rot);
		sigv = getGlobalCoordinates(sigv, sq1trans, sq1rot);
		gams = getGlobalCoordinates(gams, sq2trans, sq2rot);
		gamt = getGlobalCoordinates(gamt, sq2trans, sq2rot);*/

		closestDist = ShapeUtils::squaredDistance(p, q);
		pMinusQ = (p - q);
		yi[0] = glm::dot(sigu, pMinusQ);
		yi[1] = glm::dot(sigv, pMinusQ);
		yi[2] = glm::dot(gams, pMinusQ);
		yi[3] = glm::dot(gamt, pMinusQ);
	}

	void doubleNanAdjustsments() {
		for (int i = 0; i < 8; i++) {
			this->nanAdjustments[i].x = this->adjustment * 4.0f;
			this->nanAdjustments[i].y = this->adjustment * 4.0f;
		}
	}

	///////////////////////////////////
	///////////////////////////////////

	// TODO it's possible the functions used can produce a nan
	//pt must be on surface of superquadric and in local coordinate system (center of object at origin)
	glm::vec2 SurfaceParamValuesFromSurfacePoint(glm::vec3 pt) {
		// First solve for u, then use that in solving for v:
		float u, v;

		float pi = glm::pi<float>();
		float piOver2 = pi / 2.0f;

		// u = sin^(-1) ( e ^ ( ln(z/a3) / e1) )
		float sign = MathUtils::sgnf(pt.z);
		//u = sign * asin(expf(logf(MathUtils::abs(pt.z) / a3) / e1));        //logf = ln
		u = asin(sign * expf(logf(MathUtils::abs(pt.z) / a3) / e1));

		if (MathUtils::abs(u + piOver2) < SQ_EPSILON) {
			//u = -pi/2, v can be anything
			v = 0.0f;
		}
		else if (MathUtils::abs(u - piOver2) < SQ_EPSILON) {
			//u = pi/2, v can be anything
			v = 0.0f;
		}
		else {

			// v = arccos( e ^ ( ln (x/a1 * cos(u)^e1) / e2))
			float cosU = MathUtils::cos(u);
			float logDenom = a1 * powf(MathUtils::abs(cosU), e1);
			float logParam = ShapeUtils::absf(pt.x) / logDenom;
			float log = logf(logParam);

			float ex = log / e2;
			ex = expf(ex);

			if (ex > 1.0f) {
				ex = 1.0f; //Sometimes from floating pt rounding, value is 1.0000004, which causes nan in asin
			}

			//ex is always going to be positive due to the abs(pt.x)
			// Remultiply the sign onto ex before evaluating acos:
			if (pt.x < 0.0f) {
				ex *= -1.0f;
			}
			if (cosU < 0.0f) {
				ex *= -1.0f;
			}

			v = acos(ex); //returns something in [0, pi]

			// v is supposed to be in range: [-pi, pi]
			//  So look at pt.y sign and sign of (cos(u)^e1) to determine if v's sign should be negated
			bool x = logDenom > 0.0f;
			bool y = pt.y > 0.0f;
			if (x!=y) {
				v *= -1.0f;
			}

			if (isnan(v)) {
				std::cout << "V is nan" << std::endl;
			}

			//v = pi gives same point as v = -pi
		}
		return glm::vec2(u, v);
	}

	glm::vec2 SurfaceParamValuesFromNormal(glm::vec3 &normal) {
		float u, v; // return values

		float s1, s2, s3, a1n1, a2n2, a3n3, atanParam1, atanParam2, delta1, delta2;

		s1 = MathUtils::sgnf(normal.x);
		s2 = MathUtils::sgnf(normal.y);
		s3 = MathUtils::sgnf(normal.z);

		a1n1 = MathUtils::abs(a1 * normal.x);
		a2n2 = MathUtils::abs(a2 * normal.y);
		a3n3 = a3 * normal.z;

		delta1 = 1.0f / (2.0f - e1);
		delta2 = 1.0f / (2.0f - e2);

		atanParam1 = s1 * powf(a1n1, delta2);
		atanParam2 = s2 * powf(a2n2, delta2);

		v = atan2(atanParam2, atanParam1);

		if (a1n1 > a2n2) {
			atanParam1 = powf(a1n1, delta1);
			atanParam2 = s3 * powf(MathUtils::abs(a3n3 * powf(MathUtils::abs(MathUtils::cos(v)), 2.0f - e2)), delta1);
		}
		else {
			atanParam1 = powf(a2n2, delta1);
			atanParam2 = s3 * powf(MathUtils::abs(a3n3 * powf(MathUtils::abs(MathUtils::sin(v)), 2.0f - e2)), delta1);
		}

		u = atan2(atanParam2, atanParam1);

		return glm::vec2(u, v);
	}

	glm::vec3 NormalFromSurfaceParams(glm::vec2 &params) {
		return NormalFromSurfaceParams(params.x, params.y);
	}

	glm::vec3 NormalFromSurfaceParams(float u, float v) {
		glm::vec3 normal;

		float cosX, cosY, sinX, sinY;
		cosX = MathUtils::cos(u);
		cosY = MathUtils::cos(v);
		sinX = MathUtils::sin(u);
		sinY = MathUtils::sin(v);

		float fstVal = MathUtils::sgnf(cosY) * (1.0f / a1);
		float sndVal = powf(MathUtils::abs(cosY), 2.0f - e2);
		float lstVal = MathUtils::sgnf(cosX) * powf(MathUtils::abs(cosX), 2.0f - e1);
		normal.x = fstVal * sndVal * lstVal;

		fstVal = MathUtils::sgnf(sinY) * (1.0f / a2);
		sndVal = powf(MathUtils::abs(sinY), 2.0f - e2);
		//lstVal same as normal.x lstval
		normal.y = fstVal * sndVal * lstVal;

		fstVal = MathUtils::sgnf(sinX) * (1.0f / a3);
		sndVal = powf(MathUtils::abs(sinX), 2.0f - e1);
		normal.z = fstVal * sndVal;

		return normal;
	}

	glm::vec3 PointsFromSurfaceParams(float u, float v) {
		float cosX, cosY, sinX, sinY;
		cosX = MathUtils::cos(u);
		cosY = MathUtils::cos(v);
		sinX = MathUtils::sin(u);
		sinY = MathUtils::sin(v);
		float lastVal = powf(MathUtils::abs(cosX), e1);

		glm::vec3 xyz;
		xyz.x = MathUtils::sgnf(cosY) * a1 * powf(MathUtils::abs(cosY), e2) * lastVal;
		xyz.y = MathUtils::sgnf(sinY) * a2 * powf(MathUtils::abs(sinY), e2) * lastVal;
		xyz.z = MathUtils::sgnf(sinX) * a3 * powf(MathUtils::abs(sinX), e1);

		return xyz;
	}

	float f(float x, float y, float z) {
		float result;

		float exp = 1.0f / e1;
		float first = powf(powf(x / a1, 2.0f), exp);
		float secnd = powf(powf(y / a2, 2.0f), exp);
		float sum = powf(first + secnd, e1 / e2);
		float third = powf(powf(z / a3, 2.0f), (1.0f / e2));

		result = sum + third;


		//result = powf(powf(x / a1, 2.0f / e1) +
		//	powf(y / a2, 2.0f / e1), e1 / e2) +
		//	powf(z / a3, 2.0f / e2);
		return result;
	}

	float f(glm::vec3 pt) {
		return f(pt.x, pt.y, pt.z);
	}

	//Solves for x in system: Jx = y
	static void solve(float J[4][4], float y[4], float x[4]) {
		LUbksub(J, y, x);
	}

	static void LUdcmp(float Aorig[4][4], float L[4][4], float U[4][4], float P[4][4]) {
		//Setup matrices:
		float A[4][4];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (i == j) {
					P[i][j] = 1.0f;
				}
				else {
					P[i][j] = 0.0f;
				}
				if (i > j) {
					U[i][j] = 0.0f;
				}
				else if (i < j) {
					L[i][j] = 0.0f;
				}

				A[i][j] = Aorig[i][j];
			}
		}

		for (int j = 0; j < 4; j++) {
			//pivoting:  
			int maxRow = 0;
			float maxVal = 0.0;
			for (int i = j; i < 4; i++) {
				float value = 0.0f;
				for (int k = 0; k < j; k++) {
					value += L[i][k] * U[k][j];
				}
				value = MathUtils::abs(A[i][j] - value);
				if (value > maxVal) {
					maxVal = value;
					maxRow = i;
				}
			}
			if (j != maxRow) {
				swapRows(A, j, maxRow);
				swapRows(P, j, maxRow);
				swapRows(L, j, maxRow);
			}
			L[j][j] = 1.0f;

			for (int i = 0; i <= j; i++) {
				float sum = 0.0f;
				for (int k = 0; k < i; k++) {
					sum += L[i][k] * U[k][j];
				}
				U[i][j] = A[i][j] - sum;
			}
			for (int i = j + 1; i < 4; i++) {
				float sum = 0.0f;
				for (int k = 0; k <= j - 1; k++) {
					sum += L[i][k] * U[k][j];
				}
				sum = A[i][j] - sum;
				if (MathUtils::abs(U[j][j]) < SQ_EPSILON)
					sum = 0.0f;
				else
					sum /= U[j][j];
				L[i][j] = sum;
			}
		}

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (i > j) {
					U[i][j] = 0.0f;
				}
				else if (i < j) {
					L[i][j] = 0.0f;
				}
			}
		}
	}

	static void LUbksub(float A[4][4], float b[4], float x[4]) {
		float P[4][4];
		float U[4][4];
		float L[4][4];

		float bPrime[4];
		float y[4];
		LUdcmp(A, L, U, P);

		//Permute b so it matches the row interchanges performed on A:
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (P[i][j] == 1.0) {
					bPrime[i] = b[j];
				}
			}
		}

		//Solve L y = bPrime:
		for (int i = 0; i < 4; i++) {
			float value = 0.0;
			for (int j = 0; j < i; j++) {
				value += L[i][j] * y[j];
			}
			y[i] = (bPrime[i] - value) / L[i][i];
		}

		// Then solve U x = y :
		for (int i = 4 - 1; i >= 0; i--) {
			float value = 0.0;
			for (int j = i + 1; j < 4; j++) {
				value += U[i][j] * x[j];
			}
			if (MathUtils::abs(U[i][i]) < SQ_EPSILON) {
				x[i] = 0.0;
			}
			else {
				x[i] = (y[i] - value) / U[i][i];
			}
		}
	}

	static void swapRows(float matrix[4][4], int rowA, int rowB) {
		float aTemp[4] = { matrix[rowA][0], matrix[rowA][1], matrix[rowA][2], matrix[rowA][3] };
		for (int i = 0; i < 4; i++) {
			matrix[rowA][i] = matrix[rowB][i];
			matrix[rowB][i] = aTemp[i];
		}
	}

	//static glm::vec3 getLocalCoordinates(glm::vec3 globalCoords, glm::vec3 &translation, glm::mat4 &rotation) {
	//	//  Translate back to origin, then un-rotate to standard orientation
	//	glm::vec3 local = (-1.0f * translation) + globalCoords;
	//	glm::mat4 transpose = glm::transpose(rotation);
	//	return applyRotation(local, transpose);   //Transpose of rotation matrix is its inverse since orthogonal
	//}

	//static glm::vec3 getGlobalCoordinates(glm::vec3 localCoords, glm::vec3 &translation, glm::mat4 &rotation) {
	//	//  Rotate pt then translate it:
	//	glm::vec3 global = applyRotation(localCoords, rotation);
	//	return global + translation;
	//}

	static glm::vec3 applyRotation(glm::vec3 &pt, glm::mat4 &rotation) {
		return glm::vec3(rotation * glm::vec4(pt.x, pt.y, pt.z, 1.0f));
	}

	static bool IsZeroVector(float y[4]) {
		for (int i = 0; i < 4; i++) {
			if (MathUtils::abs(y[i]) > SQ_EPSILON) {
				return false;
			}
		}
		return true;
	}

	void evalParams(float u, float v, glm::vec3 &xyz, glm::vec3 &normal) {
		xyz.x = a1 * sqC(u, e1) * sqC(v, e2);
		xyz.y = a2 * sqC(u, e1) * sqS(v, e2);
		xyz.z = a3 * sqS(u, e1);
		normal.x = sqC(u, 2 - e1) * sqC(v, 2 - e2) / a1;
		normal.y = sqC(u, 2 - e1) * sqS(v, 2 - e2) / a2;
		normal.z = sqS(u, 2 - e1) / a3;
	}

	float sqC(float x, float e) {
		float cosx = MathUtils::cos(x);
		return MathUtils::sgnf(cosx) * powf(MathUtils::abs(cosx), e);
	}

	float sqS(float x, float e) {
		float sinx = MathUtils::sin(x);
		return MathUtils::sgnf(sinx) * powf(MathUtils::abs(sinx), e);
	}

	float fx(float x, float y, float z) {
		float d, twoovere1, first, second;
		d = e1 / e2;
		twoovere1 = 2.0f / e1;
		first = d * MathUtils::sgnf(x) * powf((powf(((1.0f / a1) * MathUtils::abs(x)), twoovere1) + MathUtils::sgnf(y) * powf(((1.0f / a2) * MathUtils::abs(y)), twoovere1)), d - 1.0f);
		second = powf(1.0f / a1, twoovere1) * (MathUtils::sgnf(x) * twoovere1 * powf(MathUtils::abs(x), twoovere1 - 1.0f));
		return first * second;
	}

	float fy(float x, float y, float z) {
		float d, twoovere1, first, second;
		d = e1 / e2;
		twoovere1 = 2.0f / e1;
		first = d * MathUtils::sgnf(x) * powf((powf(((1.0f / a1) * MathUtils::abs(x)), twoovere1) + MathUtils::sgnf(y) * powf(((1.0f / a2) * MathUtils::abs(y)), twoovere1)), d - 1.0f);
		second = powf(1.0f / a2, twoovere1) * (MathUtils::sgnf(y) * twoovere1 * powf(MathUtils::abs(y), twoovere1 - 1.0f));
		if (isnan(first) || isnan(second)) {
			std::cout << "fY is nan" << std::endl;
		}
		return first * second;
	}

	float fz(float x, float y, float z) {
		float twoovere2;
		twoovere2 = 2.0f / e2;
		float term1 = powf((1.0f / a3), twoovere2);
		float term3 = MathUtils::sgnf(z) * powf(MathUtils::abs(z), twoovere2 - 1.0f);
		return term1 * twoovere2 * term3;
	}

	//static float myPowf(float base, float exp) {
	//	if (base < 0.0f) {
	//		if (MathUtils::abs(exp - roundf(exp)) < .01f) {
	//			if (MathUtils::abs(exp / 2.0f - roundf(exp / 2.0f)) < .01f) {
	//				//even exponent
	//				return powf(MathUtils::abs(base), exp);
	//			}
	//		}
	//		return -1.0f * powf(MathUtils::abs(base), exp);
	//	}
	//	return powf(base, exp);
	//}

	/*glm::vec3 gradient(float x, float y, float z) {
	float d, twoovere2, twoovere1, first, second;
	glm::vec3 grad;
	d = e2 / e1;
	twoovere2 = 2.0f / e2;
	twoovere1 = 2.0f / e1;

	first = d * powf((powf(((1.0f / a1) * x), twoovere2) + powf(((1.0f / a2) * y), twoovere2)), d - 1.0f);
	second = powf(1.0f / a1, twoovere2) * (twoovere2 * powf(x, twoovere2 - 1.0f));
	grad.x = first * second;

	second = powf(1.0f / a2, twoovere2) * (twoovere2 * powf(y, twoovere2 - 1.0f));
	grad.y = first * second;

	grad.z = powf((1.0f / a3), twoovere1) * twoovere1 * powf(z, twoovere1 - 1.0f);

	return grad;
	}*/

	float gradientmagnitude(float fx, float  fy, float  fz) {
		return sqrtf(powf(fx, 2.0f) + powf(fy, 2.0f) + powf(fz, 2.0f));
	}

	glm::vec3 unitnormal(glm::vec3 pt) {
		return unitnormal(pt.x, pt.y, pt.z);
	}

	glm::vec3 unitnormal(float x, float y, float z) {
		float gradx, grady, gradz;
		gradx = fx(x, y, z);
		grady = fy(x, y, z);
		gradz = fz(x, y, z);
		return unitnormalfromgradient(gradx, grady, gradz);
	}

	glm::vec3 unitnormalfromgradient(float fx, float fy, float fz) {
		float mag = gradientmagnitude(fx, fy, fz);
		return glm::vec3(fx / mag, fy / mag, fz / mag);
	}

	void AddTrianglePoints(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3, glm::vec3 pt1Normal, glm::vec3 pt2Normal, glm::vec3 pt3Normal) {
		AddPoint(pt1, pt1Normal);
		AddPoint(pt2, pt2Normal);
		AddPoint(pt3, pt3Normal);

		//Track data on triangle area:
		float A = glm::distance(pt1, pt2);
		float B = glm::distance(pt2, pt3);
		float C = glm::distance(pt1, pt3);

		float p = (A + B + C) / 2.0f;

		float area = sqrt(p * (p - A) * (p - B) * (p - C));

		allTriangleAreas.push_back(area);
	}

	//pt1, pt2, pt3, and pt4 should be in counterclockwise order
	void AddSquarePoints(glm::vec3 pt1, glm::vec3 pt2, glm::vec3 pt3, glm::vec3 pt4,
		glm::vec3 pt1Normal, glm::vec3 pt2Normal, glm::vec3 pt3Normal, glm::vec3 pt4Normal) {

		AddTrianglePoints(pt1, pt2, pt3, pt1Normal, pt2Normal, pt3Normal);
		AddTrianglePoints(pt1, pt3, pt4, pt1Normal, pt3Normal, pt4Normal);
	}

	void AddPoint(glm::vec3 pt, glm::vec3 ptNormal) {
		this->vertices.push_back(pt.x);
		this->vertices.push_back(pt.y);
		this->vertices.push_back(pt.z);
		this->vertices.push_back(ptNormal.x);
		this->vertices.push_back(ptNormal.y);
		this->vertices.push_back(ptNormal.z);
	}

	bool isNonsingular(float u, float v) {
		return checkVecForNaN(dSigmaDu(u, v)) || checkVecForNaN(dSigmaDv(u, v))
			|| checkVecForNaN(ddSigmaDuDu(u, v)) || checkVecForNaN(ddSigmaDuDv(u, v)) || checkVecForNaN(ddSigmaDvDv(u, v));
	}

	virtual std::string getShapeCSVline1() override {

		std::string codeStr = std::to_string(this->csvCode);
		std::ostringstream os;
		os << codeStr;

		if (this->csvCode == 4) {
			//Only need extra info for custom superquadrics:
			os << "," << this->a1 << "," << this->a2 << "," << this->a3;
			os << "," << this->e1 << "," << this->e2;
		}
		os << std::endl;
		return os.str();
	}

	void setShapeCSVcode(int code) {
		this->csvCode = code;
	}
};

class SuperEllipsoid {
public:


	/* sqC (v, n)
	* This function implements the c(v,n) utility function
	*
	* c(v,n) = sgnf(cos(v)) * |cos(v)|^n
	*/
	static float sqC(float v, float n) {
		return MathUtils::sgnf(MathUtils::cos(v)) * (float)powf(MathUtils::abs(MathUtils::cos(v)), n);
	}

	/* sqCT (v, n, alpha)
	* This function implements the CT(v,n,alpha) utility function
	*
	* CT(v,n,alpha) = alpha + c(v,n)
	*/
	static float sqCT(float v, float n, float alpha) {
		return alpha + sqC(v, n);
	}

	/* sqS (v, n)
	* This function implements the s(v,n) utility function
	*
	* s(v,n) = sgnf(sin(v)) * |sin(v)|^n
	*/
	static float sqS(float v, float n) {
		return MathUtils::sgnf(MathUtils::sin(v)) * (float)powf(MathUtils::abs(MathUtils::sin(v)), n);
	}

	//-------------------------------------------------------------------------

	/* sqEllipsoid(a1, a2, a3, u, v, n, e, *x, *y, *z, *nx, *ny, *nz)
	*
	* a1, a2, and a3 are the x, y, and z scaling factors, respecfully.
	* For proper generation of the solid, u should be >= -PI / 2 and <= PI / 2.
	* Similarly, v should be >= -PI and <= PI.
	*/
	static void sqEllipsoid(float a1, float a2, float a3, float u, float v, float e1, float e2,
		float *x, float  *y, float *z, float *nx, float *ny, float *nz) {
		//transformParams(u, v, e1, e2);
		*x = a1 * sqC(u, e1) * sqC(v, e2);
		*y = a2 * sqC(u, e1) * sqS(v, e2);
		*z = a3 * sqS(u, e1);
		*nx = sqC(u, 2 - e1) * sqC(v, 2 - e2) / a1;
		*ny = sqC(u, 2 - e1) * sqS(v, 2 - e2) / a2;
		*nz = sqS(u, 2 - e1) / a3;
	}

	/*
	static void uniformAreaPoints(Superquadric &sq) {
	const double PI = glm::pi<double>();

	double uLowerBound, uUpperBound, vLowerBound, vUpperBound;

	uLowerBound = -PI / 2.0;
	uUpperBound = PI / 2.0;
	vLowerBound = -PI;
	vUpperBound = PI;

	double dividend = 8.0;

	//Discretization of the domain
	double squareLength = PI / dividend; //Step size
	double halfSquareLength = squareLength / 2.0;

	std::vector<std::vector<double>> squareAreas;
	double u, v, uCenter, vCenter, area;
	double totalArea = 0.0;

	//Each discretized pt, (u,v), represents the bottom left vertex of a square in the domain
	// Calculates the surface area at center of each square (u+halfSquareLength, v+halfSquareLength),
	// And finds the total area
	for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	std::vector<double> curUsquares;
	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	uCenter = u + halfSquareLength;
	vCenter = v + halfSquareLength;
	area = SuperEllipsoid::SurfaceArea(sq, u, v);
	totalArea += area;
	curUsquares.push_back(area);
	}
	squareAreas.push_back(curUsquares);
	}

	int zeroes, ones, twos, threes, fours, fives, sixes, sevens, eights, nines, tenPlus;
	zeroes = ones = twos = threes = fours = fives = sixes = sevens = eights = nines = tenPlus = 0;

	int avgPtsPerSquare = 3;
	int numPoints = (int)(2.0*dividend) * (int) (dividend) * avgPtsPerSquare; // Dividend*2 # of v segments, Dividend # of u segments, gives (dividend^2 / 2) squares, * avgPtsPerSquare

	//To generate numPoints number of points, let every square contain M = numPoints * (a / totalArea),
	// where a = the area of each square (surface area evaluated at center)
	// Generates the m points roughly equally spaced in each square
	int uIndex, vIndex;
	uIndex = 0;
	double pointsToAreaRatio = numPoints / totalArea;

	glm::dvec3 center, centerN, pt2, pt3, pt4, pt5, pt6, pt2N, pt3N, pt4N, pt5N, pt6N;

	//Pt offsets from btmLeft
	double thirdLength, twoThirdsLength, oneQuarterLength, threeQuarterLength;
	thirdLength = (1.0f / 3.0f) * squareLength;
	twoThirdsLength = (thirdLength * 2.0f);
	oneQuarterLength = .25f * squareLength;
	threeQuarterLength = oneQuarterLength * 3.0f;

	double oneFifthLength, twoFifths, threeFifths, fourFifths;
	oneFifthLength = (.2f * squareLength);
	twoFifths = oneFifthLength * 2.0f;
	threeFifths = oneFifthLength * 3.0f;
	fourFifths = oneFifthLength * 4.0f;

	std::map<float, std::map<float, glm::vec3>> points;

	int numTris = 0;
	for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	std::vector<double> curUsquares =  squareAreas[uIndex++];
	vIndex = 0;
	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	double curSquareArea = curUsquares[vIndex++];

	glm::dvec3 btmLeft, btmLeftN, topRight, topRightN, topLeft, topLeftN, btmRight, btmRightN;
	sq.evalParams(u, v, btmLeft, btmLeftN);
	sq.evalParams(u + squareLength, v + squareLength, topRight, topRightN);
	sq.evalParams(u, v + squareLength, topLeft, topLeftN);
	sq.evalParams(u + squareLength, v, btmRight, btmRightN);


	if (points.find(u) == points.end()) {
	points[u] = std::map<float, glm::vec3>();
	}
	points[u][v] = btmLeft;
	points[u][v + squareLength] = topLeft;

	if (points.find(u+squareLength) == points.end()) {
	points[u+squareLength] = std::map<float, glm::vec3>();
	}
	points[u + squareLength][v] = btmRight;
	points[u + squareLength][v + squareLength] = topRight;

	//All squares will have at least 1 pt, the bottom left vertex of the square
	// To form triangles, each square will also utilize the bottom-left vertices of the
	// above, right, and above-right squares
	int m = round(pointsToAreaRatio * curSquareArea);

	m = (m > 1 ? m : 1);

	//Based on the amount of points for this square, try to roughly equally space
	// them between the 4 corners of the square
	switch (m) {
	case 0:
	zeroes++;
	break;

	case 1:
	//Just use bottom left corner of square
	//Triangles will be: T1 = btmLeft, topLeft, topRight, T2 = btmLeft, topRight, btmRight
	sq.AddTrianglePoints(btmLeft, topLeft, btmRight, btmLeftN, topLeftN, btmRightN);
	sq.AddTrianglePoints(topLeft, topRight, btmRight, topLeftN, topRightN, btmRightN);
	numTris += 2;
	//std::cout << "1" << std::endl;
	ones++;
	break;

	case 2:
	//Use btm left pt and point right in center
	//the 4 Triangles will look like:
	//          _____
	//			|\ /|
	//			|/_\|
	sq.evalParams(u + halfSquareLength, v + halfSquareLength, center, centerN);

	sq.AddTrianglePoints(btmLeft, center, topLeft, btmLeftN, centerN, topLeftN);
	sq.AddTrianglePoints(btmLeft, btmRight, center, btmLeftN, btmRightN, centerN);
	sq.AddTrianglePoints(topLeft, center, topRight, topLeftN, centerN, topRightN);
	sq.AddTrianglePoints(topRight, center, btmRight, topRightN, centerN, btmRightN);
	numTris += 4;

	if (points.find(u + halfSquareLength)==points.end()) {
	points[u + halfSquareLength] = std::map<float, glm::vec3>();
	}
	points[u + halfSquareLength][v + halfSquareLength] = center;

	twos++;
	//std::cout << "2" << std::endl;
	break;

	case 3:
	// Use btmLeft as pt1, (.333,.666) as pt2, and (.666, .333) as pt3 (Assuming here each side length is 1)
	// Triangles: t1 = pt1, topLeft, pt2
	//			  t2 = pt1, pt2, pt3
	//            t3 = pt1, pt3, btmRight
	//            t4 = topLeft, pt2, topRight
	//            t5 = topRight, pt2, pt3
	//			  t6 = topRight, pt3, btmRight

	sq.evalParams(u + thirdLength, v + twoThirdsLength, pt2, pt2N);
	sq.evalParams(u + twoThirdsLength, v + thirdLength, pt3, pt3N);

	sq.AddTrianglePoints(btmLeft, topLeft, pt2, btmLeftN, topLeftN, pt2N);
	sq.AddTrianglePoints(btmLeft, pt2, pt3, btmLeftN, pt2N, pt2N);
	sq.AddTrianglePoints(btmLeft, pt3, btmRight, btmLeftN, pt3N, btmRightN);
	sq.AddTrianglePoints(topLeft, pt2, topRight, topLeftN, pt2N, topRightN);
	sq.AddTrianglePoints(topRight, pt2, pt3, topRightN, pt2N, pt3N);
	sq.AddTrianglePoints(topRight, pt3, btmRight, topRightN, pt2N, pt3N);
	numTris += 6;

	if (points.find(u + thirdLength) == points.end()) {
	points[u + thirdLength] = std::map<float, glm::vec3>();
	}
	points[u + thirdLength][v + twoThirdsLength] = pt2;

	if (points.find(u + twoThirdsLength) == points.end()) {
	points[u + twoThirdsLength] = std::map<float, glm::vec3>();
	}
	points[u + twoThirdsLength][v + thirdLength] = pt3;

	threes++;
	//std::cout << "3" << std::endl;
	break;

	case 4:
	//btmLeft = pt1, (.25, .75) = pt2, (.5, .25) = pt3, (.75, .75) = pt4
	//Triangles:    t1 = pt1, topLeft, pt2
	//				t2 = pt1, pt2, pt3
	//				t3 = pt1, pt3, btmRight
	//				t4 = pt3, pt4, btmRight
	//				t5 = pt2, pt3, pt4
	//				t6 = btmRight, pt4, topRight
	//				t7 = pt2, pt4, topRight
	//				t8 = topLeft, pt2, topRight

	sq.evalParams(u + oneQuarterLength, v + threeQuarterLength, pt2, pt2N);
	sq.evalParams(u + halfSquareLength, v + oneQuarterLength, pt3, pt3N);
	sq.evalParams(u + threeQuarterLength, v + threeQuarterLength, pt4, pt4N);

	sq.AddTrianglePoints(btmLeft, topLeft, pt2, btmLeftN, topLeftN, pt2N);
	sq.AddTrianglePoints(btmLeft, pt2, pt3, btmLeftN, pt2N, pt3N);
	sq.AddTrianglePoints(btmLeft, pt3, btmRight, btmLeft, pt3N, btmRightN);
	sq.AddTrianglePoints(pt3, pt4, btmRight, pt3N, pt4N, btmRightN);
	sq.AddTrianglePoints(pt2, pt3, pt4, pt2N, pt3N, pt4N);
	sq.AddTrianglePoints(btmRight, pt4, topRight, btmRightN, pt4N, topRightN);
	sq.AddTrianglePoints(pt2, pt4, topRight, pt2N, pt4N, topRightN);
	sq.AddTrianglePoints(topLeft, pt2, topRight, topLeftN, pt2N, topRightN);
	numTris += 8;

	if (points.find(u + oneQuarterLength) == points.end()) {
	points[u + oneQuarterLength] = std::map<float, glm::vec3>();
	}
	points[u + oneQuarterLength][v + threeQuarterLength] = pt2;

	if (points.find(u + halfSquareLength) == points.end()) {
	points[u + halfSquareLength] = std::map<float, glm::vec3>();
	}
	points[u + halfSquareLength][v + oneQuarterLength] = pt3;

	if (points.find(u + threeQuarterLength) == points.end()) {
	points[u + threeQuarterLength] = std::map<float, glm::vec3>();
	}
	points[u + threeQuarterLength][v + threeQuarterLength] = pt4;

	fours++;
	//std::cout << "4" << std::endl;
	break;

	case 5:
	//btmLeft = pt1, (.2, .6) = pt2, (.4, .2) = pt3, (.6, .8) = pt4, (.8, .4) = pt5
	//Triangles:    t1 = pt1, topLeft, pt2
	//				t2 = pt1, pt2, pt3
	//				t3 = pt1, pt3, btmRight
	//				t4 = pt3, pt4, pt5
	//				t5 = pt2, pt3, pt4
	//				t6 = pt3, pt5, btmRight
	//				t7 = btmRight, topRight, pt5
	//				t8 = pt5, topRight, pt4
	//				t9 = topRight, pt4, topLeft
	//				t10 = topLeft pt2, pt4

	sq.evalParams(u + oneFifthLength, v + threeFifths, pt2, pt2N);
	sq.evalParams(u + twoFifths, v + oneFifthLength, pt3, pt3N);
	sq.evalParams(u + threeFifths, v + fourFifths, pt4, pt4N);
	sq.evalParams(u + fourFifths, v + twoFifths, pt5, pt5N);

	sq.AddTrianglePoints(btmLeft, topLeft, pt2, btmLeftN, topLeftN, pt2N);
	sq.AddTrianglePoints(btmLeft, pt2, pt3, btmLeftN, pt2N, pt3N);
	sq.AddTrianglePoints(btmLeft, pt3, btmRight, btmLeftN, pt3N, btmRightN);
	sq.AddTrianglePoints(pt3, pt4, pt5, pt3N, pt4N, pt5N);
	sq.AddTrianglePoints(pt2, pt3, pt4, pt2N, pt3N, pt4N);
	sq.AddTrianglePoints(pt3, pt5, btmRight, pt3N, pt5N, btmRightN);
	sq.AddTrianglePoints(btmRight, topRight, pt5, btmRightN, topRightN, pt5N);
	sq.AddTrianglePoints(pt5, topRight, pt4, pt5N, topRightN, pt4N);
	sq.AddTrianglePoints(topRight, pt4, topLeft, topRightN, pt4N, topLeftN);
	sq.AddTrianglePoints(topLeft, pt2, pt4, topLeftN, pt2N, pt4N);

	//TODO
	/*if (points.find(u + oneQuarterLength) == points.end()) {
	points[u + oneQuarterLength] = std::map<float, glm::vec3>();
	}
	points[u + oneQuarterLength][v + threeQuarterLength] = pt2;

	if (points.find(u + halfSquareLength) == points.end()) {
	points[u + halfSquareLength] = std::map<float, glm::vec3>();
	}
	points[u + halfSquareLength][v + oneQuarterLength] = pt3;

	if (points.find(u + threeQuarterLength) == points.end()) {
	points[u + threeQuarterLength] = std::map<float, glm::vec3>();
	}
	points[u + threeQuarterLength][v + threeQuarterLength] = pt4;
	fives++;
	//std::cout << "5" << std::endl;
	break;

	case 6:
	//btmLeft = pt1, (.25, .25) = pt2, (.25, .75) = pt3, (.5, .5) = pt4, (.75, .25) = pt5, (.75, .75) = pt6
	//Triangles:    t1 = pt1, topLeft, pt2
	//				t2 = pt1, pt2, pt5
	//				t3 = pt1, pt5, btmRight
	//				t4 = pt2, pt3, topLeft
	//				t5 = pt2, pt5, pt4
	//				t6 = pt2, pt4, pt3
	//				t7 = btmRight, pt6, pt5
	//				t8 = btmRight, pt6, topRight
	//				t9 = pt4, pt5, pt6
	//				t10 = pt3, pt4, pt6
	//				t11 = topLeft, pt3, topRight
	//				t12 = pt3, pt6, topRight

	sq.evalParams(u + oneQuarterLength, v + oneQuarterLength, pt2, pt2N);
	sq.evalParams(u + oneQuarterLength, v + threeQuarterLength, pt3, pt3N);
	sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt4, pt4N);
	sq.evalParams(u + threeQuarterLength, v + oneQuarterLength, pt5, pt5N);
	sq.evalParams(u + threeQuarterLength, v + threeQuarterLength, pt6, pt6N);

	sq.AddTrianglePoints(btmLeft, topLeft, pt2, btmLeftN, topLeftN, pt2N);
	sq.AddTrianglePoints(btmLeft, pt2, pt5, btmLeftN, pt2N, pt5N);
	sq.AddTrianglePoints(btmLeft, pt5, btmRight, btmLeftN, pt5N, btmRightN);
	sq.AddTrianglePoints(pt2, pt3, topLeft, pt2N, pt3N, topLeftN);
	sq.AddTrianglePoints(pt2, pt5, pt4, pt2N, pt5N, pt4N);
	sq.AddTrianglePoints(pt2, pt4, pt3, pt2N, pt4N, pt3N);
	sq.AddTrianglePoints(btmRight, pt6, pt5, btmRightN, pt6N, pt5N);
	sq.AddTrianglePoints(btmRight, pt6, topRight, btmRightN, pt6N, topRightN);
	sq.AddTrianglePoints(pt4, pt5, pt6, pt4N, pt5N, pt6N);
	sq.AddTrianglePoints(pt3, pt4, pt6, pt3N, pt4N, pt6N);
	sq.AddTrianglePoints(topLeft, pt3, topRight, topLeftN, pt3N, topRightN);
	sq.AddTrianglePoints(pt3, pt6, topRight, pt3N, pt6N, topRightN);

	sixes++;
	//std::cout << "6" << std::endl;
	break;

	case 7:

	sevens++;
	break;
	case 8:
	eights++;
	//std::cout << "uh oh" << std::endl;
	break;

	case 9:
	nines++;
	break;
	default:
	tenPlus++;
	//std::cout << "default........." << m << std::endl;
	break;
	}
	}
	}

	std::cout << zeroes << std::endl;
	std::cout << ones << std::endl;
	std::cout << twos << std::endl;
	std::cout << threes << std::endl;
	std::cout << fours << std::endl;
	std::cout << fives << std::endl;
	std::cout << sixes << std::endl;
	std::cout << sevens << std::endl;
	std::cout << eights << std::endl;
	std::cout << nines << std::endl;
	std::cout << tenPlus << std::endl << std::endl;

	//Save max radii to be used in bounding sphere tests:
	float max = (sq.a1 > sq.a2 ? sq.a1 : sq.a2);
	max = (max > sq.a3 ? max : sq.a3);
	sq.boundingSphereRadius = max; //TODO consider adding a buffer? like +.1f just in case objects are really close?

	sq.points = Points(points);
	}

	static void uniformAreaPoints2(Superquadric &sq) {
	const double PI = glm::pi<double>();

	double uLowerBound, uUpperBound, vLowerBound, vUpperBound;

	uLowerBound = -PI / 2.0;
	uUpperBound = PI / 2.0;
	vLowerBound = -PI;
	vUpperBound = PI;

	double dividend = 16.0;

	//Discretization of the domain
	double squareLength = PI / dividend; //Step size
	double halfSquareLength = squareLength / 2.0;

	std::vector<std::vector<double>> squareAreas;
	double u, v, uCenter, vCenter, area;
	double totalArea = 0.0;

	//Each discretized pt, (u,v), represents the bottom left vertex of a square in the domain
	// Calculates the surface area at center of each square (u+halfSquareLength, v+halfSquareLength),
	// And finds the total area
	for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	std::vector<double> curUsquares;
	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	uCenter = u + halfSquareLength;
	vCenter = v + halfSquareLength;
	area = SuperEllipsoid::SurfaceArea(sq, u, v);
	if (isnan(area)) {
	//std::cout << " whaaaa how??" << std::endl;
	area = 0.0;
	}
	totalArea += area;
	curUsquares.push_back(area);
	}
	squareAreas.push_back(curUsquares);
	}

	int zeroes, ones, twos, threes, fours, fives, sixes, sevens, eights, nines, tenPlus;
	zeroes = ones = twos = threes = fours = fives = sixes = sevens = eights = nines = tenPlus = 0;

	int avgPtsPerSquare = 3;
	int numPoints = (int)(2.0*dividend) * (int)(dividend)* avgPtsPerSquare; // Dividend*2 # of v segments, Dividend # of u segments, gives (dividend^2 / 2) squares, * avgPtsPerSquare


	//To generate numPoints number of points, let every square contain M = numPoints * (a / totalArea),
	// where a = the area of each square (surface area evaluated at center)
	// Generates the m points roughly equally spaced in each square
	int uIndex, vIndex;
	uIndex = 0;
	double pointsToAreaRatio = numPoints / totalArea;

	glm::dvec3 center, centerN, pt2, pt3, pt4, pt5, pt6, pt7, pt8, pt9, pt10, pt11, pt12, pt13, pt14, pt15, pt16, pt17, pt18,
	pt2N, pt3N, pt4N, pt5N, pt6N, pt7N, pt8N, pt9N, pt10N, pt11N, pt12N, pt13N, pt14N, pt15N, pt16N, pt17N, pt18N;

	glm::dvec3 pt19, pt20, pt21, pt22, pt19N, pt20N, pt21N, pt22N;

	//Pt offsets from btmLeft
	double thirdLength, twoThirdsLength, oneQuarterLength, threeQuarterLength;
	thirdLength = (1.0f / 3.0f) * squareLength;
	twoThirdsLength = (thirdLength * 2.0f);
	oneQuarterLength = .25f * squareLength;
	threeQuarterLength = oneQuarterLength * 3.0f;

	double oneFifthLength, twoFifths, threeFifths, fourFifths;
	oneFifthLength = (.2f * squareLength);
	twoFifths = oneFifthLength * 2.0f;
	threeFifths = oneFifthLength * 3.0f;
	fourFifths = oneFifthLength * 4.0f;

	std::map<float, std::map<float, glm::vec3>> points;

	int numTris = 0;
	for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	std::vector<double> curUsquares = squareAreas[uIndex++];
	vIndex = 0;
	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	double curSquareArea = curUsquares[vIndex++];

	glm::dvec3 btmLeft, btmLeftN, topRight, topRightN, topLeft, topLeftN, btmRight, btmRightN;
	sq.evalParams(u, v, btmLeft, btmLeftN);
	sq.evalParams(u + squareLength, v + squareLength, topRight, topRightN);
	sq.evalParams(u, v + squareLength, topLeft, topLeftN);
	sq.evalParams(u + squareLength, v, btmRight, btmRightN);


	if (points.find(u) == points.end()) {
	points[u] = std::map<float, glm::vec3>();
	}
	points[u][v] = btmLeft;
	points[u][v + squareLength] = topLeft;

	if (points.find(u + squareLength) == points.end()) {
	points[u + squareLength] = std::map<float, glm::vec3>();
	}
	points[u + squareLength][v] = btmRight;
	points[u + squareLength][v + squareLength] = topRight;

	//All squares will have at least 1 pt, the bottom left vertex of the square
	// To form triangles, each square will also utilize the bottom-left vertices of the
	// above, right, and above-right squares
	int m = round(pointsToAreaRatio * curSquareArea);

	//m = (m > 1 ? m : 1);

	//Based on the amount of points for this square, try to roughly equally space
	// them between the 4 corners of the square
	switch (m) {
	case 0:
	//Use whole square
	sq.AddSquarePoints(btmLeft, btmRight, topRight, topLeft, btmLeftN, btmRightN, topRightN, topLeftN);
	zeroes++;
	break;

	case 1:
	case 2:
	//Divide the square in half vertically

	//bottom midpoint
	sq.evalParams(u + halfSquareLength, v, pt2, pt2N);

	//top midpoint:
	sq.evalParams(u + halfSquareLength, v + squareLength, pt3, pt3N);

	sq.AddSquarePoints(btmLeft, pt2, pt3, topLeft, btmLeftN, pt2N, pt3N, topLeftN);
	sq.AddSquarePoints(pt2, btmRight, topRight, pt3, pt2N, btmRightN, topRightN, pt3N);

	ones++;
	break;

	case 3:
	case 4:
	//Divide square into 4 squares by dividing in half vertically, then horizontally

	//bottom midpoint
	sq.evalParams(u + halfSquareLength, v, pt2, pt2N);

	//top midpoint:
	sq.evalParams(u + halfSquareLength, v + squareLength, pt3, pt3N);

	//Left midpoint
	sq.evalParams(u, v + halfSquareLength, pt4, pt4N);

	//Right midpoint
	sq.evalParams(u + squareLength, v + halfSquareLength, pt5, pt5N);

	//center point
	sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt6, pt6N);

	sq.AddSquarePoints(btmLeft, pt2, pt6, pt4, btmLeftN, pt2N, pt6N, pt4N);
	sq.AddSquarePoints(pt4, pt6, pt3, topLeft, pt4N, pt6N, pt3N, topLeftN);
	sq.AddSquarePoints(pt2, btmRight, pt5, pt6, pt2N, btmRightN, pt5N, pt6N);
	sq.AddSquarePoints(pt6, pt5, topRight, pt3, pt6N, pt5N, topRightN, pt3N);

	twos++;
	//std::cout << "2" << std::endl;
	break;

	case 5:
	case 6:
	//One vertical line, two equally spaced horizontal lines divide the square into 6 squares

	//bottom midpoint
	sq.evalParams(u + halfSquareLength, v, pt2, pt2N);

	//top midpoint:
	sq.evalParams(u + halfSquareLength, v + squareLength, pt3, pt3N);

	//upper left midpoint
	sq.evalParams(u, v + twoThirdsLength, pt4, pt4N);

	//lower left midpoint
	sq.evalParams(u, v + thirdLength, pt5, pt5N);

	//upper right midpoint
	sq.evalParams(u + squareLength, v + twoThirdsLength, pt6, pt6N);

	//lower right midpoint
	sq.evalParams(u + squareLength, v + thirdLength, pt7, pt7N);

	//upper center point
	sq.evalParams(u + halfSquareLength, v + twoThirdsLength, pt8, pt8N);

	// lower center point
	sq.evalParams(u + halfSquareLength, v + thirdLength, pt9, pt9N);

	//Left half squares
	sq.AddSquarePoints(btmLeft, pt2, pt9, pt5, btmLeftN, pt2N, pt9N, pt5N);
	sq.AddSquarePoints(pt5, pt9, pt8, pt4, pt5N, pt9N, pt8N, pt4N);
	sq.AddSquarePoints(pt4, pt8, pt3, topLeft, pt4N, pt8N, pt3N, topLeftN);

	//Right half squares
	sq.AddSquarePoints(pt2, btmRight, pt7, pt9, pt2N, btmRightN, pt7N, pt9N);
	sq.AddSquarePoints(pt9, pt7, pt6, pt8, pt9N, pt7N, pt6N, pt8N);
	sq.AddSquarePoints(pt8, pt6, topRight, pt3, pt8N, pt6N, topRightN, pt3N);

	threes++;
	//std::cout << "3" << std::endl;
	break;

	case 7:
	case 8:
	//Two vertical and horizontal lines split the square into 9 squares

	//Top left
	sq.evalParams(u + thirdLength, v + squareLength, pt2, pt2N);

	//Top Right
	sq.evalParams(u + twoThirdsLength, v + squareLength, pt3, pt3N);

	//left upper
	sq.evalParams(u, v + twoThirdsLength, pt4, pt4N);

	//center upper left
	sq.evalParams(u + thirdLength, v + twoThirdsLength, pt5, pt5N);

	//center upper right
	sq.evalParams(u + twoThirdsLength, v + twoThirdsLength, pt6, pt6N);

	//right upper
	sq.evalParams(u + squareLength, v + twoThirdsLength, pt7, pt7N);

	//left lower
	sq.evalParams(u, v + thirdLength, pt8, pt8N);

	// center lower left
	sq.evalParams(u + thirdLength, v + thirdLength, pt9, pt9N);

	//center lower right
	sq.evalParams(u + twoThirdsLength, v + thirdLength, pt10, pt10N);

	//right lower
	sq.evalParams(u + squareLength, v + thirdLength, pt11, pt11N);

	//bottom left
	sq.evalParams(u + thirdLength, v, pt12, pt12N);

	//bottom right
	sq.evalParams(u + twoThirdsLength, v, pt13, pt13N);

	//Left column squares
	sq.AddSquarePoints(btmLeft, pt12, pt9, pt8, btmLeftN, pt12N, pt9N, pt8N);
	sq.AddSquarePoints(pt8, pt9, pt5, pt4, pt8N, pt9N, pt5N, pt4N);
	sq.AddSquarePoints(pt4, pt5, pt2, topLeft, pt4N, pt5N, pt2N, topLeftN);

	//Center column:
	sq.AddSquarePoints(pt12, pt13, pt10, pt9, pt12N, pt13N, pt10N, pt9N);
	sq.AddSquarePoints(pt9, pt10, pt6, pt5, pt9N, pt10N, pt6N, pt5N);
	sq.AddSquarePoints(pt5, pt6, pt3, pt2, pt5N, pt6N, pt3N, pt2N);

	//Right column
	sq.AddSquarePoints(pt13, btmRight, pt11, pt10, pt13N, btmRightN, pt11N, pt10N);
	sq.AddSquarePoints(pt10, pt11, pt7, pt6, pt10N, pt11N, pt7N, pt6N);
	sq.AddSquarePoints(pt6, pt7, topRight, pt3, pt6N, pt7N, topRightN, pt3N);

	fours++;
	//std::cout << "4" << std::endl;
	break;

	case 9:
	case 10:
	//Three vertical, 2 horizontal dividing lines, 12 squares

	//top row, left to right
	sq.evalParams(u + oneQuarterLength, v + squareLength, pt2, pt2N);
	sq.evalParams(u + halfSquareLength, v + squareLength, pt3, pt3N);
	sq.evalParams(u + threeQuarterLength, v + squareLength, pt4, pt4N);

	//middle top row
	sq.evalParams(u, v + twoThirdsLength, pt5, pt5N);
	sq.evalParams(u + oneQuarterLength, v + twoThirdsLength, pt6, pt6N);
	sq.evalParams(u + halfSquareLength, v + twoThirdsLength, pt7, pt7N);
	sq.evalParams(u + threeQuarterLength, v + twoThirdsLength, pt8, pt8N);
	sq.evalParams(u + squareLength, v + twoThirdsLength, pt9, pt9N);

	//middle lower row
	sq.evalParams(u, v + thirdLength, pt10, pt10N);
	sq.evalParams(u + oneQuarterLength, v + thirdLength, pt11, pt11N);
	sq.evalParams(u + halfSquareLength, v + thirdLength, pt12, pt12N);
	sq.evalParams(u + threeQuarterLength, v + thirdLength, pt13, pt13N);
	sq.evalParams(u + squareLength, v + thirdLength, pt14, pt14N);

	//bottom row
	sq.evalParams(u + oneQuarterLength, v, pt15, pt15N);
	sq.evalParams(u + halfSquareLength, v, pt16, pt16N);
	sq.evalParams(u + threeQuarterLength, v, pt17, pt17N);

	//Leftmost column of squares:
	sq.AddSquarePoints(pt5, pt6, pt2, topLeft, pt5N, pt6N, pt2N, topLeftN);
	sq.AddSquarePoints(pt10, pt11, pt6, pt5, pt10N, pt11N, pt6N, pt5N);
	sq.AddSquarePoints(btmLeft, pt15, pt11, pt10, btmLeftN, pt15N, pt11N, pt10N);

	//2nd leftmost column
	sq.AddSquarePoints(pt6, pt7, pt3, pt2, pt6N, pt7N, pt3N, pt2N);
	sq.AddSquarePoints(pt11, pt12, pt7, pt6, pt11N, pt12N, pt7N, pt6N);
	sq.AddSquarePoints(pt15, pt16, pt12, pt11, pt15N, pt16N, pt12N, pt11N);

	//2nd rightmost col
	sq.AddSquarePoints(pt7, pt8, pt4, pt3, pt7N, pt8N, pt4N, pt3N);
	sq.AddSquarePoints(pt12, pt13, pt8, pt7, pt12N, pt13N, pt8N, pt7N);
	sq.AddSquarePoints(pt16, pt17, pt13, pt12, pt16N, pt17N, pt13N, pt12N);

	//Rightmost
	sq.AddSquarePoints(pt8, pt9, topRight, pt4, pt8N, pt9N, topRightN, pt4N);
	sq.AddSquarePoints(pt13, pt14, pt9, pt8, pt13N, pt14N, pt9N, pt8N);
	sq.AddSquarePoints(pt17, btmRight, pt14, pt13, pt17N, btmRightN, pt14N, pt13N);

	fives++;
	//std::cout << "5" << std::endl;
	break;

	default:
	//3 horizontal & vertical lines, 16 total squares

	//top row, left to right
	sq.evalParams(u + oneQuarterLength, v + squareLength, pt2, pt2N);
	sq.evalParams(u + halfSquareLength, v + squareLength, pt3, pt3N);
	sq.evalParams(u + threeQuarterLength, v + squareLength, pt4, pt4N);

	//2nd row from top
	sq.evalParams(u, v + threeQuarterLength, pt5, pt5N);
	sq.evalParams(u + oneQuarterLength, v + threeQuarterLength, pt6, pt6N);
	sq.evalParams(u + halfSquareLength, v + threeQuarterLength, pt7, pt7N);
	sq.evalParams(u + threeQuarterLength, v + threeQuarterLength, pt8, pt8N);
	sq.evalParams(u + squareLength, v + threeQuarterLength, pt9, pt9N);

	//middle row
	sq.evalParams(u, v + halfSquareLength, pt10, pt10N);
	sq.evalParams(u + oneQuarterLength, v + halfSquareLength, pt11, pt11N);
	sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt12, pt12N);
	sq.evalParams(u + threeQuarterLength, v + halfSquareLength, pt13, pt13N);
	sq.evalParams(u + squareLength, v + halfSquareLength, pt14, pt14N);

	//2nd from bottom row
	sq.evalParams(u, v + oneQuarterLength, pt15, pt15N);
	sq.evalParams(u + oneQuarterLength, v + oneQuarterLength, pt16, pt16N);
	sq.evalParams(u + halfSquareLength, v + oneQuarterLength, pt17, pt17N);
	sq.evalParams(u + threeQuarterLength, v + oneQuarterLength, pt18, pt18N);
	sq.evalParams(u + squareLength, v + oneQuarterLength, pt19, pt19N);

	//bottom row
	sq.evalParams(u + oneQuarterLength, v, pt20, pt20N);
	sq.evalParams(u + halfSquareLength, v, pt21, pt21N);
	sq.evalParams(u + threeQuarterLength, v, pt22, pt22N);

	//Leftmost column of squares:
	sq.AddSquarePoints(pt5, pt6, pt2, topLeft, pt5N, pt6N, pt2N, topLeftN);
	sq.AddSquarePoints(pt10, pt11, pt6, pt5, pt10N, pt11N, pt6N, pt5N);
	sq.AddSquarePoints(pt15, pt16, pt11, pt10, pt15N, pt16N, pt11N, pt10N);
	sq.AddSquarePoints(btmLeft, pt20, pt16, pt15, btmLeftN, pt20N, pt16N, pt15N);

	//2nd leftmost column
	sq.AddSquarePoints(pt6, pt7, pt3, pt2, pt6N, pt7N, pt3N, pt2N);
	sq.AddSquarePoints(pt11, pt12, pt7, pt6, pt11N, pt12N, pt7N, pt6N);
	sq.AddSquarePoints(pt16, pt17, pt12, pt11, pt16N, pt17N, pt12N, pt11N);
	sq.AddSquarePoints(pt20, pt21, pt17, pt16, pt20N, pt21N, pt17N, pt16N);

	//2nd rightmost col
	sq.AddSquarePoints(pt7, pt8, pt4, pt3, pt7N, pt8N, pt4N, pt3N);
	sq.AddSquarePoints(pt12, pt13, pt8, pt7, pt12N, pt13N, pt8N, pt7N);
	sq.AddSquarePoints(pt17, pt18, pt13, pt12, pt17N, pt18N, pt13N, pt12N);
	sq.AddSquarePoints(pt21, pt22, pt18, pt17, pt21N, pt22N, pt18N, pt17N);

	//Rightmost
	sq.AddSquarePoints(pt8, pt9, topRight, pt4, pt8N, pt9N, topRightN, pt4N);
	sq.AddSquarePoints(pt13, pt14, pt9, pt8, pt13N, pt14N, pt9N, pt8N);
	sq.AddSquarePoints(pt18, pt19, pt14, pt13, pt18N, pt19N, pt14N, pt13N);
	sq.AddSquarePoints(pt22, btmRight, pt19, pt18, pt22N, btmRightN, pt19N, pt18N);

	sixes++;
	//std::cout << "6" << std::endl;

	if (m > 12) {
	tenPlus++;
	std::cout << "default........." << m << std::endl;
	}
	break;
	}
	}
	}

	std::cout << zeroes << std::endl;
	std::cout << ones << std::endl;
	std::cout << twos << std::endl;
	std::cout << threes << std::endl;
	std::cout << fours << std::endl;
	std::cout << fives << std::endl;
	std::cout << sixes << std::endl;
	std::cout << sevens << std::endl;
	std::cout << eights << std::endl;
	std::cout << nines << std::endl;
	std::cout << tenPlus << std::endl;

	//Find average area:
	float totalAreaa = 0.0f;
	for (int i = 0; i < sq.allTriangleAreas.size(); i++) {
	totalAreaa += sq.allTriangleAreas[i];
	}
	float average = totalAreaa / (float) sq.allTriangleAreas.size();

	//Find variance:
	float variance = 0.0f;
	for (int i = 0; i < sq.allTriangleAreas.size(); i++) {
	variance += powf(sq.allTriangleAreas[i] - average, 2.0f) / (double)(sq.allTriangleAreas.size() - 1);
	}
	std::cout << "Average triangle area: " << average << std::endl;
	std::cout <<  "Area variance" << variance << std::endl << std::endl;

	//Save max radii to be used in bounding sphere tests:
	float max = (sq.a1 > sq.a2 ? sq.a1 : sq.a2);
	max = (max > sq.a3 ? max : sq.a3);
	sq.boundingSphereRadius = max + .1f;

	sq.points = Points(points);
	}
	*/
	/*
	static void uniformAreaPoints3(Superquadric &sq) {
	const double PI = glm::pi<double>();

	double uLowerBound, uUpperBound, vLowerBound, vUpperBound;

	uLowerBound = -PI / 2.0;
	uUpperBound = PI / 2.0;
	vLowerBound = -PI;
	vUpperBound = PI;

	double dividend = 6.0;

	//Discretization of the domain
	double squareLength = PI / dividend; //Step size
	float halfSquareLength = squareLength / 2.0;

	std::vector<std::vector<double>> squareAreas;
	double uCenter, vCenter, area;
	double totalArea = 0.0;

	float u, v;

	//Each discretized pt, (u,v), represents the bottom left vertex of a square in the domain
	// Calculates the surface area at center of each square (u+halfSquareLength, v+halfSquareLength),
	// And finds the total area
	for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	std::vector<double> curUsquares;
	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	uCenter = u + halfSquareLength;
	vCenter = v + halfSquareLength;
	area = SuperEllipsoid::SurfaceArea(sq, u, v);
	if (isnan(area)) {
	//std::cout << " whaaaa how??" << std::endl;
	area = 0.0;
	}
	totalArea += area;
	curUsquares.push_back(area);
	}
	squareAreas.push_back(curUsquares);
	}

	int zeroes, ones, twos, threes, fours, fives, sixes, sevens, eights, nines, tenPlus;
	zeroes = ones = twos = threes = fours = fives = sixes = sevens = eights = nines = tenPlus = 0;

	int avgPtsPerSquare = 2;
	int numPoints = (int)(2.0*dividend) * (int)(dividend)* avgPtsPerSquare; // Dividend*2 # of v segments, Dividend # of u segments, gives (dividend^2 / 2) squares, * avgPtsPerSquare


	//To generate numPoints number of points, let every square contain M = numPoints * (a / totalArea),
	// where a = the area of each square (surface area evaluated at center)
	// Generates the m points roughly equally spaced in each square
	int uIndex, vIndex;
	uIndex = 0;
	double pointsToAreaRatio = numPoints / totalArea;

	glm::dvec3 center, centerN, pt2, pt3, pt4, pt5, pt6, pt7, pt8, pt9, pt10, pt11, pt12, pt13, pt14, pt15, pt16, pt17, pt18,
	pt2N, pt3N, pt4N, pt5N, pt6N, pt7N, pt8N, pt9N, pt10N, pt11N, pt12N, pt13N, pt14N, pt15N, pt16N, pt17N, pt18N;

	glm::dvec3 pt19, pt20, pt21, pt22, pt19N, pt20N, pt21N, pt22N;

	//Pt offsets from btmLeft
	float thirdLength, twoThirdsLength, oneQuarterLength, threeQuarterLength;
	thirdLength = (1.0f / 3.0f) * squareLength;
	twoThirdsLength = (thirdLength * 2.0f);
	oneQuarterLength = .25f * squareLength;
	threeQuarterLength = oneQuarterLength * 3.0f;

	float oneFifthLength, twoFifths, threeFifths, fourFifths;
	oneFifthLength = (.2f * squareLength);
	twoFifths = oneFifthLength * 2.0f;
	threeFifths = oneFifthLength * 3.0f;
	fourFifths = oneFifthLength * 4.0f;

	float point3Len, point7Len, point35Len, point65Len;
	point3Len = squareLength * .3;
	point7Len = .7 * squareLength;
	point35Len = .35 * squareLength;
	point65Len = .65 * squareLength;

	std::vector<ParamPoint> points;

	ParamPoint pp, p2, p3, p4, p5, p6, p7, p8, p9, p10;

	//sq.evalParams(-(float)PI / 2.0f, -(float)PI, pt2, pt2N);
	//sq.evalParams(0, 0.0f, pt3, pt3N);
	sq.evalParams(0, -PI/2.0f, pt4, pt4N);

	pp = { (float)PI / 2.0f, (float)0, pt4 };
	//p2 = { 0, 0.0f, pt3 };
	//p3 = { (float)PI / 2.0f, (float)PI / 2.0f, pt4 };
	points.push_back(pp);
	//points.push_back(p2);
	//points.push_back(p3);



	//int numTris = 0;
	//for (u = uLowerBound; u < uUpperBound; u += squareLength) {
	//	std::vector<double> curUsquares = squareAreas[uIndex++];
	//	vIndex = 0;
	//	for (v = vLowerBound; v < vUpperBound; v += squareLength) {
	//		double curSquareArea = curUsquares[vIndex++];

	//		glm::dvec3 btmLeft, btmLeftN, topRight, topRightN, topLeft, topLeftN, btmRight, btmRightN;
	//		sq.evalParams(u, v, btmLeft, btmLeftN);
	//		sq.evalParams(u + squareLength, v + squareLength, topRight, topRightN);
	//		sq.evalParams(u, v + squareLength, topLeft, topLeftN);
	//		sq.evalParams(u + squareLength, v, btmRight, btmRightN);

	//		//# of points for this square:
	//		int m = round(pointsToAreaRatio * curSquareArea);

	//		//m = (m > 1 ? m : 1);

	//		//Based on the amount of points for this square, try to roughly equally space
	//		// them between the 4 corners of the square
	//		switch (m) {
	//		case 0:
	//			//No points //TODO join 0-squares
	//			zeroes++;
	//			//break;

	//		case 1:
	//			sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt2, pt2N);
	//			pp = { u + halfSquareLength, v + halfSquareLength, pt2 };
	//			points.push_back(pp);
	//			ones++;
	//			break;

	//		case 2:
	//			sq.evalParams(u + thirdLength, v + thirdLength, pt2, pt2N);
	//			sq.evalParams(u + twoThirdsLength, v + twoThirdsLength, pt3, pt3N);
	//			pp = { u + halfSquareLength, v + halfSquareLength, pt2 };
	//			p2 = { u + thirdLength, v + twoThirdsLength, pt3 };
	//			points.push_back(pp);
	//			points.push_back(p2);
	//			twos++;
	//			break;

	//		case 3:
	//			sq.evalParams(u + point3Len, v + point3Len, pt2, pt2N);
	//			sq.evalParams(u + halfSquareLength, v + point7Len, pt3, pt3N);
	//			sq.evalParams(u + point7Len, v + point3Len, pt4, pt4N);
	//			pp = { u + point3Len, v + point3Len, pt2 };
	//			p2 = { u + halfSquareLength, v + point7Len, pt3 };
	//			p3 = { u + point7Len, v + point3Len, pt4 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			threes++;
	//			break;
	//		case 4:
	//			sq.evalParams(u + point3Len, v + point3Len, pt2, pt2N);
	//			sq.evalParams(u + point3Len, v + point7Len, pt3, pt3N);
	//			sq.evalParams(u + point7Len, v + point3Len, pt4, pt4N);
	//			sq.evalParams(u + point7Len, v + point7Len, pt5, pt5N);
	//			pp = { u + point3Len, v + point3Len, pt2 };
	//			p2 = { u + point3Len, v + point7Len, pt3 };
	//			p3 = { u + point7Len, v + point3Len, pt4 };
	//			p4 = { u + point7Len, v + point7Len, pt5 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			fours++;
	//			break;

	//		case 5:
	//			sq.evalParams(u + oneQuarterLength, v + oneQuarterLength, pt2, pt2N);
	//			sq.evalParams(u + oneQuarterLength, v + threeQuarterLength, pt3, pt3N);
	//			sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt4, pt4N);
	//			sq.evalParams(u + threeQuarterLength, v + threeQuarterLength, pt5, pt5N);
	//			sq.evalParams(u + threeQuarterLength, v + oneQuarterLength, pt6, pt6N);
	//			pp = { u + oneQuarterLength, v + oneQuarterLength, pt2 };
	//			p2 = { u + oneQuarterLength, v + threeQuarterLength, pt3 };
	//			p3 = { u + halfSquareLength, v + halfSquareLength, pt4 };
	//			p4 = { u + threeQuarterLength, v + threeQuarterLength, pt5 };
	//			p5 = { u + threeQuarterLength , v + oneQuarterLength, pt6 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			points.push_back(p5);
	//			fives++;
	//			break;
	//		case 6:
	//			
	//			(.2, .35)
	//			(.2, .65)
	//			(.5, .8)
	//			(.5,.2)
	//			(.8, .35)
	//			(.8, .65)
	//			
	//			sq.evalParams(u + oneFifthLength, v + point35Len, pt2, pt2N);
	//			sq.evalParams(u + oneFifthLength, v + point65Len, pt3, pt3N);
	//			sq.evalParams(u + halfSquareLength, v + fourFifths, pt4, pt4N);
	//			sq.evalParams(u + halfSquareLength, v + oneFifthLength, pt5, pt5N);
	//			sq.evalParams(u + fourFifths, v + point35Len, pt6, pt6N);
	//			sq.evalParams(u + fourFifths, v + point65Len, pt7, pt7N);
	//			pp = { u + oneFifthLength, v + point35Len, pt2 };
	//			p2 = { u + oneFifthLength, v + point65Len, pt3 };
	//			p3 = { u + halfSquareLength, v + fourFifths, pt4 };
	//			 p4 = { u + halfSquareLength, v + oneFifthLength, pt5 };
	//			 p5 = { u + fourFifths , v + point35Len, pt6 };
	//			p6 = { u + fourFifths, v + point65Len, pt7 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			points.push_back(p5);
	//			points.push_back(p6);
	//			sixes++;
	//			break;

	//		case 7:
	//			sq.evalParams(u + oneFifthLength, v + point35Len, pt2, pt2N);
	//			sq.evalParams(u + oneFifthLength, v + point65Len, pt3, pt3N);
	//			sq.evalParams(u + halfSquareLength, v + fourFifths, pt4, pt4N);
	//			sq.evalParams(u + halfSquareLength, v + oneFifthLength, pt5, pt5N);
	//			sq.evalParams(u + fourFifths, v + point35Len, pt6, pt6N);
	//			sq.evalParams(u + fourFifths, v + point65Len, pt7, pt7N);
	//			sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt8, pt8N);
	//			pp = { u + oneFifthLength, v + point35Len, pt2 };
	//			p2 = { u + oneFifthLength, v + point65Len, pt3 };
	//			p3 = { u + halfSquareLength, v + fourFifths, pt4 };
	//			p4 = { u + halfSquareLength, v + oneFifthLength, pt5 };
	//			p5 = { u + fourFifths , v + point35Len, pt6 };
	//			p6 = { u + fourFifths, v + point65Len, pt7 };
	//			p7 = { u + halfSquareLength, v + halfSquareLength, pt8 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			points.push_back(p5);
	//			points.push_back(p6);
	//			points.push_back(p7);
	//			sevens++;
	//			break;
	//		case 8:
	//			/*(.2, .35)
	//				(.2, .65)
	//				(.4, .8)
	//				(.4,.2)
	//				(.6, .8)
	//				(.6, .2)
	//				(.8, .35)
	//				(.8, .65)*/
	//			sq.evalParams(u + oneFifthLength, v + point35Len, pt2, pt2N);
	//			sq.evalParams(u + oneFifthLength, v + point65Len, pt3, pt3N);
	//			sq.evalParams(u + twoFifths, v + fourFifths, pt4, pt4N);
	//			sq.evalParams(u + twoFifths, v + oneFifthLength, pt5, pt5N);
	//			sq.evalParams(u + fourFifths, v + point35Len, pt6, pt6N);
	//			sq.evalParams(u + fourFifths, v + point65Len, pt7, pt7N);
	//			sq.evalParams(u + threeFifths, v + fourFifths, pt8, pt8N);
	//			sq.evalParams(u + threeFifths, v + oneFifthLength, pt9, pt9N);
	//			 pp = { u + oneFifthLength, v + point35Len, pt2 };
	//			 p2 = { u + oneFifthLength, v + point65Len, pt3 };
	//			 p3 = { u + twoFifths, v + fourFifths, pt4 };
	//			 p4 = { u + twoFifths, v + oneFifthLength, pt5 };
	//			 p5 = { u + fourFifths , v + point35Len, pt6 };
	//			 p6 = { u + fourFifths, v + point65Len, pt7 };
	//			 p7 = { u + threeFifths, v + fourFifths, pt8 };
	//			 p8 = { u + threeFifths, v + oneFifthLength, pt9 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			points.push_back(p5);
	//			points.push_back(p6);
	//			points.push_back(p7);
	//			points.push_back(p8);
	//			eights++;
	//			break;

	//		case 9:
	//			sq.evalParams(u + oneFifthLength, v + point35Len, pt2, pt2N);
	//			sq.evalParams(u + oneFifthLength, v + point65Len, pt3, pt3N);
	//			sq.evalParams(u + twoFifths, v + fourFifths, pt4, pt4N);
	//			sq.evalParams(u + twoFifths, v + oneFifthLength, pt5, pt5N);
	//			sq.evalParams(u + fourFifths, v + point35Len, pt6, pt6N);
	//			sq.evalParams(u + fourFifths, v + point65Len, pt7, pt7N);
	//			sq.evalParams(u + threeFifths, v + fourFifths, pt8, pt8N);
	//			sq.evalParams(u + threeFifths, v + oneFifthLength, pt9, pt9N);
	//			sq.evalParams(u + halfSquareLength, v + halfSquareLength, pt10, pt10N);
	//			 pp = { u + oneFifthLength, v + point35Len, pt2 };
	//			p2 = { u + oneFifthLength, v + point65Len, pt3 };
	//			p3 = { u + twoFifths, v + fourFifths, pt4 };
	//			 p4 = { u + twoFifths, v + oneFifthLength, pt5 };
	//			 p5 = { u + fourFifths , v + point35Len, pt6 };
	//			 p6 = { u + fourFifths, v + point65Len, pt7 };
	//			 p7 = { u + threeFifths, v + fourFifths, pt8 };
	//			 p8 = { u + threeFifths, v + oneFifthLength, pt9 };
	//			 p9 = { u + halfSquareLength, v + halfSquareLength, pt10 };

	//			points.push_back(pp);
	//			points.push_back(p2);
	//			points.push_back(p3);
	//			points.push_back(p4);
	//			points.push_back(p5);
	//			points.push_back(p6);
	//			points.push_back(p7);
	//			points.push_back(p8);
	//			points.push_back(p9);
	//			nines++;
	//			break;

	//		default:
	//			std::cout << " More than 9" << std::endl;
	//			tenPlus++;
	//			break;
	//		}
	//	}
	//}

	//std::cout << zeroes << std::endl;
	//std::cout << ones << std::endl;
	//std::cout << twos << std::endl;
	//std::cout << threes << std::endl;
	//std::cout << fours << std::endl;
	//std::cout << fives << std::endl;
	//std::cout << sixes << std::endl;
	//std::cout << sevens << std::endl;
	//std::cout << eights << std::endl;
	//std::cout << nines << std::endl;
	//std::cout << tenPlus << std::endl;

	////Find average area:
	//float totalAreaa = 0.0f;
	//for (int i = 0; i < sq.allTriangleAreas.size(); i++) {
	//	totalAreaa += sq.allTriangleAreas[i];
	//}
	//float average = totalAreaa / (float)sq.allTriangleAreas.size();

	////Find variance: 
	//float variance = 0.0f;
	//for (int i = 0; i < sq.allTriangleAreas.size(); i++) {
	//	variance += powf(sq.allTriangleAreas[i] - average, 2.0f) / (double)(sq.allTriangleAreas.size() - 1);
	//}
	//std::cout << "Average triangle area: " << average << std::endl;
	//std::cout << "Area variance" << variance << std::endl << std::endl;

	//sq.points.insert(sq.points.end(), points.begin(), points.end());
	//sq.points = Points(points);
	//}



	// Transforming the U and V parameters this way provides more uniformly distributed points on 
	// the superquadric surface
	static void transformParams(float &u, float &v, float e1, float e2) {
		float tanU = tan(u);
		u = atan(MathUtils::sgnf(tanU) * powf(MathUtils::abs(tanU), (1.0f / e1)));

		float tanV = tan(v);
		float offset = 0.0f;
		float pi = glm::pi<float>();
		float pi2 = pi / 2.0f;
		if (v > pi2) {
			offset = pi;
		}
		else if (v < -pi2) {
			offset = -pi;
		}
		v = atan(MathUtils::sgnf(tanV) * powf(MathUtils::abs(tanV), 1.0f / e2)) + offset;
	}

	/*
	* Generates a solid ellipsoid using the parameters from sq
	*/
	static void sqSolidEllipsoid(Superquadric &sq) {
		float U, dU, V, dV;
		//float S, dS, T, dT;
		int X, Y; 	/* for looping */
		float x, y, z;
		float nx, ny, nz;
		float v1x, v1y, v1z, n1x, n1y, n1z;
		float v3x, v3y, v3z, n3x, n3y, n3z;

		//Used to determine bounding sphere radius
		float max;

		/* Calculate delta variables */
		dU = (float)(sq.u2 - sq.u1) / (float)sq.u_segs;
		dV = (float)(sq.v2 - sq.v1) / (float)sq.v_segs;
		//dS = (float)(sq.s2 - sq.s1) / (float)sq.u_segs;
		//dT = (float)(sq.t2 - sq.t1) / (float)sq.v_segs;

		/* Initialize variables for loop */
		U = sq.u1;
		//S = sq.s1;
		std::map<float, std::map<float, glm::vec3>> points;
		for (Y = 0; Y < sq.u_segs; Y++) {
			/* Initialize variables for loop */
			V = sq.v1;
			//T = sq.t1;
			for (X = 0; X < sq.v_segs; X++) {
				/* VERTEX #1 */
				sqEllipsoid(sq.a1, sq.a2, sq.a3, U, V, sq.e1, sq.e2, &x, &y, &z, &nx, &ny, &nz);

				if (points.find(U) == points.end()) {
					points[U] = std::map<float, glm::vec3>();
				}
				points[U][V] = glm::vec3(x, y, z);
				if (sq.isNonsingular(U, V)) {
					sq.nonsingulars.push_back(glm::vec3(U, V, 0.0f));
				}

				updateMax(x, y, z, max);
				v1x = x;
				v1y = y;
				v1z = z;
				n1x = nx;
				n1y = ny;
				n1z = nz;
				sq.vertices.push_back(x);
				sq.vertices.push_back(y);
				sq.vertices.push_back(z);
				sq.vertices.push_back(nx);
				sq.vertices.push_back(ny);
				sq.vertices.push_back(nz);

				/* VERTEX #2 */
				sqEllipsoid(sq.a1, sq.a2, sq.a3, U + dU, V, sq.e1, sq.e2, &x, &y, &z, &nx, &ny, &nz);

				if (points.find(U + dU) == points.end()) {
					points[U + dU] = std::map<float, glm::vec3>();
				}
				points[U + dU][V] = glm::vec3(x, y, z);
				if (sq.isNonsingular(U + dU, V)) {
					sq.nonsingulars.push_back(glm::vec3(U + dU, V, 0.0f));
				}

				updateMax(x, y, z, max);
				sq.vertices.push_back(x);
				sq.vertices.push_back(y);
				sq.vertices.push_back(z);
				sq.vertices.push_back(nx);
				sq.vertices.push_back(ny);
				sq.vertices.push_back(nz);

				/* VERTEX #3 */
				sqEllipsoid(sq.a1, sq.a2, sq.a3, U + dU, V + dV, sq.e1, sq.e2, &x, &y, &z, &nx, &ny, &nz);

				points[U + dU][V + dV] = glm::vec3(x, y, z);
				if (sq.isNonsingular(U + dU, V + dV)) {
					sq.nonsingulars.push_back(glm::vec3(U + dU, V + dV, 0.0f));
				}

				updateMax(x, y, z, max);
				v3x = x;
				v3y = y;
				v3z = z;
				n3x = nx;
				n3y = ny;
				n3z = nz;
				sq.vertices.push_back(x);
				sq.vertices.push_back(y);
				sq.vertices.push_back(z);
				sq.vertices.push_back(nx);
				sq.vertices.push_back(ny);
				sq.vertices.push_back(nz);

				/* VERTEX #4 */
				sqEllipsoid(sq.a1, sq.a2, sq.a3, U, V + dV, sq.e1, sq.e2, &x, &y, &z, &nx, &ny, &nz);

				points[U][V + dV] = glm::vec3(x, y, z);
				if (sq.isNonsingular(U, V + dV)) {
					sq.nonsingulars.push_back(glm::vec3(U, V + dV, 0.0f));
				}

				updateMax(x, y, z, max);
				sq.vertices.push_back(v1x);
				sq.vertices.push_back(v1y);
				sq.vertices.push_back(v1z);
				sq.vertices.push_back(n1x);
				sq.vertices.push_back(n1y);
				sq.vertices.push_back(n1z);

				sq.vertices.push_back(v3x);
				sq.vertices.push_back(v3y);
				sq.vertices.push_back(v3z);
				sq.vertices.push_back(n3x);
				sq.vertices.push_back(n3y);
				sq.vertices.push_back(n3z);
				sq.vertices.push_back(x);
				sq.vertices.push_back(y);
				sq.vertices.push_back(z);
				sq.vertices.push_back(nx);
				sq.vertices.push_back(ny);
				sq.vertices.push_back(nz);

				/* Update variables for next loop */
				V += dV;
				//T += dT;
			}
			/* Update variables for next loop */
			//S += dS;
			U += dU;
		}

		sq.boundingSphereRadius = max + sq.BoundingSphereBuffer;
		//sq.points = Points(points);

		/*if (sq.nonsingulars.size() > 0) {
		std::cout << "found nonsingulars" << std::endl;
		}*/
	}

	static void updateMax(float x, float y, float z, float &max) {
		float dist = sqrtf(x*x + y*y + z*z);
		if (dist > max) {
			max = dist;
		}
	}

	/* sqEllipsoidInsideOut ( sq, x, y, z )
	*
	* Tests to see if point P is inside the SuperQuadric sq.
	* Returns 1 if on the surface, > 1 if outside the surface, or
	* < 1 if inside the surface
	*/
	static float sqEllipsoidInsideOut(Superquadric &sq, float x, float y, float z) {
		float result;
		result = powf(powf(x / sq.a1, 2 / sq.e1) +
			powf(y / sq.a2, 2 / sq.e1), sq.e1 / sq.e2) +
			powf(z / sq.a3, 2 / sq.e2);
		return result;
	}
};

//float fx(float x, float y, float z) {
//	float d, twoOverE2, first, second;
//	d = e2 / e1;
//	twoOverE2 = 2.0f / e2;
//	first = d * powf((powf(((1.0f / a1) * x), twoOverE2) + powf(((1.0f / a2) * y), twoOverE2)), d - 1.0f);
//	second = powf(1.0f / a1, twoOverE2) * (twoOverE2 * powf(x, twoOverE2 - 1.0f));
//	return first * second;
//}

//float fy(float x, float y, float z) {
//	float d, twoOverE2, first, second;
//	d = e2 / e1;
//	twoOverE2 = 2.0f / e2;
//	first = d * powf((powf(((1.0f / a1) * x), twoOverE2) + powf(((1.0f / a2) * y), twoOverE2)), d - 1.0f);
//	second = powf(1.0f / a2, twoOverE2) * (twoOverE2 * powf(y, twoOverE2 - 1.0f));
//	return first * second;
//}

//float fz(float x, float y, float z) {
//	float twoOverE1;
//	twoOverE1 = 2.0f / e1;
//	return powf((1.0f / a3), twoOverE1) * twoOverE1 * powf(z, twoOverE1 - 1.0f);
//}

//glm::vec3 Gradient(float x, float y, float z) {
//	float d, twoOverE2, twoOverE1, first, second;
//	glm::vec3 grad;
//	d = e2 / e1;
//	twoOverE2 = 2.0f / e2;
//	twoOverE1 = 2.0f / e1;

//	first = d * powf((powf(((1.0f / a1) * x), twoOverE2) + powf(((1.0f / a2) * y), twoOverE2)), d - 1.0f);
//	second = powf(1.0f / a1, twoOverE2) * (twoOverE2 * powf(x, twoOverE2 - 1.0f));
//	grad.x = first * second;

//	second = powf(1.0f / a2, twoOverE2) * (twoOverE2 * powf(y, twoOverE2 - 1.0f));
//	grad.y = first * second;

//	grad.z = powf((1.0f / a3), twoOverE1) * twoOverE1 * powf(z, twoOverE1 - 1.0f);

//	return grad;
//}

//float GradientMagnitude(float fx, float  fy, float  fz) {
//	return sqrtf(powf(fx, 2.0f) + powf(fy, 2.0f) + powf(fz, 2.0f));
//}

//glm::vec3 UnitNormal(glm::vec3 pt) {
//	return UnitNormal(pt.x, pt.y, pt.z);
//}

//glm::vec3 UnitNormal(float x, float y, float z) {
//	float gradx, grady, gradz, mag;
//	gradx = fx(x, y, z);
//	grady = fy(x, y, z);
//	gradz = fz(x, y, z);
//	return UnitNormalFromGradient(gradx, grady, gradz);
//}

//glm::vec3 UnitNormalFromGradient(float fx, float fy, float fz) {
//	float mag = GradientMagnitude(fx, fy, fz);
//	return glm::vec3(fx / mag, fy / mag, fz / mag);
//}

//glm::vec3 UnitNormalFromGradient(glm::vec3 gradient) {
//	/*float mag = GradientMagnitude(gradient.x, gradient.y, gradient.z);
//	return gradient / mag*/
//	return glm::normalize(gradient);
//}

//glm::vec3 TangentA(glm::vec3 normal) {
//	float min;
//	glm::vec3 tangent;
//	bool xMin, yMin, zMin;
//	xMin = yMin = zMin = false;

//	min = normal.x;
//	xMin = true;

//	if (normal.y < min) {
//		min = normal.y;
//		yMin = true;
//		xMin = false;
//	}

//	if (normal.z < min) {
//		yMin = xMin = false;
//	}

//	if (xMin) {
//		tangent.x = 0.0f;
//		tangent.y = -normal.z;
//		tangent.z = normal.y;
//	}
//	else if (yMin) {
//		tangent.x = -tangent.z;
//		tangent.y = 0.0f;
//		tangent.z = tangent.x;
//	}
//	else {
//		tangent.x = -normal.y;
//		tangent.y = normal.x;
//		tangent.z = 0.0f;
//	}

//	return glm::normalize(tangent);
//}

//glm::vec3 TangentB(glm::vec3 normal, glm::vec3 tangent1) {
//	return glm::cross(normal, tangent1);
//}

//static glm::vec3 ClosestPoint(Superquadric &sq1, Superquadric &sq2, glm::vec3 p1, glm::vec3 p2) {
//	//Initial estimate:
//	float xi[6] = { p1.x, p1.y, p1.z, p2.x, p2.y, p2.z };

//	float yi[6];

//	glm::vec3 p2MinusP1 = p2 - p1;

//	// yi is evaluation of each element of xi at 6 corresponding functions
//	//	 f1 is Inside-Outside - 1 of sq1 at p1 (minus 1 to equal 0)
//	//   f2 is Inside-Outside - 1 of sq2 at p2
//	//   f3 is TangetA of sq1 dot p2MinusP1
//	//   f4 is TangetB of sq1 dot p2MinusP1
//	//   f5 is TangetA of sq2 dot p2MinusP1
//	//   f6 is TangetB of sq2 dot p2MinusP1
//	evalSystem(sq1, sq2, p1, p2, yi);

//	if (!IsZeroVector(yi)) {
//		float Ji[6][6];
//		evalJ(Ji, xi);
//	}
//}

//static void evalSystem(Superquadric &sq1, Superquadric &sq2, glm::vec3 p1, glm::vec3 p2, float yi[6]) {
//	yi[0] = sq1.f(p1);
//	yi[1] = sq2.f(p2);

//	// ni is normal of sqi, tai is tangent A of sqi, tbi is tangent B of sqi
//	glm::vec3 n1, n2, ta1, ta2, tb1, tb2, p2minusP1;
//	n1 = sq1.UnitNormal(p1);
//	ta1 = sq1.TangentA(n1);
//	tb1 = sq1.TangentB(n1, ta1);

//	n2 = sq2.UnitNormal(p2);
//	ta2 = sq2.TangentA(n2);
//	tb2 = sq2.TangentB(n2, tb1);

//	p2minusP1 = (p2 - p1);
//	yi[2] = glm::dot(ta1, p2minusP1);
//	yi[3] = glm::dot(tb1, p2minusP1);
//	yi[4] = glm::dot(ta2, p2minusP1);
//	yi[5] = glm::dot(tb2, p2minusP1);
//}

//static void evalJ(float J[6][6], float x[6]) {

//}