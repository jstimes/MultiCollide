#pragma once

#include "ShapeUtils.h"

class Impact2D {

	float mu;
	float e;

	glm::vec2 u1;
	glm::vec2 u2;
	glm::vec2 contactVelocity;
	float vn;
	float vt;

	glm::vec2 contactNormal;
	glm::vec2 contactTangent;
	glm::mat2 inverseInertia;

	float nWdelta;
	float nWsigma;
	glm::vec2 sigma;
	glm::vec2 delta;

	glm::vec2 roots;

	float Inr;
	float Inc;
	float Ins;
	float IncTilde;

public:

	struct Impact2DOutput {
		glm::vec3 v1;
		glm::vec3 v2;
		
		float w1;
		float w2;
	};

	Impact2DOutput impact(float m1, float m2, float inertia1, float inertia2, 
		float frictionCoeff, float restitutionCoeff, glm::vec3 &r1, glm::vec3 &r2, 
		glm::vec3 &v1, glm::vec3 &v2, float w1, float w2) {

		this->mu = frictionCoeff;
		this->e = restitutionCoeff;

		contactNormal = glm::normalize(glm::vec2(r2.x, r2.y)); //TODO - this will only work for circles
		contactTangent = glm::vec2(-contactNormal.y, contactNormal.x);

		//Assert t x n = 1:
		float cross = contactTangent.x * contactNormal.y - contactTangent.y * contactNormal.x;
		if (MathUtils::abs(1.0f - cross) > .0001f) {
			if (MathUtils::abs(-1.0f - cross) < .0001f) {
				contactTangent *= -1.0f;
			}
		}

		glm::vec2 r1Perp = glm::vec2(-r1.y, r1.x);
		glm::vec2 r2Perp = glm::vec2(-r2.y, r2.x);
		inverseInertia = ((m1 + m2) / (m1*m2)) * glm::mat2() + 
			(1.0f / inertia1) * glm::outerProduct(r1Perp, r1Perp) + (1.0f / inertia2) * glm::outerProduct(r2Perp, r2Perp);

		u1 = glm::vec2(v1) + w1*r1Perp;
		u2 = glm::vec2(v2) + w2*r2Perp;
		contactVelocity = u1 - u2;
		vn = glm::dot(contactVelocity, contactNormal);
		vt = glm::dot(contactVelocity, contactTangent);

		glm::vec2 Ir2D;
		if (mu < 0.00001f) {
			frictionlessContact();
			Ir2D = contactNormal * Inr;
		}
		else {
			frictionalContact();
			Ir2D = Ins * sigma + (Inr - Ins) * delta;
		}

		glm::vec3 Ir(Ir2D.x, Ir2D.y, 0.0f);

		Impact2DOutput output;
		output.v1 = v1 + (1.0f / m1) * Ir;
		output.v2 = v2 - (1.0f / m2) * Ir;
		output.w1 = w1 + (1.0f / inertia1) * (r1.x * Ir.y - r1.y * Ir.x);
		output.w2 = w2 - (1.0f / inertia2) * (r2.x * Ir.y - r2.y * Ir.x);

		return output;
	}

	void frictionlessContact() {
		
		Inc = (-1.0f * vn) / glm::dot(contactNormal, inverseInertia * contactNormal);

		Inr = Inc * (1.0f + e);
	}

	void frictionalContact() {
		calculateSigma();
		calculateDelta();

		glm::vec2 Wsigma = inverseInertia * sigma;
		float tWsigma = glm::dot(contactTangent, Wsigma);
		nWsigma = glm::dot(contactNormal, Wsigma);
		nWdelta = glm::dot(contactNormal, inverseInertia * delta);

		Ins = 0.0f;
		if (MathUtils::abs(vt) < 0.00001f || glm::distance(sigma, delta) < 0.0001f) {
			Inc = (-1.0f * vn) / glm::dot(contactNormal, inverseInertia * delta);

			Inr = Inc * (1.0f + e);
			return;
		}
		else if (MathUtils::abs(tWsigma) > 0.00001f && MathUtils::checkOppositeSigns(vt, tWsigma)) {
			Ins = (-1.0f * vt) / tWsigma;
		}
		else {
			Ins = INFINITY;
			std::cout << "Ins is infinity" << std::endl;
		}

		IncTilde = (-1.0f * vn) / nWsigma;
		if (nWsigma <= 0.0f) {
			IncTilde = INFINITY;
			std::cout << "Inc is infinity" << std::endl;
		}

		if (Ins <= IncTilde) {
			//Case 1:
			roots = scr();
		}
		else {
			//Case 2
			roots = case2();
		}

		if (roots.x > Inc) {
			Inr = roots.x;

			if (roots.y > Inc) {
				std::cout << "Both roots in case 1 are greater than Inc" << std::endl;
			}
		}
		else if (roots.y > Inc) {
			Inr = roots.y;
		}
		else {
			std::cout << "Neither root in case 1 is greater than Inc" << std::endl;
		}
	}

private:

	glm::vec2 scr() {

		float a = -.5f * nWdelta;
		float b = (-1.0f * (vn + glm::dot(contactNormal, inverseInertia * (sigma - delta)) * Ins));
		Inc = b / nWdelta;
		float c2 = (e*e - 1.0f) * phi2(Inc) + e*e*(phi1(Ins) - phi2(Ins)) - .5f * nWdelta * Ins * Ins;

		return MathUtils::solveQuadratic(a, b, c2);
	}

	glm::vec2 case2() {
		Inc = IncTilde;
		float E2Ins = E1(Ins);

		if (E2Ins < 0.0f) {
			//cr
			float a = -.5f * nWsigma;
			float b = -vn;
			float c = (e*e - 1.0f) * phi1(Inc);

			return MathUtils::solveQuadratic(a, b, c);
		}
		else {
			//csr
			float a = -.5f * nWdelta;
			float b = -1.0f * (vn + glm::dot(contactNormal, inverseInertia * (sigma - delta)) * Ins);
			float c1 = (e*e - 1.0f) * phi1(Inc) + phi1(Ins) - phi2(Ins) - .5f * nWdelta * Ins * Ins;

			return MathUtils::solveQuadratic(a, b, c1);
		}
	}

	void calculateSigma() {
		sigma = glm::vec2(0.0f);
		if (vt > 0.0f) {
			sigma = contactNormal - mu * contactTangent;
		}
		if (vt < 0.0f) {
			sigma = contactNormal + mu * contactTangent;
		}
	}

	void calculateDelta() {
		float tWn = glm::dot(contactTangent, inverseInertia * contactNormal);
		float abs_tWn = MathUtils::abs(tWn);
		float tWt = glm::dot(contactTangent, inverseInertia * contactTangent);
		float mu_tWt = mu * tWt;

		delta = contactNormal;

		if (abs_tWn <= mu_tWt) {
			delta = delta - ((tWn / tWt) * contactTangent);
		}
		else {
			if (vt > 0.0001f) {
				delta += (mu * contactTangent);
			}
			else if (vt < -0.0001f) {
				delta -= (mu * contactTangent);
			}
			else {
				delta -= (mu * (tWn / abs_tWn) * contactTangent);
			}
		}
	}

	float phi1(float In) {
		return -vn * In - .5f * nWsigma * In * In;
	}

	float phi2(float In) {
		return -vn * In - nWsigma * Ins * In - .5f * nWdelta * (In - Ins) * (In - Ins);
	}

	float E1(float In) {
		return -.5f * nWsigma * In * In - vn * In + (e*e - 1.0f) * phi1(Inc);
	}
};