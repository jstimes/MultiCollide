#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>  
#include<vector>

#include <Eigen/Dense>
#include "MathUtils.h"

const int NumIter = 2000;
const double constantStep = 0.005;
//const bool DEBUG = false;

struct ImpactOutput
{
	glm::vec3 v1end, v2end, w1end, w2end, Iend;
	std::vector<glm::vec3> impulsePts;
	std::vector<glm::vec3> velocityPts;
	int endOfSliding;
	int endOfCompression;
};

using namespace Eigen;
class Impact
{

public:

	std::string s1name;
	std::string s2name;

	double m1, m2, mu, e;
	Eigen::Matrix3d R1, R2, Q1, Q2, RTAN, P1, P2;
	Eigen::Vector3d r1, r2, v1, v2, w1, w2, nhat, uhat, ohat;

	Eigen::Vector3d v1end, v2end, w1end, w2end;

	//Wrapper constructor that converts glm data types to Eigen, which is what's used by the program
	Impact(float m1, float m2, float mu, float e, glm::mat3 &R1, glm::mat3 &R2, glm::mat3 &Q1, glm::mat3 &Q2, glm::mat3 &RTAN,
		glm::vec3 &r1, glm::vec3 &r2, glm::vec3 &v1, glm::vec3 &v2, glm::vec3 &w1, glm::vec3 &w2) {


		this->m1 = m1;
		this->m2 = m2;
		this->e = e;
		this->mu = mu;
		//this->StepsizeIn = StepsizeIn;
		this->R1 = MathUtils::glmToEigenMatrix(R1);
		this->R2 = MathUtils::glmToEigenMatrix(R2);
		this->Q1 = MathUtils::glmToEigenMatrix(Q1);
		this->Q2 = MathUtils::glmToEigenMatrix(Q2);
		this->RTAN = MathUtils::glmToEigenMatrix(RTAN);
		this->r1 = MathUtils::glmToEigenVector(r1);
		this->r2 = MathUtils::glmToEigenVector(r2);
		this->v1 = MathUtils::glmToEigenVector(v1);
		this->v2 = MathUtils::glmToEigenVector(v2);
		this->w1 = MathUtils::glmToEigenVector(w1);
		this->w2 = MathUtils::glmToEigenVector(w2);
		P1 = getPmatrix(this->r1);
		P2 = getPmatrix(this->r2);
		nhat = this->RTAN * Eigen::Vector3d(1.0, 0.0, 0.0);//normal vector, currently is x direction
		uhat = this->RTAN * Eigen::Vector3d(0.0, 1.0, 0.0); //unit vector in tangential plane, y direction 
		ohat = this->RTAN * Eigen::Vector3d(0.0, 0.0, 1.0); //unit vector in tangential plane, happen to be z direction.


															//if(DEBUG) std::cout << r1 << std::endl;
															//if(DEBUG) std::cout << r2 << std::endl;
															//if(DEBUG) std::cout << v1 << std::endl;
															//if(DEBUG) std::cout << v2 << std::endl;
	}

	Impact(float m1, float m2, float mu, float e, Eigen::Matrix3d &R1, Eigen::Matrix3d &R2, Eigen::Matrix3d &Q1, Eigen::Matrix3d &Q2, Eigen::Matrix3d &RTAN,
		Eigen::Vector3d &r1, Eigen::Vector3d &r2, Eigen::Vector3d &v1, Eigen::Vector3d &v2, Eigen::Vector3d &w1, Eigen::Vector3d &w2)
	{
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
		P1 = getPmatrix(this->r1);
		P2 = getPmatrix(this->r2);
		nhat = this->RTAN * Eigen::Vector3d(1.0, 0.0, 0.0);//normal vector, currently is x direction
		uhat = this->RTAN * Eigen::Vector3d(0.0, 1.0, 0.0); //unit vector in tangential plane, y direction 
		ohat = this->RTAN * Eigen::Vector3d(0.0, 0.0, 1.0); //unit vector in tangential plane, happen to be z direction.


															//if(DEBUG) std::cout << r1 << std::endl;
															//if(DEBUG) std::cout << r2 << std::endl;
															//if(DEBUG) std::cout << v1 << std::endl;
															//if(DEBUG) std::cout << v2 << std::endl;
	}

	Eigen::Matrix3d getPmatrix(Eigen::Vector3d r);

	ImpactOutput impact()
	{
		using namespace std;

		double initialVperpu, initialVperpw, initialVnormaln;
		Matrix3d Rot, S, A, Identity3;
		Vector3d initialVperp, initialVnormal;
		MatrixXd uwJuzhen(3, 2);
		Matrix2d B, BNew;
		Vector2d c;
		S = R1*P1*Q1.inverse()*P1*R1.inverse() + R2*P2*Q2.inverse()*P2*R2.inverse();
		uwJuzhen << uhat[0], ohat[0],
			uhat[1], ohat[1],
			uhat[2], ohat[2];
		Identity3 << 1.0, 0.0, 0.0,
			0.0, 1.0, 0.0,
			0.0, 0.0, 1.0;
		A = (Identity3 - nhat*nhat.transpose())*((1.0 / m1 + 1.0 / m2)*Identity3 - S);
		initialVperp = -((Identity3 - nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (Identity3 - nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
		initialVperpu = initialVperp.transpose()*uhat;
		initialVperpw = initialVperp.transpose()*ohat;
		initialVnormal = -((nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
		initialVnormaln = initialVnormal.transpose()*nhat;
		B = (-mu)*uwJuzhen.transpose()*A*uwJuzhen;
		double B11, B12, B21, B22;
		B11 = B(0, 0); B12 = B(0, 1); B21 = B(1, 0); B22 = B(1, 1);//the elements of B matrix
		BNew = uwJuzhen.transpose()*A*uwJuzhen;//This is the new definition of B in the note that does not have mu in it
		c = uwJuzhen.transpose()*A*nhat;
		double c1, c2;
		c1 = c(0); c2 = c(1);

		//start initialization part for the numerical integration using the Euler's method.
		double StepsizeIn = constantStep;
		double vvu[NumIter], vvw[NumIter], vvn[NumIter], IIu[NumIter], IIw[NumIter], IIn[NumIter], EE[NumIter], curvatureOfV[NumIter];
		for (int i = 0; i<NumIter; i++)
		{
			vvu[i] = 0;
			vvw[i] = 0;
			vvn[i] = 0;
			IIu[i] = 0;
			IIw[i] = 0;
			IIn[i] = 0;
			EE[i] = 0;
			curvatureOfV[i] = 0;
		}
		vvu[0] = initialVperpu;
		vvw[0] = initialVperpw;
		vvn[0] = initialVnormaln;
		int count = 0; int finish = 0; //finish is the total steps for calc, <NumIter

		int compress = 1;//(*compression=1 means compress, =0 means restitution*)
		int sliding = 1;
		if (vvu[count] * vvu[count] + vvw[count] * vvw[count]>pow(10.0, -6))//(*sliding=1 means slide, sliding=0 means sticking*)
		{
			sliding = 1;
			//printf("this is the case\n");
		}
		else
		{
			sliding = 0;
		}

		double top = 0.0; double bottom = 1.0; double x = 0.0; double y = 0.0; double valueofYPrime = 0.0;//curvature related calculation

		int countCom = 0; int countSlide = 0;//either 0 or 1, different status,comp means compression
		vvu[0] = initialVperpu; vvw[0] = initialVperpw;
		vvn[0] = initialVnormaln; EE[0] = 0.0;
		double tempVVN = vvn[0];


		while (count<NumIter - 1)
		{
			if (EE[count]<-0.0000000000001)//checking for finish condition
			{
				finish = count;
				break;
			}
			if (compress == 1 && tempVVN*vvn[count]<0.000001)//checking for finishing compression phase, the sign of vn(which is normal velocity) changes
			{
				compress = 0;
				countCom = count;
				EE[count] = e*e*EE[count];
			}
			//Now coompute the curvature related thing to determine the stepsize
			x = vvu[count];
			y = vvw[count];

			if ((B11*x + B12*y + c1*sqrt(x*x + y*y)) != 0)
			{
				valueofYPrime = (B21*x + B22*y + c2*sqrt(x*x + y*y)) / (B11*x + B12*y + c1*sqrt(x*x + y*y));
			}
			else
			{
				bottom = 0.0;
			}

			if ((sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y))) != 0)
			{
				top = (((B21*c1 - B11*c2)*y - B22*(c1*x + B11*sqrt(x*x + y*y)) + B12*(c2*x + B21*sqrt(x*x + y*y)))*(y - x*valueofYPrime)) / (sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)));
			}
			else
			{
				bottom = 0;
			}
			bottom = pow(1.0 + valueofYPrime*valueofYPrime, 1.5);

			if (bottom != 0)
			{
				curvatureOfV[count] = abs(top / bottom);
			}
			else
			{
				curvatureOfV[count] = 0.0;
			}

			if (sliding == 0)
			{
				curvatureOfV[count] = 0.0;
			}
			//change different step sizes
			if (curvatureOfV[count]<0.05)
			{
				StepsizeIn = constantStep;
			}
			else if (curvatureOfV[count]<0.5 && curvatureOfV[count] >= 0.05)
			{
				StepsizeIn = constantStep / 2.0;
			}
			else if (curvatureOfV[count]<2.5 && curvatureOfV[count] >= 0.5)
			{
				StepsizeIn = constantStep / 5.0;
			}
			else
			{
				StepsizeIn = constantStep / 10.0;
			}


			if (sliding == 0)StepsizeIn = constantStep;
			//Now determine the end of sliding
			if (sliding == 1 && sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count])<0.001)//sliding to sticking state might happen
			{
				if ((BNew.inverse()*c).squaredNorm()<mu*mu)//sliding change to sticking
				{
					sliding = 0;
					countSlide = count;
				}// else still compute everything in the same method as sliding

			}


			if (sliding == 1)//update all the part in the sliding situation
			{
				tempVVN = vvn[count];
				count++;
				IIn[count] = IIn[count - 1] + StepsizeIn;

				//below is the method for updating these things
				Vector2d temp5, temp6;
				temp5 << vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]),
					vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
				temp6 = B*temp5;
				vvu[count] = vvu[count - 1] + StepsizeIn*(c[0] + temp6[0]);
				vvw[count] = vvw[count - 1] + StepsizeIn*(c[1] + temp6[1]);
				IIu[count] = IIu[count - 1] + StepsizeIn*(-1.0*mu)*vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
				IIw[count] = IIw[count - 1] + StepsizeIn*(-1.0*mu)*vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);

				MatrixXd temp7(1, 3), temp8(3, 1);
				double temp9;
				temp7 = nhat.transpose()*S*RTAN;
				temp8 << IIn[count],
					IIu[count],
					IIw[count];
				temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
				vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
				EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
			}

			else if (sliding == 0)//update all the part in the sticking situation
			{
				count++;
				IIn[count] = IIn[count - 1] + StepsizeIn;
				IIu[count] = IIu[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(0);
				IIw[count] = IIw[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(1);
				vvu[count] = 0;
				vvw[count] = 0;
				MatrixXd temp7(1, 3), temp8(3, 1);
				double temp9;
				temp7 = nhat.transpose()*S*RTAN;
				temp8 << IIn[count],
					IIu[count],
					IIw[count];
				temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
				vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
				EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
			}

		}

		/*for (int i = 0; i < count; i++) {
		std::cout << IIn[i] << ", " << IIu[i] << ", " << IIw[i] << std::endl;
		}*/

		/*  int iiter=0;
		cout<<"the values of IIn are: "<<endl;
		while(iiter<NumIter)
		{
		for(int j=1;j<=10;j++)
		{
		cout<<IIn[iiter]<<" ";
		iiter++;
		}
		cout<<endl;
		}*/
		if (countSlide == 0) countSlide = finish;
		cout << "at the end sliding1/sticking0?: " << sliding << endl;
		cout << "finish: " << finish << endl;
		cout << "countCom: " << countCom << endl;
		cout << "countSlide: " << countSlide << endl;

		//compute the post velocity and the post angular velocity:
		Vector3d Iend;
		Iend << RTAN*Vector3d(IIn[finish], IIu[finish], IIw[finish]);//because the coordinate system is always x-w, y-n, z-u

		Iend = -Iend;

		v1end = v1 + (1 / m1)*Iend;
		v2end = v2 - (1 / m2)*Iend;

		w1end = w1 + Q1.inverse()*r1.cross(R1.inverse()*Iend);
		w2end = w2 - Q2.inverse()*r2.cross(R2.inverse()*Iend);

		/*  cout<<"Iend: "<<endl<<Iend<<endl<<endl;
		cout<<"v1end: "<<endl<<v1end<<endl<<endl;
		cout<<"v2end: "<<endl<<v2end<<endl<<endl;
		cout<<"w1end: "<<endl<<w1end<<endl<<endl;
		cout<<"w2end: "<<endl<<w2end<<endl<<endl;*/

		std::vector<glm::vec3> impulsePts;
		std::vector<glm::vec3> velocityPts;
		// -1 ?
		for (int i = 0; i < count; i++) {
			impulsePts.push_back(glm::vec3(IIn[i], IIu[i], IIw[i]));
			velocityPts.push_back(glm::vec3(vvn[i], vvu[i], vvw[i]));
		}

		ImpactOutput output = { MathUtils::eigenToGlmVector(v1end), MathUtils::eigenToGlmVector(v2end),
			MathUtils::eigenToGlmVector(w1end), MathUtils::eigenToGlmVector(w2end), MathUtils::eigenToGlmVector(Iend), impulsePts,
			velocityPts, countSlide, countCom };

		return output;
	}

	//Pre-coord system changes:

	//ImpactOutput impact()
	//{
	//	//double initialVperpu, initialVperpw, initialVnormaln;
	//	//Eigen::Matrix3d Rot, S, A, Identity3;
	//	//Eigen::Vector3d initialVperp, initialVnormal;
	//	//Eigen::MatrixXd uwJuzhen(3, 2);
	//	//Eigen::Matrix2d B, BNew;
	//	//Eigen::Vector2d c;
	//	//S = R1*P1*Q1.inverse()*P1*R1.inverse() + R2*P2*Q2.inverse()*P2*R2.inverse();
	//	//uwJuzhen << uhat[0], ohat[0],
	//	//	uhat[1], ohat[1],
	//	//	uhat[2], ohat[2];
	//	//Identity3 << 1, 0, 0,
	//	//	0, 1, 0,
	//	//	0, 0, 1;
	//	//A = (Identity3 - nhat*nhat.transpose())*((1.0 / m1 + 1.0 / m2)*Identity3 - S);
	//	////cout<<"A: "<<A<<endl;
	//	//initialVperp = -((Identity3 - nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (Identity3 - nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	////cout<<"initialVperp: "<<initialVperp<<endl;
	//	//initialVperpu = initialVperp.transpose()*uhat;
	//	//initialVperpw = initialVperp.transpose()*ohat;
	//	//initialVnormal = -((nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	////cout<<"initialVnormal: "<<initialVnormal<<endl;
	//	//initialVnormaln = initialVnormal.transpose()*nhat;
	//	//B = (-mu)*uwJuzhen.transpose()*A*uwJuzhen;
	//	//double B11, B12, B21, B22;
	//	//B11 = B(0, 0); B12 = B(0, 1); B21 = B(1, 0); B22 = B(1, 1);//the elements of B matrix
	//	//BNew = uwJuzhen.transpose()*A*uwJuzhen;//This is the new definition of B in the note that does not have mu in it
	//	//c = uwJuzhen.transpose()*A*nhat;
	//	//double c1, c2;
	//	//c1 = c(0); c2 = c(1);

	//	////start initialization part for the numerical integration using the Euler's method.

	//	//double vvu[NumIter], vvw[NumIter], vvn[NumIter], IIu[NumIter], IIw[NumIter], IIn[NumIter], EE[NumIter], curvatureOfV[NumIter];
	//	//for (int i = 0; i<NumIter; i++)
	//	//{
	//	//	vvu[i] = 0;
	//	//	vvw[i] = 0;
	//	//	vvn[i] = 0;
	//	//	IIu[i] = 0;
	//	//	IIw[i] = 0;
	//	//	IIn[i] = 0;
	//	//	EE[i] = 0;
	//	//	curvatureOfV[i] = 0;
	//	//}
	//	//vvu[0] = initialVperpu;
	//	//vvw[0] = initialVperpw;
	//	//vvn[0] = initialVnormaln;
	//	//int count = 0; int finish = 0; //finish is the total steps for calc, <NumIter
	//	//int compress = 1;//(*compression=1 means compress, =0 means restitution*)
	//	//int sliding = 1;
	//	//if (vvu[count] * vvu[count] + vvw[count] * vvw[count]>pow(10.0, -6))//(*sliding=1 means slide, sliding=0 means sticking*)
	//	//{
	//	//	sliding = 1;
	//	//	//printf("this is the case\n");
	//	//}
	//	//else
	//	//{
	//	//	sliding = 0;
	//	//}

	//	////double pp = 0; double qq = 0; double zz = 0;double ll=0;//no longer needed, case rank(MM)=1
	//	//double top = 0.0; double bottom = 1.0; double x = 0.0; double y = 0.0; double valueofYPrime = 0.0;//curvature related calculation

	//	//int countCom = 0; int countSlide = 0;//either 0 or 1, different status,comp means compression
	//	//vvu[0] = initialVperpu; vvw[0] = initialVperpw;
	//	//vvn[0] = initialVnormaln; EE[0] = 0.0;
	//	//double tempVVN = vvn[0];


	//	//while (count<NumIter)
	//	//{
	//	//	//cout<<"value of sliding is: "<<sliding<<endl;
	//	//	if (EE[count]<0.0)//checking for finish condition
	//	//	{
	//	//		finish = count;
	//	//		break;
	//	//	}
	//	//	if (compress == 1 && tempVVN*vvn[count]<0.000001)//checking for finishing compression phase, the sign of vn(which is normal velocity) changes
	//	//	{
	//	//		compress = 0;
	//	//		countCom = count;
	//	//		EE[count] = e*e*EE[count];
	//	//	}
	//	//	//Now coompute the curvature related thing to determine the stepsize
	//	//	x = vvu[count];
	//	//	y = vvw[count];

	//	//	if ((B11*x + B12*y + c1*sqrt(x*x + y*y)) != 0)
	//	//	{
	//	//		valueofYPrime = (B21*x + B22*y + c2*sqrt(x*x + y*y)) / (B11*x + B12*y + c1*sqrt(x*x + y*y));
	//	//	}
	//	//	else
	//	//	{
	//	//		bottom = 0.0;
	//	//	}

	//	//	if ((sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y))) != 0)
	//	//	{
	//	//		top = (((B21*c1 - B11*c2)*y - B22*(c1*x + B11*sqrt(x*x + y*y)) + B12*(c2*x + B21*sqrt(x*x + y*y)))*(y - x*valueofYPrime)) / (sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)));
	//	//	}
	//	//	else
	//	//	{
	//	//		bottom = 0;
	//	//	}
	//	//	bottom = pow(1.0 + valueofYPrime*valueofYPrime, 1.5);

	//	//	if (bottom != 0)
	//	//	{
	//	//		curvatureOfV[count] = abs(top / bottom);
	//	//		//cout<<count<<": "<<curvatureOfV[count]<<endl;
	//	//	}
	//	//	else
	//	//	{
	//	//		curvatureOfV[count] = 0.0;
	//	//	}

	//	//	if (sliding == 0)
	//	//	{
	//	//		curvatureOfV[count] = 0.0;
	//	//	}
	//	//	//change different step sizes
	//	//	if (curvatureOfV[count]<0.05)
	//	//	{
	//	//		StepsizeIn = 0.05;
	//	//	}
	//	//	else if (curvatureOfV[count]<0.5 && curvatureOfV[count] >= 0.05)
	//	//	{
	//	//		StepsizeIn = 0.03;
	//	//	}
	//	//	else if (curvatureOfV[count]<2.5 && curvatureOfV[count] >= 0.5)
	//	//	{
	//	//		StepsizeIn = 0.025;
	//	//	}
	//	//	else
	//	//	{
	//	//		StepsizeIn = 0.0025;
	//	//	}


	//	//	if (sliding == 0)StepsizeIn = 0.005;
	//	//	//Now determine the end of sliding
	//	//	if (sliding == 1 && sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count])<0.001)//sliding to sticking state might happen
	//	//	{
	//	//		if ((BNew.inverse()*c).squaredNorm()<mu*mu)//sliding change to sticking
	//	//		{
	//	//			sliding = 0;
	//	//			countSlide = count;
	//	//		}// else still compute everything in the same method as sliding

	//	//	}


	//	//	if (sliding == 1)//update all the part in the sliding situation
	//	//	{
	//	//		tempVVN = vvn[count];
	//	//		count++;
	//	//		IIn[count] = IIn[count - 1] + StepsizeIn;

	//	//		//below is the method for updating these things
	//	//		Eigen::Vector2d temp5, temp6;
	//	//		temp5 << vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]),
	//	//			vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//	//		temp6 = B*temp5;
	//	//		vvu[count] = vvu[count - 1] + StepsizeIn*(c[0] + temp6[0]);
	//	//		vvw[count] = vvw[count - 1] + StepsizeIn*(c[1] + temp6[1]);
	//	//		IIu[count] = IIu[count - 1] + StepsizeIn*(-1.0*mu)*vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//	//		IIw[count] = IIw[count - 1] + StepsizeIn*(-1.0*mu)*vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);





	//	//		Eigen::MatrixXd temp7(1, 3), temp8(3, 1);
	//	//		double temp9;
	//	//		temp7 = nhat.transpose()*S*RTAN;
	//	//		temp8 << IIw[count],
	//	//			IIn[count],
	//	//			IIu[count];
	//	//		temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
	//	//		vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
	//	//		EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	//	//	}

	//	//	else if (sliding == 0)//update all the part in the sticking situation
	//	//	{
	//	//		count++;
	//	//		IIn[count] = IIn[count - 1] + StepsizeIn;
	//	//		//IIu[count]=IIu[count-1]+StepsizeIn*alphaM;
	//	//		//IIw[count]=IIw[count-1]+StepsizeIn*betaM;
	//	//		IIu[count] = IIu[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(0);
	//	//		IIw[count] = IIw[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(1);
	//	//		vvu[count] = 0;
	//	//		vvw[count] = 0;
	//	//		Eigen::MatrixXd temp7(1, 3), temp8(3, 1);
	//	//		double temp9;
	//	//		temp7 = nhat.transpose()*S*RTAN;
	//	//		temp8 << IIw[count],
	//	//			IIn[count],
	//	//			IIu[count];
	//	//		temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
	//	//		vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
	//	//		EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	//	//	}

	//	//}
	//	//int iiter = 0;
	//	//std::cout << "the values of IIu are: " << std::endl;
	//	//while (iiter<NumIter)
	//	//{
	//	//	for (int j = 1; j <= 10; j++)
	//	//	{
	//	//		//cout<<EE[iiter]<<" ";
	//	//		std::cout << IIn[iiter] << " ";
	//	//		iiter++;
	//	//	}
	//	//	std::cout << std::endl;
	//	//}
	//	//if (countSlide == 0) countSlide = finish;
	//	//std::cout << "at the end sliding1/sticking0?: " << sliding << std::endl;
	//	//std::cout << "finish: " << finish << std::endl;
	//	//std::cout << "countCom: " << countCom << std::endl;
	//	//std::cout << "countSlide: " << countSlide << std::endl;

	//	////compute the post velocity and the post angular velocity:
	//	//
	//	//Eigen::Vector3d v1end, v2end, w1end, w2end, Iend;
	//	//Iend = RTAN * Eigen::Vector3d( IIw[finish], IIn[finish], IIu[finish]);//because the coordinate system is always x-w, y-n, z-u
	//	//Iend = -Iend;
	//	//v1end = v1 + (1 / m1)*Iend;
	//	//v2end = v2 - (1 / m2)*Iend;

	//	//w1end = w1 + Q1.inverse()*r1.cross(R1.inverse()*Iend);
	//	//w2end = w2 - Q2.inverse()*r2.cross(R2.inverse()*Iend);

	//	//std::cout << "Iend: " << std::endl << Iend << std::endl << std::endl;
	//	//std::cout << "v1end: " << std::endl << v1end << std::endl << std::endl;
	//	//std::cout << "v2end: " << std::endl << v2end << std::endl << std::endl;
	//	//std::cout << "w1end: " << std::endl << w1end << std::endl << std::endl;
	//	//std::cout << "w2end: " << std::endl << w2end << std::endl << std::endl;

	//	double initialVperpu, initialVperpw, initialVnormaln;
	//	Eigen::Matrix3d Rot, S, A, Identity3;
	//	Eigen::Vector3d initialVperp, initialVnormal;
	//	Eigen::MatrixXd uwJuzhen(3, 2);
	//	Eigen::Matrix2d B, BNew;
	//	Eigen::Vector2d c;
	//	S = R1*P1*Q1.inverse()*P1*R1.inverse() + R2*P2*Q2.inverse()*P2*R2.inverse();
	//	uwJuzhen << uhat[0], ohat[0],
	//	uhat[1], ohat[1],
	//	uhat[2], ohat[2];
	//	Identity3 << 1.0, 0.0, 0.0,
	//	0.0, 1.0, 0.0,
	//	0.0, 0.0, 1.0;
	//	A = (Identity3 - nhat*nhat.transpose())*((1.0 / m1 + 1.0 / m2)*Identity3 - S);
	//	initialVperp = -((Identity3 - nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (Identity3 - nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	initialVperpu = initialVperp.transpose()*uhat;
	//	initialVperpw = initialVperp.transpose()*ohat;
	//	initialVnormal = -((nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	//	initialVnormaln = initialVnormal.transpose()*nhat;
	//	B = (-mu)*uwJuzhen.transpose()*A*uwJuzhen;
	//	double B11, B12, B21, B22;
	//	B11 = B(0, 0); B12 = B(0, 1); B21 = B(1, 0); B22 = B(1, 1);//the elements of B matrix
	//	BNew = uwJuzhen.transpose()*A*uwJuzhen;//This is the new definition of B in the note that does not have mu in it
	//	c = uwJuzhen.transpose()*A*nhat;
	//	double c1, c2;
	//	c1 = c(0); c2 = c(1);

	//	//start initialization part for the numerical integration using the Euler's method.
	//	double StepsizeIn = constantStep;
	//	double vvu[NumIter+1], vvw[NumIter + 1], vvn[NumIter + 1], IIu[NumIter + 1], IIw[NumIter + 1], IIn[NumIter + 1], EE[NumIter + 1], curvatureOfV[NumIter + 1];
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
	//	vvu[0] = initialVperpu;
	//	vvw[0] = initialVperpw;
	//	vvn[0] = initialVnormaln;
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

	//	double top = 0.0; double bottom = 1.0; double x = 0.0; double y = 0.0; double valueofYPrime = 0.0;//curvature related calculation

	//	int countCom = 0; int countSlide = 0;//either 0 or 1, different status,comp means compression
	//	vvu[0] = initialVperpu; vvw[0] = initialVperpw;
	//	vvn[0] = initialVnormaln; EE[0] = 0.0;
	//	double tempVVN = vvn[0];


	//	while (count<NumIter)
	//	{
	//		if (EE[count]<-0.0000000000001)//checking for finish condition
	//		{
	//			finish = count;
	//			break;
	//		}
	//		if (compress == 1 && tempVVN*vvn[count]<0.000001)//checking for finishing compression phase, the sign of vn(which is normal velocity) changes
	//		{
	//			compress = 0;
	//			countCom = count;
	//			EE[count] = e*e*EE[count];
	//		}
	//		//Now coompute the curvature related thing to determine the stepsize
	//		x = vvu[count];
	//		y = vvw[count];

	//		if ((B11*x + B12*y + c1*sqrt(x*x + y*y)) != 0)
	//		{
	//			valueofYPrime = (B21*x + B22*y + c2*sqrt(x*x + y*y)) / (B11*x + B12*y + c1*sqrt(x*x + y*y));
	//		}
	//		else
	//		{
	//			bottom = 0.0;
	//		}

	//		if ((sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y))) != 0)
	//		{
	//			top = (((B21*c1 - B11*c2)*y - B22*(c1*x + B11*sqrt(x*x + y*y)) + B12*(c2*x + B21*sqrt(x*x + y*y)))*(y - x*valueofYPrime)) / (sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)));
	//		}
	//		else
	//		{
	//			bottom = 0;
	//		}
	//		bottom = pow(1.0 + valueofYPrime*valueofYPrime, 1.5);

	//		if (bottom != 0)
	//		{
	//			curvatureOfV[count] = abs(top / bottom);
	//		}
	//		else
	//		{
	//			curvatureOfV[count] = 0.0;
	//		}

	//		if (sliding == 0)
	//		{
	//			curvatureOfV[count] = 0.0;
	//		}
	//		//change different step sizes
	//		if (curvatureOfV[count]<0.05)
	//		{
	//			StepsizeIn = constantStep;
	//		}
	//		else if (curvatureOfV[count]<0.5 && curvatureOfV[count] >= 0.05)
	//		{
	//			StepsizeIn = constantStep / 2.0;
	//		}
	//		else if (curvatureOfV[count]<2.5 && curvatureOfV[count] >= 0.5)
	//		{
	//			StepsizeIn = constantStep / 5.0;
	//		}
	//		else
	//		{
	//			StepsizeIn = constantStep / 10.0;
	//		}


	//		if (sliding == 0)StepsizeIn = constantStep;
	//		//Now determine the end of sliding
	//		if (sliding == 1 && sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count])<0.001)//sliding to sticking state might happen
	//		{
	//			if ((BNew.inverse()*c).squaredNorm()<mu*mu)//sliding change to sticking
	//			{
	//				sliding = 0;
	//				countSlide = count;
	//			}// else still compute everything in the same method as sliding

	//		}


	//		if (sliding == 1)//update all the part in the sliding situation
	//		{
	//			tempVVN = vvn[count];
	//			count++;
	//			IIn[count] = IIn[count - 1] + StepsizeIn;

	//			//below is the method for updating these things
	//			Eigen::Vector2d temp5, temp6;
	//			temp5 << vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]),
	//				vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//			temp6 = B*temp5;
	//			vvu[count] = vvu[count - 1] + StepsizeIn*(c[0] + temp6[0]);
	//			vvw[count] = vvw[count - 1] + StepsizeIn*(c[1] + temp6[1]);
	//			IIu[count] = IIu[count - 1] + StepsizeIn*(-1.0*mu)*vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
	//			IIw[count] = IIw[count - 1] + StepsizeIn*(-1.0*mu)*vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);

	//			Eigen::MatrixXd temp7(1, 3), temp8(3, 1);
	//			double temp9;
	//			temp7 = nhat.transpose()*S*RTAN;
	//			temp8 << IIn[count],
	//				IIu[count],
	//				IIw[count];
	//			temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
	//			vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
	//			EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	//		}

	//		else if (sliding == 0)//update all the part in the sticking situation
	//		{
	//			count++;
	//			IIn[count] = IIn[count - 1] + StepsizeIn;
	//			IIu[count] = IIu[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(0);
	//			IIw[count] = IIw[count - 1] + StepsizeIn*((-1.0)*BNew.inverse()*c)(1);
	//			vvu[count] = 0;
	//			vvw[count] = 0;
	//			Eigen::MatrixXd temp7(1, 3), temp8(3, 1);
	//			double temp9;
	//			temp7 = nhat.transpose()*S*RTAN;
	//			temp8 << IIn[count],
	//				IIu[count],
	//				IIw[count];
	//			temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
	//			vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
	//			EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	//		}

	//	}
	//	/*  int iiter=0;
	//	cout<<"the values of IIn are: "<<endl;
	//	while(iiter<NumIter)
	//	{
	//	for(int j=1;j<=10;j++)
	//	{
	//	cout<<IIn[iiter]<<" ";
	//	iiter++;
	//	}
	//	cout<<endl;
	//	}*/
	//	if (countSlide == 0) countSlide = finish;
	//	if(DEBUG) std::cout << "at the end sliding1/sticking0?: " << sliding << std::endl;
	//	if(DEBUG) std::cout << "finish: " << finish << std::endl;
	//	if(DEBUG) std::cout << "countCom: " << countCom << std::endl;
	//	if(DEBUG) std::cout << "countSlide: " << countSlide << std::endl;

	//	//compute the post velocity and the post angular velocity:
	//	Eigen::Vector3d Iend;
	//	Iend << RTAN*Eigen::Vector3d(IIn[finish], IIu[finish], IIw[finish]);//because the coordinate system is always x-w, y-n, z-u

	//	Iend = -Iend;

	//	v1end = v1 + (1 / m1)*Iend;
	//	v2end = v2 - (1 / m2)*Iend;

	//	w1end = w1 + Q1.inverse()*r1.cross(R1.inverse()*Iend);
	//	w2end = w2 - Q2.inverse()*r2.cross(R2.inverse()*Iend);

	//	/*  cout<<"Iend: "<<endl<<Iend<<endl<<endl;
	//	cout<<"v1end: "<<endl<<v1end<<endl<<endl;
	//	cout<<"v2end: "<<endl<<v2end<<endl<<endl;
	//	cout<<"w1end: "<<endl<<w1end<<endl<<endl;
	//	cout<<"w2end: "<<endl<<w2end<<endl<<endl;*/

	//	ImpactOutput output = { MathUtils::eigenToGlmVector(v1end), MathUtils::eigenToGlmVector(v2end),
	//		MathUtils::eigenToGlmVector(w1end), MathUtils::eigenToGlmVector(w2end), MathUtils::eigenToGlmVector(Iend) };

	//	//getchar();

	//	return output;

	//}


};