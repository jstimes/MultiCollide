#pragma once

#include<stdio.h>
#include<iostream>
#include <Eigen/Dense>
#include<vector>
#include <math.h>  

#include "MathUtils.h"

////////////////////////includes from Jacob's quartic code
#include <stdexcept>
#include <complex>
//const std::complex<double> EPSILON = std::numeric_limits<double>::epsilon();
//const std::complex<double> pi = 3.1415926536;
//const std::complex<double> i(0.0, 1.0);
//const double DEFAULT_ACCURACY = .000001;
///////////////////////////////////////////////////

using namespace Eigen;

struct ImpactClosedForm2Output
{
	glm::vec3 v1end, v2end, w1end, w2end, Iend;
};

//const double constStraightSlidingisZero = 8.0*0.001;
class ImpactClosedForm2
{
	//Constants for root finding:
	static const std::complex<double> EPSILON;// = std::numeric_limits<double>::epsilon();
	static const std::complex<double> pi;// = 3.1415926536;
	static const std::complex<double> i;// (0.0, 1.0);
	static const double DEFAULT_ACCURACY;// = .000001;

	static const int NumIter;// = 10000; //maximum number of iterations
	static const double constantStep; // = 0.0001; //numerical intergration step size
	static const double constSlidingisZero;// = 8.0*0.001; //the judging criteria for sliding velocity equals zero
	static const double constStraightSlidingisZero;// = 8.0*0.001;

	double m1, m2, mu, e;
	Matrix3d R1, R2, Q1, Q2, RTAN, P1, P2;
	Vector3d r1, r2, v1, v2, w1, w2, nhat, uhat, ohat;

public:
	Vector3d v1end, v2end, w1end, w2end;
	Vector3d Iend;
	Matrix3d getPmatrix(Vector3d r);

	//Wrapper constructor that converts glm data types to Eigen, which is what's used by the program
	ImpactClosedForm2(float m1, float m2, float mu, float e, glm::mat3 &R1, glm::mat3 &R2, glm::mat3 &Q1, glm::mat3 &Q2, glm::mat3 &RTAN,
		glm::vec3 &r1, glm::vec3 &r2, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &w1, glm::vec3 &w2)

		: ImpactClosedForm2(m1, m2, mu, e, MathUtils::glmToEigenMatrix(R1), MathUtils::glmToEigenMatrix(R2), MathUtils::glmToEigenMatrix(Q1), MathUtils::glmToEigenMatrix(Q2),
			MathUtils::glmToEigenMatrix(RTAN), MathUtils::glmToEigenVector(r1), MathUtils::glmToEigenVector(r2), MathUtils::glmToEigenVector(v1), MathUtils::glmToEigenVector(v2),
			MathUtils::glmToEigenVector(w1), MathUtils::glmToEigenVector(w2)) { }

	ImpactClosedForm2(double m1, double m2, double mu, double e, Matrix3d R1, Matrix3d R2, Matrix3d Q1, Matrix3d Q2, Matrix3d RTAN,
		Vector3d r1, Vector3d r2, Vector3d v1, Vector3d v2, Vector3d w1, Vector3d w2) {
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
		nhat = RTAN * Vector3d(1.0, 0.0, 0.0);//normal vector, currently is x direction
		uhat = RTAN * Vector3d(0.0, 1.0, 0.0); //unit vector in tangential plane, y direction 
		ohat = RTAN * Vector3d(0.0, 0.0, 1.0); //unit vector in tangential plane, happen to be z direction.
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
		using namespace std;
		ostringstream ossR, ossI;
		ossR << c.real();
		ossI << c.imag();
		return "" + ossR.str() + " + " + ossI.str() + "i";
	}

	int QuadraticRoots(std::complex<double> coefficients[], std::complex<double> roots[]) {
		using namespace std;
		complex<double> a = coefficients[2];
		complex<double> b = coefficients[1];
		complex<double> c = coefficients[0];
		complex<double> underRoot = b*b - 4.0 * a*c;
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
		complex<double> p = coefficients[2] / coefficients[3];
		complex<double> q = coefficients[1] / coefficients[3];
		complex<double> r = coefficients[0] / coefficients[3];

		complex<double> a = (3.0 * q - p*p) / 3.0;
		complex<double> b = (2.0 * p*p*p - 9.0*p*q + 27.0*r) / 27.0;

		complex<double> innerRoot = (b*b) / 4.0 + (a*a*a) / 27.0;
		complex<double> innerA = -b / 2.0 + sqrt(innerRoot);
		complex<double> innerB = -b / 2.0 - sqrt(innerRoot);
		complex<double> A = pow(-b / 2.0 + sqrt(innerRoot), 1.0 / 3.0); //cubic root
		complex<double> B = pow(innerB, 1.0 / 3.0);
		if (innerB.real() < EPSILON.real()) {
			//B = cbrt(innerB.real());
			double sign = (innerB.real() < 0.0 ? -1.0 : 1.0);
			B = sign * std::pow(abs(innerB.real()), (1.0 / 3.0));
		}

		complex<double> convertBack = p / 3.0;

		// compare complexes returns true if real of innerRoot is greater than EPSILON
		if (compareComplexes(innerRoot, EPSILON)) {
			//1 real root
			complex<double> y1 = A + B;
			complex<double> conj = ((i * sqrt(3.0)) / 2.0) * (A - B);
			complex<double> y2 = (-.5) * y1 + conj;
			complex<double> y3 = (-.5) * y1 - conj;
			roots[0] = y1 - convertBack;
			roots[1] = y2 - convertBack;
			roots[2] = y3 - convertBack;
			return 1;
		}
		else {
			complex<double> sqrtAover3 = sqrt(-a / 3.0);
			if (!compareComplexes(innerRoot, EPSILON)) {
				//3 real, unequal roots
				complex<double> phi;
				complex<double> acosParam = sqrt((b*b / 4.0) / (-(a*a*a) / 27.0));
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
		complex<double> p = coefficients[3] / coefficients[4];
		complex<double> q = coefficients[2] / coefficients[4];
		complex<double> r = coefficients[1] / coefficients[4];
		complex<double> s = coefficients[0] / coefficients[4];

		complex<double> cubic[4] = { 4.0 * q * s - r*r - p*p*s, p*r - 4.0*s, -q, 1.0 };
		complex<double> cubicRoots[3];
		CubicRoots(cubic, cubicRoots);
		complex<double> z1 = cubicRoots[0];

		complex<double> R, D, E;
		R = sqrt(.25 * p * p - q + z1);
		if (compareComplexes(abs(R), EPSILON)) {
			//R != 0
			complex<double> firstPart = .75 * p * p - R*R - 2.0 * q;
			complex<double> secondPart = .25 * (4.0 * p * q - 8.0 * r - p*p*p) * (1.0 / R);
			D = sqrt(firstPart + secondPart);
			E = sqrt(firstPart - secondPart);
		}
		else {
			// R == 0
			complex<double> firstPart = .75 * p * p - 2.0 * q;
			complex<double> secondPart = 2.0 * sqrt(z1*z1 - 4.0 * s);
			D = sqrt(firstPart + secondPart);
			E = sqrt(firstPart + secondPart);
		}

		complex<double> negPover4 = -p / 4.0;
		roots[0] = negPover4 + .5 * (R + D);
		roots[1] = negPover4 + .5 * (R - D);
		roots[2] = negPover4 - .5 * (R - E);
		roots[3] = negPover4 - .5 * (R + E);
		return 0;
	}



	Vector2d centrifugal(double mu, double c1, double c2, double B11, double B12, double B21, double B22)//find out centrifugal
	{
		using namespace std;
		// q(x) = a0 + a1 *x +a2 *x^2 + a3*x^3 + a4*x^4
		complex<double> q4[] = { c2 + B21, -2 * (c1 + B11 - B22), -2 * (B21 + 2 * B12), 2 * (B11 - B22 - c1), B21 - c2 };
		complex<double> rootsQ4[4];
		QuarticRoots(q4, rootsQ4);
		//cout << "quartic equation q(x) roots:" << endl;
		//cout << complexToString(rootsQ4[0]) << endl<< complexToString(rootsQ4[1]) <<endl << complexToString(rootsQ4[2]) <<endl << complexToString(rootsQ4[3]) << endl << endl;

		Vector2d res, res0;
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

	double Phi1(double ImpactClosedForm2Vnormaln, Vector3d nhat, Vector3d Is, double Izs, Matrix3d S, Vector3d sigma, double msigma, double Iz) //Phi1 function
	{
		return (-ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma)))* Iz - (1 / (2 * msigma))*Iz*Iz;
	}

	double Phi2(double ImpactClosedForm2Vnormaln, Vector3d nhat, Vector3d Il, double Izl, Matrix3d S, Vector3d delta, double mdelta, double Iz) //Phi1 function
	{
		return (-ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Il - Izl* delta)))* Iz - (1 / (2 * mdelta))*Iz*Iz;
	}

	ImpactClosedForm2Output impact();
	//void impact()
	//{
	//	double ImpactClosedForm2Vperpu, ImpactClosedForm2Vperpw, ImpactClosedForm2Vnormaln;
	//	Matrix3d Rot, S, A, Identity3;
	//	Vector3d ImpactClosedForm2Vperp, ImpactClosedForm2Vnormal;
	//	MatrixXd uwJuzhen(3, 2);
	//	Matrix2d B, BNew, Binv;
	//	Vector2d c;
	//	S = R1*P1*Q1.inverse()*P1*R1.inverse() + R2*P2*Q2.inverse()*P2*R2.inverse();
	//	uwJuzhen << uhat[0], ohat[0],
	//		uhat[1], ohat[1],
	//		uhat[2], ohat[2];
	//	Identity3 << 1.0, 0.0, 0.0,
	//		0.0, 1.0, 0.0,
	//		0.0, 0.0, 1.0;
	//	A = (Identity3 - nhat*nhat.transpose())*((1.0 / m1 + 1.0 / m2)*Identity3 - S);
	//	ImpactClosedForm2Vperp = ((Identity3 - nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (Identity3 - nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	ImpactClosedForm2Vperpu = ImpactClosedForm2Vperp.transpose()*uhat;
	//	ImpactClosedForm2Vperpw = ImpactClosedForm2Vperp.transpose()*ohat;
	//	ImpactClosedForm2Vnormal = ((nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	ImpactClosedForm2Vnormaln = ImpactClosedForm2Vnormal.transpose()*nhat;
	//	B = (-mu)*uwJuzhen.transpose()*A*uwJuzhen;
	//	double B11, B12, B21, B22;
	//	B11 = B(0, 0); B12 = B(0, 1); B21 = B(1, 0); B22 = B(1, 1);//the elements of B matrix
	//	BNew = uwJuzhen.transpose()*A*uwJuzhen;//This is the new definition of B in the note that does not have mu in it
	//	Binv = BNew.inverse();
	//	c = uwJuzhen.transpose()*A*nhat;
	//	double c1, c2;
	//	c1 = c(0); c2 = c(1);

	//	Vector2d shat;
	//	Vector3d sigma;
	//	shat = centrifugal(mu, c1, c2, B11, B12, B21, B22);
	//	//cout<<"centrifugal direction: "<<endl<<shat<<endl<<endl;

	//	if ((Binv*c).norm()<mu)
	//	{
	//		sigma = nhat - uwJuzhen*Binv*c;
	//	}
	//	else
	//	{
	//		sigma = nhat - mu*uwJuzhen*shat;
	//	}
	//	// cout<<"sigma: "<<endl<<sigma<<endl<<endl;


	//	//start ImpactClosedForm2ization part for the numerical integration using the Euler's method.
	//	double StepsizeIn = constantStep;
	//	double vvu[NumIter], vvw[NumIter], vvn[NumIter], IIu[NumIter], IIw[NumIter], IIn[NumIter], EE[NumIter], curvatureOfV[NumIter],
	//		nuPrimeNorm[NumIter], stepSizeInCount[NumIter];
	//	double lambdaStep = 0.5; double h1 = 0.01; double h2 = 0.01;
	//	double top = 0.0; double bottom = 1.0; double valueofYPrime = 0.0; double x = 0.0; double y = 0.0;
	//	for (int i = 0; i<NumIter; i++)
	//	{
	//		vvu[i] = 0;
	//		vvw[i] = 0;
	//		vvn[i] = 0;
	//		IIu[i] = 0;
	//		IIw[i] = 0;
	//		IIn[i] = 0;
	//		EE[i] = 0;
	//		curvatureOfV[i] = 0;
	//	}
	//	vvu[0] = ImpactClosedForm2Vperpu;
	//	vvw[0] = ImpactClosedForm2Vperpw;
	//	vvn[0] = ImpactClosedForm2Vnormaln;
	//	int count = 0; int finish = 0; //finish is the total steps for calc, <NumIter

	//	int compress = 1;//(*compression=1 means compress, =0 means restitution*)
	//	int sliding = 1;
	//	if (vvu[count] * vvu[count] + vvw[count] * vvw[count]>pow(10.0, -6))//(*sliding=1 means slide, sliding=0 means sticking*)
	//	{
	//		sliding = 1;
	//		//printf("this is the case\n");
	//	}
	//	else
	//	{
	//		sliding = 0;
	//	}

	//	//double top=0.0; double bottom=1.0; double x=0.0; double y=0.0; double valueofYPrime=0.0;//curvature related calculation

	//	int countCom = 0; int countSlide = 0;//either 0 or 1, different status,comp means compression
	//	vvu[0] = ImpactClosedForm2Vperpu; vvw[0] = ImpactClosedForm2Vperpw;
	//	vvn[0] = ImpactClosedForm2Vnormaln; EE[0] = 0.0;
	//	double tempVVN = vvn[0];


	//	double Eprime = 0.0; double Izc = 0.0; double Ec = 0.0; double Izr = 0.0; double Izs = 0.0; double Es = 0.0;
	//	int situation = 0;
	//	Vector3d Iend1, Iend21, Iend22, Iend31, Iend32a, Iend32b, Iend33, Iend34, Is;
	//	double msigma = 1.0 / (1.0 / m1 + 1.0 / m2 - nhat.transpose()*S*sigma);
	//	Iend1 << 0, 0, 0;
	//	Iend21 << 0, 0, 0;
	//	Iend22 << 0, 0, 0;
	//	Iend31 << 0, 0, 0;
	//	Iend32a << 0, 0, 0;
	//	Iend32b << 0, 0, 0;
	//	Iend33 << 0, 0, 0;
	//	Iend34 << 0, 0, 0;
	//	Is << 0, 0, 0;


	//	while (count<NumIter)
	//	{
	//		Vector3d Icurrent;

	//		Icurrent << IIn[count], IIu[count], IIw[count];

	//		if (EE[count]<-0.000000000000001)//checking for finish condition
	//		{
	//			finish = count;

	//			Iend1 = RTAN*Icurrent;
	//			situation = 1;
	//			cout << "situation1: " << situation << endl << endl;
	//			return;
	//		}

	//		if (sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count])<constSlidingisZero)
	//		{
	//			situation = 2;
	//			//cout<<"situation2: "<<situation<<endl<<endl;
	//			// cout<<"vvu[count]: "<<vvu[count]<<endl<<endl;
	//			//cout<<"vvw[count]: "<<vvw[count]<<endl<<endl;
	//			break;
	//		}
	//		Vector2d vSlideCurrent;
	//		vSlideCurrent << vvu[count], vvw[count];

	//		Vector2d temperoryVec1 = ((-mu*BNew*vSlideCurrent / vSlideCurrent.norm() + c) / ((-mu*BNew*vSlideCurrent / vSlideCurrent.norm() + c).norm()));
	//		double  temperoryConst1 = temperoryVec1(1);
	//		double  temperoryConst2 = temperoryVec1(0);


	//		if (sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count]) >= constSlidingisZero &&
	//			(abs(vvu[count] / vSlideCurrent.norm()*temperoryConst1 - vvw[count] / vSlideCurrent.norm()*temperoryConst2)< constStraightSlidingisZero))
	//		{
	//			situation = 3;
	//			cout << "situation3: " << situation << endl << endl;
	//			break;
	//		}

	//		//Now coompute the curvature related thing to determine the stepsize
	//		x = vvu[count];
	//		y = vvw[count];
	//		if (abs((B11*x + B12*y + c1*sqrt(x*x + y*y))) >DEFAULT_ACCURACY)
	//		{
	//			valueofYPrime = (B21*x + B22*y + c2*sqrt(x*x + y*y)) / (B11*x + B12*y + c1*sqrt(x*x + y*y));
	//		}
	//		else
	//		{
	//			bottom = 0.0;
	//		}
	//		if (abs(sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)))  >DEFAULT_ACCURACY)
	//		{
	//			top = (((B21*c1 - B11*c2)*y - B22*(c1*x + B11*sqrt(x*x + y*y)) + B12*(c2*x + B21*sqrt(x*x + y*y)))*(y - x*valueofYPrime)) / (sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)));
	//		}
	//		else
	//		{
	//			bottom = 0;
	//		}
	//		bottom = pow(1.0 + valueofYPrime*valueofYPrime, 1.5);

	//		if (abs(bottom)>DEFAULT_ACCURACY)
	//		{
	//			curvatureOfV[count] = abs(top / bottom);
	//		}
	//		else
	//		{
	//			curvatureOfV[count] = 0.0;
	//		}
	//		if (x*x + y*y>constSlidingisZero)
	//		{
	//			nuPrimeNorm[count] = sqrt((B11*x / sqrt(x*x + y*y) + B12*y / sqrt(x*x + y*y) + c1)*(B11*x / sqrt(x*x + y*y) + B12*y / sqrt(x*x + y*y) + c1)
	//				+ (B21*x / sqrt(x*x + y*y) + B22*y / sqrt(x*x + y*y) + c2)*(B21*x / sqrt(x*x + y*y) + B22*y / sqrt(x*x + y*y) + c2));
	//		}
	//		else
	//		{
	//			nuPrimeNorm[count] = 0;
	//		}
	//		if (nuPrimeNorm[count] == 0 || curvatureOfV[count] == 0)
	//		{
	//			StepsizeIn = constantStep;
	//		}
	//		else
	//		{
	//			StepsizeIn = lambdaStep*h1 / nuPrimeNorm[count] + (1 - lambdaStep)*h2 / (curvatureOfV[count] * nuPrimeNorm[count]);
	//		}
	//		stepSizeInCount[count] = StepsizeIn;

	//		if (compress == 1 && tempVVN*vvn[count]<0.000001)//checking for finishing compression phase, the sign of vn(which is normal velocity) changes
	//		{
	//			compress = 0;
	//			countCom = count;
	//			Izc = IIn[count];
	//			EE[count] = e*e*EE[count];
	//		}
	//		tempVVN = vvn[count];
	//		count++;
	//		IIn[count] = IIn[count - 1] + StepsizeIn;
	//		//below is the method for updating these things
	//		Vector2d temp5, temp6;
	//		temp5 << vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]),
	//			vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//		temp6 = B*temp5;
	//		vvu[count] = vvu[count - 1] + StepsizeIn*(c[0] + temp6[0]);
	//		vvw[count] = vvw[count - 1] + StepsizeIn*(c[1] + temp6[1]);
	//		IIu[count] = IIu[count - 1] + StepsizeIn*(-1.0*mu)*vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//		IIw[count] = IIw[count - 1] + StepsizeIn*(-1.0*mu)*vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);

	//		MatrixXd temp7(1, 3), temp8(3, 1);
	//		double temp9;
	//		temp7 = nhat.transpose()*S*RTAN;
	//		temp8 << IIn[count],
	//			IIu[count],
	//			IIw[count];
	//		temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
	//		vvn[count] = ImpactClosedForm2Vnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
	//		EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	//	}

	//	if (situation == 2)
	//	{
	//		Vector2d temporaryVect3;
	//		temporaryVect3 << ImpactClosedForm2Vperpu, ImpactClosedForm2Vperpw;
	//		Izs = IIn[count];
	//		Is = (IIn[count] * nhat - uwJuzhen*Binv*(temporaryVect3 + IIn[count] * c));
	//		Es = EE[count];
	//		Eprime = -ImpactClosedForm2Vnormaln - (1 / m1 + 1 / m2)* Izs + (nhat.transpose()*S*Is);
	//		double a2 = -1 / (2 * msigma);
	//		double a1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma));

	//		if (Eprime>0)
	//		{
	//			Izc = msigma* (-ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma)));
	//			Ec = Es + Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izc) - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
	//			double a0 = e*e*Ec - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izc);
	//			double root1 = (-a1 - sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);
	//			double root2 = (-a1 + sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);

	//			if (root1 > Izc && root2 > Izc)
	//			{
	//				Izr = root1<root2 ? root1 : root2;
	//			}
	//			else if (root1 > Izc)
	//			{
	//				Izr = root1;
	//			}
	//			else
	//			{
	//				Izr = root2;
	//			}
	//			Iend21 = (Is + (Izr - Izs)* sigma);
	//			/*	  cout<<"compress: "<<compress<<endl<<endl;
	//			cout<<"sigma: "<<sigma<<endl<<endl;
	//			cout<<"msigma: "<<msigma<<endl<<endl;
	//			cout<<"Iend21: "<<Iend21<<endl<<endl;
	//			cout<<"Ec: "<<Ec<<endl<<endl;
	//			cout<<"Izc: "<<Izc<<endl<<endl;
	//			cout<<"Izs: "<<Izs<<endl<<endl;
	//			cout<<"Phi1Izc: "<< Phi1( ImpactClosedForm2Vnormaln,  nhat,  Is,  Izs,  S,  sigma,  msigma, Izc)<<endl<<endl;
	//			cout<<"a0: "<<a0<<endl<<endl;
	//			cout<<"a1: "<<a1<<endl<<endl;
	//			cout<<"a2: "<<a2<<endl<<endl;
	//			cout<<"root1: "<<root1<<endl<<endl;
	//			cout<<"root2: "<<root2<<endl<<endl;
	//			for(int k=0;k<50;k++)
	//			{
	//			cout<<stepSizeInCount[k]<<endl;
	//			}
	//			cout<<endl<<endl;*/

	//		}
	//		else
	//		{
	//			double b0 = Es - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
	//			double root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//			double root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//			if (root3 > Izs && root4 > Izs)
	//			{
	//				Izr = root3<root4 ? root3 : root4;
	//			}
	//			else if (root3 > Izs)
	//			{
	//				Izr = root3;
	//			}
	//			else
	//			{
	//				Izr = root4;
	//			}
	//			Iend22 = (Is + (Izr - Izs)*sigma);
	//		}

	//	}
	//	//////////////////////////////////situation3
	//	if (situation == 3)
	//	{
	//		double Izl, El, q1, q2, q3, q4, lambda, DeltaIz, mdelta, Chatac, Chatae;
	//		Vector2d gammal, temp6;
	//		Vector3d temp5, Il, delta;
	//		Izl = IIn[count];
	//		temp5 << IIn[count], IIu[count], IIw[count];
	//		Il = RTAN*temp5;
	//		El = EE[count];
	//		gammal << vvu[count], vvw[count];
	//		q3 = vvu[count] / gammal.norm(); q4 = vvw[count] / gammal.norm();
	//		temp6 << q3, q4;
	//		q1 = (-mu*BNew*temp6 / temp6.norm() + c)(0);
	//		q2 = (-mu*BNew*temp6 / temp6.norm() + c)(1);
	//		if (abs(q1)<1e-6  &&  abs(q2) >DEFAULT_ACCURACY && abs(q3) < 1e-6 && abs(q4) >DEFAULT_ACCURACY)
	//		{
	//			lambda = q2 / q4;
	//		}
	//		else if ((abs(q1) >DEFAULT_ACCURACY && abs(q2) < 1e-6 && abs(q3) >DEFAULT_ACCURACY && abs(q4) < 1e-6))
	//		{
	//			lambda = q1 / q3;
	//		}
	//		else if ((abs(q1) >DEFAULT_ACCURACY && abs(q2) >DEFAULT_ACCURACY && abs(q3) >DEFAULT_ACCURACY && abs(q4) >DEFAULT_ACCURACY))
	//		{
	//			lambda = q1 / q3;
	//		}
	//		else if ((abs(q1) < 1e-6 && abs(q2) < 1e-6 && abs(q3) < 1e-6 && abs(q4) < 1e-6))
	//		{
	//			lambda = 100000000.0;
	//		}
	//		else
	//		{
	//			lambda = 0.0000000001;
	//		}
	//		///
	//		DeltaIz = lambda>0 ? 100000000 : -gammal.norm() / lambda;
	//		delta = nhat - mu*uwJuzhen*(gammal / gammal.norm());
	//		Izs = Izl + DeltaIz;
	//		mdelta = 1 / (1 / m1 + 1 / m2 - (nhat.transpose()*S*delta));
	//		Chatac = mdelta*(-ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Il - Izl*delta)));

	//		if (compress == 1)
	//		{
	//			double a0, a1, a2, root1, root2, d0, d1, d2, root5, root6, b0, root3, root4;
	//			if (Izs < Chatac)
	//			{
	//				Es = Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izs); Is = Il + (Izs - Izl)*delta;
	//				a2 = -1 / (2 * msigma);
	//				a1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
	//				Izc = msigma* (-ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma)));
	//				Ec = Es + Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izc) - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
	//				a0 = e*e*Ec - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izc);
	//				root1 = (-a1 - sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2); root2 = (-a1 + sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);
	//				if (root1 > Izc && root2 > Izc)
	//				{
	//					Izr = root1<root2 ? root1 : root2;
	//				}
	//				else if (root1 > Izc)
	//				{
	//					Izr = root1;
	//				}
	//				else
	//				{
	//					Izr = root2;
	//				}

	//				Iend31 = (Is + (Izr - Izs)* sigma);
	//				cout << "Iend31: " << Iend31 << endl << endl;
	//			}
	//			else
	//			{
	//				Izc = Chatac; Ec = Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
	//				d2 = -1 / (2 * mdelta); d1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Il - Izl*delta));
	//				d0 = e*e*Ec - Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
	//				root5 = (-d1 - sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);
	//				root6 = (-d1 + sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);

	//				if (root5 > Chatac && root6 > Chatac)
	//				{
	//					Chatae = root5<root6 ? root5 : root6;

	//				}
	//				else if (root5 > Chatac)
	//				{
	//					Chatae = root5;
	//				}
	//				else
	//				{
	//					Chatae = root6;
	//				}

	//				if (Izs<Chatae)
	//				{
	//					Es = e*e *Ec + Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izs) - Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
	//					Is = Il + (Izs - Izl)*delta;
	//					a2 = -1 / (2 * msigma);
	//					a1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
	//					b0 = Es - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
	//					root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//					root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//					if (root3 > Izs && root4 > Izs)
	//					{
	//						Izr = root3<root4 ? root3 : root4;
	//					}
	//					else if (root3 > Izs)
	//					{
	//						Izr = root3;
	//					}
	//					else
	//					{
	//						Izr = root4;
	//					}
	//					Iend32a = (Is + (Izr - Izs)*sigma);
	//				}
	//				else
	//				{
	//					Izr = Chatae;
	//					Iend32b = Il + (Izr - Izl)*delta;
	//					cout << "Iend32b: " << Iend32b << endl << endl;
	//				}

	//			}

	//		}
	//		if (compress == 0)
	//		{
	//			double a0, a1, a2, root1, root2, d0, d1, d2, root5, root6, b0, root3, root4;
	//			d2 = -1 / (2 * mdelta);
	//			d1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Il - Izl* delta));
	//			d0 = El - Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izl);
	//			root5 = (-d1 - sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2); root6 = (-d1 + sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);
	//			if (root5 > Chatac && root6 > Chatac)
	//			{
	//				Chatae = root5<root6 ? root5 : root6;
	//			}
	//			else if (root5 > Chatac)
	//			{
	//				Chatae = root5;
	//			}
	//			else
	//			{
	//				Chatae = root6;
	//			}

	//			if (Izs < Chatae)
	//			{
	//				Es = El + Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izs) - Phi2(ImpactClosedForm2Vnormaln, nhat, Il, Izl, S, delta, mdelta, Izl);
	//				Is = Il + (Izs - Izl)*delta;
	//				a2 = -1 / (2 * msigma);
	//				a1 = -ImpactClosedForm2Vnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
	//				b0 = Es - Phi1(ImpactClosedForm2Vnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
	//				root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//				root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
	//				if (root3 > Izs && root4 > Izs)
	//				{
	//					Izr = root3<root4 ? root3 : root4;
	//				}
	//				else if (root3 > Izs)
	//				{
	//					Izr = root3;
	//				}
	//				else
	//				{
	//					Izr = root4;
	//				}
	//				Iend33 = (Is + (Izr - Izs)* sigma);
	//			}
	//			else
	//			{
	//				Izr = Chatae;
	//				Iend34 = Il + (Izr - Izl)* delta;
	//			}


	//		}



	//	}


	//	if (countSlide == 0) countSlide = finish;
	//	cout << "at the end sliding1/sticking0?: " << sliding << endl;
	//	cout << "finish: " << finish << endl;
	//	cout << "countCom: " << countCom << endl;
	//	cout << "countSlide: " << countSlide << endl << endl;

	//	//compute the post velocity and the post angular velocity:
	//	if (Iend1.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.2Case1,Iend1" << endl << endl;
	//		Iend = Iend1;
	//	}
	//	else if (Iend21.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.2Case21,Iend21" << endl << endl;
	//		Iend = Iend21;
	//	}
	//	else if (Iend22.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.2Case22,Iend22" << endl << endl;
	//		Iend = Iend22;
	//	}
	//	else if (Iend31.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.3Case1,Iend31" << endl << endl;
	//		Iend = Iend31;
	//	}
	//	else if (Iend32a.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.3Case2a,Iend32a" << endl << endl;
	//		Iend = Iend32a;
	//	}
	//	else if (Iend32b.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.3Case2b,Iend32b" << endl << endl;
	//		Iend = Iend32b;
	//	}
	//	else if (Iend33.norm()>DEFAULT_ACCURACY)
	//	{
	//		cout << "Sc6.3Case3,Iend33" << endl << endl;
	//		Iend = Iend33;
	//	}
	//	else
	//	{
	//		cout << "Sc6.3Case4,Iend34" << endl << endl;
	//		Iend = Iend34;
	//	}
	//	// Vector3d IendTmp; It's already been in the world frame.
	//	//IendTmp=Iend;
	//	//Iend=RTAN*IendTmp;//because the coordinate system is always x-w, y-n, z-u

	//	v1end = v1 + (1 / m1)*Iend;
	//	v2end = v2 - (1 / m2)*Iend;

	//	w1end = w1 + Q1.inverse()*r1.cross(R1.inverse()*Iend);
	//	w2end = w2 - Q2.inverse()*r2.cross(R2.inverse()*Iend);

	//}
};

