#pragma once

#include<stdio.h>
#include<iostream>
#include <Eigen/Dense>
#include<vector>
#include <math.h>  

#include "MathUtils.h"


//includes from Jacob's quartic code
#include <stdexcept>
#include <complex>

struct ImpactClosedFormOutput
{
	glm::vec3 v1end, v2end, w1end, w2end, Iend;
};

class ImpactClosedForm
{

	//Constants for root finding:
	static const std::complex<double> EPSILON;// = std::numeric_limits<double>::epsilon();
	static const std::complex<double> pi;// = 3.1415926536;
	static const std::complex<double> i;// (0.0, 1.0);
	static const double DEFAULT_ACCURACY;// = .000001;

	//static int NumIter;// = 10000; //maximum number of iterations
	static const double constantStep; // = 0.0001; //numerical intergration step size
	static const double constSlidingisZero;// = 8.0*0.001; //the judging criteria for sliding velocity equals zero
	static const double constStraightSlidingisZero;// = 8.0*0.001;

	double m1, m2, mu, e;

	Eigen::Matrix3d R1, R2, Q1, Q2, RTAN, P1, P2;

	Eigen::Vector3d r1, r2, v1, v2, w1, w2, nhat, uhat, ohat;

public:
	Eigen::Vector3d v1end, v2end, w1end, w2end;

	Eigen::Vector3d Iend;

	Eigen::Matrix3d getPmatrix(Eigen::Vector3d r) {
		Eigen::Matrix3d P;
		P << 0, -r[2], r[1],
			r[2], 0, -r[0],
			-r[1], r[0], 0;
		return P;
	}

	//Wrapper constructor that converts glm data types to Eigen, which is what's used by the program
	ImpactClosedForm(float m1, float m2, float mu, float e, glm::mat3 &R1, glm::mat3 &R2, glm::mat3 &Q1, glm::mat3 &Q2, glm::mat3 &RTAN,
		glm::vec3 &r1, glm::vec3 &r2, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &w1, glm::vec3 &w2)

		: ImpactClosedForm(m1, m2, mu, e, MathUtils::glmToEigenMatrix(R1), MathUtils::glmToEigenMatrix(R2), MathUtils::glmToEigenMatrix(Q1), MathUtils::glmToEigenMatrix(Q2),
			MathUtils::glmToEigenMatrix(RTAN), MathUtils::glmToEigenVector(r1), MathUtils::glmToEigenVector(r2), MathUtils::glmToEigenVector(v1), MathUtils::glmToEigenVector(v2),
			MathUtils::glmToEigenVector(w1), MathUtils::glmToEigenVector(w2)) { }

	ImpactClosedForm(double m1, double m2, double mu, double e, Eigen::Matrix3d R1, Eigen::Matrix3d R2, Eigen::Matrix3d Q1, Eigen::Matrix3d Q2, Eigen::Matrix3d RTAN,
		Eigen::Vector3d r1, Eigen::Vector3d r2, Eigen::Vector3d v1, Eigen::Vector3d v2, Eigen::Vector3d w1, Eigen::Vector3d w2) {
		this->m1 = m1;
		this->m2 = m2;
		this->e = e;
		this->mu = mu;
		//this->StepsizeIn = StepsizeIn;
		this->R1 = R1;
		this->R2 = R2;
		this->Q1 = Q1;
		this->Q2 = Q2;
		this->RTAN = RTAN;
		this->r1 = r1;
		this->r2 = r2;
		this->v1 = v1;
		this->v2 = v2;
		this->w1 = w1;
		this->w2 = w2;
		P1 = getPmatrix(r1);
		P2 = getPmatrix(r2);
		nhat = RTAN * Eigen::Vector3d(1.0, 0.0, 0.0);//normal vector, currently is x direction
		uhat = RTAN * Eigen::Vector3d(0.0, 1.0, 0.0); //unit vector in tangential plane, y direction 
		ohat = RTAN * Eigen::Vector3d(0.0, 0.0, 1.0); //unit vector in tangential plane, happen to be z direction.
	}


	bool compareComplexes(std::complex<double> c1, std::complex<double> c2)
	{
		if (c1.real() > c2.real())
			return true;
		if (c2.real() > c1.real())
			return false;
		return c1.imag() > c2.imag();
	}

	std::string complexToString(std::complex<double> c)
	{
		std::ostringstream ossR, ossI;
		ossR << c.real();
		ossI << c.imag();
		return "" + ossR.str() + " + " + ossI.str() + "i";
	}

	int QuadraticRoots(std::complex<double> coefficients[], std::complex<double> roots[]) {
		std::complex<double> a = coefficients[2];
		std::complex<double> b = coefficients[1];
		std::complex<double> c = coefficients[0];
		std::complex<double> underRoot = b*b - 4.0 * a*c;
		if (compareComplexes(underRoot, EPSILON)) {
			roots[0] = (-b + sqrt(underRoot)) / (2.0 * a);
			roots[1] = (-b - sqrt(underRoot)) / (2.0 * a);
			return 2;
		}
		else if (!compareComplexes(underRoot, -EPSILON)) {
			return 0;
		}
		else {
			roots[0] = roots[1] = -b / (2.0*a);
			return 2;
		}
	}

	int CubicRoots(std::complex<double> coefficients[], std::complex<double> roots[])
	{
		using namespace std;

		if (coefficients[3] == 0.0) {
			//throw std::invalid_argument("Not cubic function");
			return QuadraticRoots(coefficients, roots);
		}
		std::complex<double> p = coefficients[2] / coefficients[3];
		std::complex<double> q = coefficients[1] / coefficients[3];
		std::complex<double> r = coefficients[0] / coefficients[3];

		std::complex<double> a = (3.0 * q - p*p) / 3.0;
		std::complex<double> b = (2.0 * p*p*p - 9.0*p*q + 27.0*r) / 27.0;

		std::complex<double> innerRoot = (b*b) / 4.0 + (a*a*a) / 27.0;
		std::complex<double> innerA = -b / 2.0 + sqrt(innerRoot);
		std::complex<double> innerB = -b / 2.0 - sqrt(innerRoot);
		std::complex<double> A = pow(-b / 2.0 + sqrt(innerRoot), 1.0 / 3.0); //cubic root
		std::complex<double> B = pow(innerB, 1.0 / 3.0);
		if (innerB.real() < EPSILON.real()) {
			//B = cbrt(innerB.real());
			double sign = (innerB.real() < 0.0 ? -1.0 : 1.0);
			B = sign * std::pow(abs(innerB.real()), (1.0 / 3.0));
		}

		std::complex<double> convertBack = p / 3.0;

		// compare complexes returns true if real of innerRoot is greater than EPSILON
		if (compareComplexes(innerRoot, EPSILON)) {
			//1 real root
			std::complex<double> y1 = A + B;
			std::complex<double> conj = ((i * sqrt(3.0)) / 2.0) * (A - B);
			std::complex<double> y2 = (-.5) * y1 + conj;
			std::complex<double> y3 = (-.5) * y1 - conj;
			roots[0] = y1 - convertBack;
			roots[1] = y2 - convertBack;
			roots[2] = y3 - convertBack;
			return 1;
		}
		else {
			std::complex<double> sqrtAover3 = sqrt(-a / 3.0);
			if (!compareComplexes(innerRoot, EPSILON)) {
				//3 real, unequal roots
				std::complex<double> phi;
				std::complex<double> acosParam = sqrt((b*b / 4.0) / (-(a*a*a) / 27.0));
				if (compareComplexes(b, EPSILON)) {
					phi = acos(-acosParam.real());
				}
				else {
					phi = acos(acosParam.real());
				}
				for (int k = 0; k < 3; k++) {
					roots[k] = 2.0 * sqrtAover3 * cos(phi / 3.0 + (2.0 * k * pi) / 3.0) - convertBack;
				}
			}
			else {
				// b > 0
				if (compareComplexes(b, EPSILON)) {
					roots[0] = -2.0 * sqrtAover3 - convertBack;
					roots[1] = roots[2] = sqrtAover3 - convertBack;
				}
				// b < 0
				else if (!compareComplexes(b, EPSILON)) {
					roots[0] = 2.0 * sqrtAover3 - convertBack;
					roots[1] = roots[2] = -1.0 * sqrtAover3 - convertBack;
				}
				//b = 0
				else {
					roots[0] = 0.0 - convertBack;
					roots[1] = 0.0 - convertBack;
					roots[2] = 0.0 - convertBack;
				}
			}
			return 3;
		}
	}

	int QuarticRoots(std::complex<double> coefficients[], std::complex<double> roots[])
	{

		using namespace std;

		if (coefficients[4] == 0.0) {
			//throw std::invalid_argument("Not quartic function");
			return CubicRoots(coefficients, roots);
		}
		std::complex<double> p = coefficients[3] / coefficients[4];
		std::complex<double> q = coefficients[2] / coefficients[4];
		std::complex<double> r = coefficients[1] / coefficients[4];
		std::complex<double> s = coefficients[0] / coefficients[4];

		std::complex<double> cubic[4] = { 4.0 * q * s - r*r - p*p*s, p*r - 4.0*s, -q, 1.0 };
		std::complex<double> cubicRoots[3];
		CubicRoots(cubic, cubicRoots);
		std::complex<double> z1 = cubicRoots[0];

		std::complex<double> R, D, E;
		R = sqrt(.25 * p * p - q + z1);
		if (compareComplexes(abs(R), EPSILON)) {
			//R != 0
			std::complex<double> firstPart = .75 * p * p - R*R - 2.0 * q;
			std::complex<double> secondPart = .25 * (4.0 * p * q - 8.0 * r - p*p*p) * (1.0 / R);
			D = sqrt(firstPart + secondPart);
			E = sqrt(firstPart - secondPart);
		}
		else {
			// R == 0
			std::complex<double> firstPart = .75 * p * p - 2.0 * q;
			std::complex<double> secondPart = 2.0 * sqrt(z1*z1 - 4.0 * s);
			D = sqrt(firstPart + secondPart);
			E = sqrt(firstPart + secondPart);
		}

		std::complex<double> negPover4 = -p / 4.0;
		roots[0] = negPover4 + .5 * (R + D);
		roots[1] = negPover4 + .5 * (R - D);
		roots[2] = negPover4 - .5 * (R - E);
		roots[3] = negPover4 - .5 * (R + E);
		return 0;
	}



	Eigen::Vector2d centrifugal(double mu, double c1, double c2, double B11, double B12, double B21, double B22)//find out centrifugal
	{
		using namespace std;

		// q(x) = a0 + a1 *x +a2 *x^2 + a3*x^3 + a4*x^4
		std::complex<double> q4[] = { c2 + B21, -2 * (c1 + B11 - B22), -2 * (B21 + 2 * B12), 2 * (B11 - B22 - c1), B21 - c2 };
		std::complex<double> rootsQ4[4];
		QuarticRoots(q4, rootsQ4);
		//cout << "quartic equation q(x) roots:" << endl;
		//cout << complexToString(rootsQ4[0]) << endl<< complexToString(rootsQ4[1]) <<endl << complexToString(rootsQ4[2]) <<endl << complexToString(rootsQ4[3]) << endl << endl;

		Eigen::Vector2d res, res0;
		res0 << -999999.0, -999999.0;
		for (int i = 0; i<4; i++)
		{

			if (abs(rootsQ4[i].imag())<0.00000001)
			{
				if (rootsQ4[i].real()*rootsQ4[i].real()*rootsQ4[i].real()*rootsQ4[i].real()*(-c1 + B11) + rootsQ4[i].real()*rootsQ4[i].real()*rootsQ4[i].real()*(2 * c2 - 2 * (B12 + B21)) + rootsQ4[i].real()*rootsQ4[i].real()*(-2 * B11 + 4 * B22) + rootsQ4[i].real()*(2 * c2 + 2 * B12 + 2 * B21) + B11 + c1> 0)
				{
					res << (1 - rootsQ4[i].real()*rootsQ4[i].real()) / (1 + rootsQ4[i].real()*rootsQ4[i].real()), (2 * rootsQ4[i].real()) / (1 + rootsQ4[i].real()*rootsQ4[i].real());
					return res;
				}
			}
		}

		return res0;
	}

	double Phi1(double initialVnormaln, Eigen::Vector3d nhat, Eigen::Vector3d Is, double Izs, Eigen::Matrix3d S, Eigen::Vector3d sigma, double msigma, double Iz) //Phi1 function
	{
		return (-initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma)))* Iz - (1 / (2 * msigma))*Iz*Iz;
	}

	double Phi2(double initialVnormaln, Eigen::Vector3d nhat, Eigen::Vector3d Il, double Izl, Eigen::Matrix3d S, Eigen::Vector3d delta, double mdelta, double Iz) //Phi1 function
	{
		return (-initialVnormaln + (nhat.transpose()*S*(Il - Izl* delta)))* Iz - (1 / (2 * mdelta))*Iz*Iz;
	}


	ImpactClosedFormOutput impact();

};

