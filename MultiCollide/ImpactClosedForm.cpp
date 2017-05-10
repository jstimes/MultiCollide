#include "ImpactClosedForm.h"

const std::complex<double> ImpactClosedForm::EPSILON = std::numeric_limits<double>::epsilon();
const std::complex<double> ImpactClosedForm::pi = 3.1415926536;
const std::complex<double> ImpactClosedForm::i = std::complex<double>(0.0, 1.0);
const double ImpactClosedForm::DEFAULT_ACCURACY = .000001;

//int ImpactClosedForm::NumIter = 10000; //maximum number of iterations
const double ImpactClosedForm::constantStep = 0.0001; //numerical intergration step size
const double ImpactClosedForm::constSlidingisZero = 8.0*0.001; //the judging criteria for sliding velocity equals zero
const double ImpactClosedForm::constStraightSlidingisZero = 8.0*0.001;


ImpactClosedFormOutput ImpactClosedForm::impact() {

	using namespace std;

	int NumIter = 10000;

	double initialVperpu, initialVperpw, initialVnormaln;
	Eigen::Matrix3d Rot, S, A, Identity3;
	Eigen::Vector3d initialVperp, initialVnormal;
	Eigen::MatrixXd uwJuzhen(3, 2);
	Eigen::Matrix2d B, BNew, Binv;
	Eigen::Vector2d c;
	S = R1*P1*Q1.inverse()*P1*R1.inverse() + R2*P2*Q2.inverse()*P2*R2.inverse();
	uwJuzhen << uhat[0], ohat[0],
		uhat[1], ohat[1],
		uhat[2], ohat[2];
	Identity3 << 1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0;
	A = (Identity3 - nhat*nhat.transpose())*((1.0 / m1 + 1.0 / m2)*Identity3 - S);
	initialVperp = ((Identity3 - nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (Identity3 - nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	initialVperpu = initialVperp.transpose()*uhat;
	initialVperpw = initialVperp.transpose()*ohat;
	initialVnormal = ((nhat*nhat.transpose())*(v1 + R1*(w1.cross(r1))) - (nhat*nhat.transpose())*(v2 + R2*(w2.cross(r2))));
	initialVnormaln = initialVnormal.transpose()*nhat;
	B = (-mu)*uwJuzhen.transpose()*A*uwJuzhen;
	double B11, B12, B21, B22;
	B11 = B(0, 0); B12 = B(0, 1); B21 = B(1, 0); B22 = B(1, 1);//the elements of B matrix
	BNew = uwJuzhen.transpose()*A*uwJuzhen;//This is the new definition of B in the note that does not have mu in it
	Binv = BNew.inverse();
	c = uwJuzhen.transpose()*A*nhat;
	double c1, c2;
	c1 = c(0); c2 = c(1);

	Eigen::Vector2d shat;
	Eigen::Vector3d sigma;
	shat = centrifugal(mu, c1, c2, B11, B12, B21, B22);
	//std::cout<<"centrifugal direction: "<<std::endl<<shat<<std::endl<<std::endl;

	if ((Binv*c).norm()<mu)
	{
		sigma = nhat - uwJuzhen*Binv*c;
	}
	else
	{
		sigma = nhat - mu*uwJuzhen*shat;
	}
	// std::cout<<"sigma: "<<std::endl<<sigma<<std::endl<<std::endl;


	//start initialization part for the numerical integration using the Euler's method.
	double StepsizeIn = constantStep;


	std::vector<double> vvu;
	std::vector<double> vvn;
	std::vector<double> vvw;
	std::vector<double> IIu;
	std::vector<double> IIw;
	std::vector<double> IIn;
	std::vector<double> EE;
	std::vector<double> curvatureOfV;
	std::vector<double> nuPrimeNorm;
	std::vector<double> stepSizeInCount;

	vvu.resize(NumIter);
	vvn.resize(NumIter);
	vvw.resize(NumIter);
	IIu.resize(NumIter);
	IIw.resize(NumIter);
	IIn.resize(NumIter);
	EE.resize(NumIter);
	curvatureOfV.resize(NumIter);
	nuPrimeNorm.resize(NumIter);
	stepSizeInCount.resize(NumIter);

	double lambdaStep = 0.5; double h1 = 0.01; double h2 = 0.01;
	double top = 0.0; double bottom = 1.0; double valueofYPrime = 0.0; double x = 0.0; double y = 0.0;
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

	//double top=0.0; double bottom=1.0; double x=0.0; double y=0.0; double valueofYPrime=0.0;//curvature related calculation

	int countCom = 0; int countSlide = 0;//either 0 or 1, different status,comp means compression
	vvu[0] = initialVperpu; vvw[0] = initialVperpw;
	vvn[0] = initialVnormaln; EE[0] = 0.0;
	double tempVVN = vvn[0];


	double Eprime = 0.0; double Izc = 0.0; double Ec = 0.0; double Izr = 0.0; double Izs = 0.0; double Es = 0.0;
	int situation = 0;
	Eigen::Vector3d Iend1, Iend21, Iend22, Iend31, Iend32a, Iend32b, Iend33, Iend34, Is;
	double msigma = 1.0 / (1.0 / m1 + 1.0 / m2 - nhat.transpose()*S*sigma);
	Iend1 << 0, 0, 0;
	Iend21 << 0, 0, 0;
	Iend22 << 0, 0, 0;
	Iend31 << 0, 0, 0;
	Iend32a << 0, 0, 0;
	Iend32b << 0, 0, 0;
	Iend33 << 0, 0, 0;
	Iend34 << 0, 0, 0;
	Is << 0, 0, 0;


	while (count<NumIter)
	{
		Eigen::Vector3d Icurrent;

		Icurrent << IIn[count], IIu[count], IIw[count];

		if (EE[count]<-0.000000000000001)//checking for finish condition
		{
			finish = count;

			Iend1 = RTAN*Icurrent;
			situation = 1;
			std::cout << "situation1: " << situation << std::endl << std::endl;
			//return;

			ImpactClosedFormOutput output = { MathUtils::eigenToGlmVector(v1end), MathUtils::eigenToGlmVector(v2end),
				MathUtils::eigenToGlmVector(w1end), MathUtils::eigenToGlmVector(w2end), MathUtils::eigenToGlmVector(Iend) };

			return output;
		}

		if (sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count])<constSlidingisZero)
		{
			situation = 2;
			//std::cout<<"situation2: "<<situation<<std::endl<<std::endl;
			// std::cout<<"vvu[count]: "<<vvu[count]<<std::endl<<std::endl;
			//std::cout<<"vvw[count]: "<<vvw[count]<<std::endl<<std::endl;
			break;
		}
		Eigen::Vector2d vSlideCurrent;
		vSlideCurrent << vvu[count], vvw[count];

		Eigen::Vector2d temperoryVec1 = ((-mu*BNew*vSlideCurrent / vSlideCurrent.norm() + c) / ((-mu*BNew*vSlideCurrent / vSlideCurrent.norm() + c).norm()));
		double  temperoryConst1 = temperoryVec1(1);
		double  temperoryConst2 = temperoryVec1(0);


		if (sqrt(vvu[count] * vvu[count] + vvw[count] * vvw[count]) >= constSlidingisZero &&
			(abs(vvu[count] / vSlideCurrent.norm()*temperoryConst1 - vvw[count] / vSlideCurrent.norm()*temperoryConst2)< constStraightSlidingisZero))
		{
			situation = 3;
			std::cout << "situation3: " << situation << std::endl << std::endl;
			break;
		}

		//Now coompute the curvature related thing to determine the stepsize
		x = vvu[count];
		y = vvw[count];
		if (abs((B11*x + B12*y + c1*sqrt(x*x + y*y))) >DEFAULT_ACCURACY)
		{
			valueofYPrime = (B21*x + B22*y + c2*sqrt(x*x + y*y)) / (B11*x + B12*y + c1*sqrt(x*x + y*y));
		}
		else
		{
			bottom = 0.0;
		}
		if (abs(sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)))  >DEFAULT_ACCURACY)
		{
			top = (((B21*c1 - B11*c2)*y - B22*(c1*x + B11*sqrt(x*x + y*y)) + B12*(c2*x + B21*sqrt(x*x + y*y)))*(y - x*valueofYPrime)) / (sqrt(x*x + y*y)*(B11*x + B12*y + c1*sqrt(x*x + y*y))*(B11*x + B12*y + c1*sqrt(x*x + y*y)));
		}
		else
		{
			bottom = 0;
		}
		bottom = pow(1.0 + valueofYPrime*valueofYPrime, 1.5);

		if (abs(bottom)>DEFAULT_ACCURACY)
		{
			curvatureOfV[count] = abs(top / bottom);
		}
		else
		{
			curvatureOfV[count] = 0.0;
		}
		if (x*x + y*y>constSlidingisZero)
		{
			nuPrimeNorm[count] = sqrt((B11*x / sqrt(x*x + y*y) + B12*y / sqrt(x*x + y*y) + c1)*(B11*x / sqrt(x*x + y*y) + B12*y / sqrt(x*x + y*y) + c1)
				+ (B21*x / sqrt(x*x + y*y) + B22*y / sqrt(x*x + y*y) + c2)*(B21*x / sqrt(x*x + y*y) + B22*y / sqrt(x*x + y*y) + c2));
		}
		else
		{
			nuPrimeNorm[count] = 0;
		}
		if (nuPrimeNorm[count] == 0 || curvatureOfV[count] == 0)
		{
			StepsizeIn = constantStep;
		}
		else
		{
			StepsizeIn = lambdaStep*h1 / nuPrimeNorm[count] + (1 - lambdaStep)*h2 / (curvatureOfV[count] * nuPrimeNorm[count]);
		}
		stepSizeInCount[count] = StepsizeIn;

		if (compress == 1 && tempVVN*vvn[count]<0.000001)//checking for finishing compression phase, the sign of vn(which is normal velocity) changes
		{
			compress = 0;
			countCom = count;
			Izc = IIn[count];
			EE[count] = e*e*EE[count];
		}
		tempVVN = vvn[count];
		count++;

		if (count >= NumIter) {
			NumIter *= 2;
			vvu.resize(NumIter);
			vvn.resize(NumIter);
			vvw.resize(NumIter);
			IIu.resize(NumIter);
			IIw.resize(NumIter);
			IIn.resize(NumIter);
			EE.resize(NumIter);
			curvatureOfV.resize(NumIter);
			nuPrimeNorm.resize(NumIter);
			stepSizeInCount.resize(NumIter);

			std::cout << "Had to double array size in Closed Form Impact" << std::endl;
		}

		IIn[count] = IIn[count - 1] + StepsizeIn;
		//below is the method for updating these things
		Eigen::Vector2d temp5, temp6;
		temp5 << vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]),
			vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
		temp6 = B*temp5;
		vvu[count] = vvu[count - 1] + StepsizeIn*(c[0] + temp6[0]);
		vvw[count] = vvw[count - 1] + StepsizeIn*(c[1] + temp6[1]);
		IIu[count] = IIu[count - 1] + StepsizeIn*(-1.0*mu)*vvu[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);
		IIw[count] = IIw[count - 1] + StepsizeIn*(-1.0*mu)*vvw[count - 1] / sqrt(vvu[count - 1] * vvu[count - 1] + vvw[count - 1] * vvw[count - 1]);

		Eigen::MatrixXd temp7(1, 3), temp8(3, 1);
		double temp9;
		temp7 = nhat.transpose()*S*RTAN;
		temp8 << IIn[count],
			IIu[count],
			IIw[count];
		temp9 = (temp7*temp8)(0, 0);//////////////////////////////////////////////might be problem for matrix product??
		vvn[count] = initialVnormaln + (1.0 / m1 + 1.0 / m2)*IIn[count] - temp9;
		EE[count] = EE[count - 1] + StepsizeIn*(-vvn[count]);
	}

	/*for (int i = 0; i < count; i++) {
		std::cout << IIn[i] << ", " << IIu[i] << ", " << IIw[i] << std::endl;
	}*/

	if (situation == 2)
	{
		Eigen::Vector2d temporaryVect3;
		temporaryVect3 << initialVperpu, initialVperpw;
		Izs = IIn[count];
		Is = (IIn[count] * nhat - uwJuzhen*Binv*(temporaryVect3 + IIn[count] * c));
		Es = EE[count];
		Eprime = -initialVnormaln - (1 / m1 + 1 / m2)* Izs + (nhat.transpose()*S*Is);
		double a2 = -1 / (2 * msigma);
		double a1 = -initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma));

		if (Eprime>0)
		{
			Izc = msigma* (-initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma)));
			Ec = Es + Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izc) - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
			double a0 = e*e*Ec - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izc);
			double root1 = (-a1 - sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);
			double root2 = (-a1 + sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);

			if (root1 > Izc && root2 > Izc)
			{
				Izr = root1<root2 ? root1 : root2;
			}
			else if (root1 > Izc)
			{
				Izr = root1;
			}
			else
			{
				Izr = root2;
			}
			Iend21 = (Is + (Izr - Izs)* sigma);
			/*	  std::cout<<"compress: "<<compress<<std::endl<<std::endl;
			std::cout<<"sigma: "<<sigma<<std::endl<<std::endl;
			std::cout<<"msigma: "<<msigma<<std::endl<<std::endl;
			std::cout<<"Iend21: "<<Iend21<<std::endl<<std::endl;
			std::cout<<"Ec: "<<Ec<<std::endl<<std::endl;
			std::cout<<"Izc: "<<Izc<<std::endl<<std::endl;
			std::cout<<"Izs: "<<Izs<<std::endl<<std::endl;
			std::cout<<"Phi1Izc: "<< Phi1( initialVnormaln,  nhat,  Is,  Izs,  S,  sigma,  msigma, Izc)<<std::endl<<std::endl;
			std::cout<<"a0: "<<a0<<std::endl<<std::endl;
			std::cout<<"a1: "<<a1<<std::endl<<std::endl;
			std::cout<<"a2: "<<a2<<std::endl<<std::endl;
			std::cout<<"root1: "<<root1<<std::endl<<std::endl;
			std::cout<<"root2: "<<root2<<std::endl<<std::endl;
			for(int k=0;k<50;k++)
			{
			std::cout<<stepSizeInCount[k]<<std::endl;
			}
			std::cout<<std::endl<<std::endl;*/

		}
		else
		{
			double b0 = Es - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
			double root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
			double root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
			if (root3 > Izs && root4 > Izs)
			{
				Izr = root3<root4 ? root3 : root4;
			}
			else if (root3 > Izs)
			{
				Izr = root3;
			}
			else
			{
				Izr = root4;
			}
			Iend22 = (Is + (Izr - Izs)*sigma);
		}

	}
	//////////////////////////////////situation3
	if (situation == 3)
	{
		double Izl, El, q1, q2, q3, q4, lambda, DeltaIz, mdelta, Chatac, Chatae;
		Eigen::Vector2d gammal, temp6;
		Eigen::Vector3d temp5, Il, delta;
		Izl = IIn[count];
		temp5 << IIn[count], IIu[count], IIw[count];
		Il = RTAN*temp5;
		El = EE[count];
		gammal << vvu[count], vvw[count];
		q3 = vvu[count] / gammal.norm(); q4 = vvw[count] / gammal.norm();
		temp6 << q3, q4;
		q1 = (-mu*BNew*temp6 / temp6.norm() + c)(0);
		q2 = (-mu*BNew*temp6 / temp6.norm() + c)(1);
		if (abs(q1)<1e-6  &&  abs(q2) >DEFAULT_ACCURACY && abs(q3) < 1e-6 && abs(q4) >DEFAULT_ACCURACY)
		{
			lambda = q2 / q4;
		}
		else if ((abs(q1) >DEFAULT_ACCURACY && abs(q2) < 1e-6 && abs(q3) >DEFAULT_ACCURACY && abs(q4) < 1e-6))
		{
			lambda = q1 / q3;
		}
		else if ((abs(q1) >DEFAULT_ACCURACY && abs(q2) >DEFAULT_ACCURACY && abs(q3) >DEFAULT_ACCURACY && abs(q4) >DEFAULT_ACCURACY))
		{
			lambda = q1 / q3;
		}
		else if ((abs(q1) < 1e-6 && abs(q2) < 1e-6 && abs(q3) < 1e-6 && abs(q4) < 1e-6))
		{
			lambda = 100000000.0;
		}
		else
		{
			lambda = 0.0000000001;
		}
		///
		DeltaIz = lambda>0 ? 100000000 : -gammal.norm() / lambda;
		delta = nhat - mu*uwJuzhen*(gammal / gammal.norm());
		Izs = Izl + DeltaIz;
		mdelta = 1 / (1 / m1 + 1 / m2 - (nhat.transpose()*S*delta));
		Chatac = mdelta*(-initialVnormaln + (nhat.transpose()*S*(Il - Izl*delta)));

		if (compress == 1)
		{
			double a0, a1, a2, root1, root2, d0, d1, d2, root5, root6, b0, root3, root4;
			if (Izs < Chatac)
			{
				Es = Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izs); Is = Il + (Izs - Izl)*delta;
				a2 = -1 / (2 * msigma);
				a1 = -initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
				Izc = msigma* (-initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma)));
				Ec = Es + Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izc) - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
				a0 = e*e*Ec - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izc);
				root1 = (-a1 - sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2); root2 = (-a1 + sqrt(a1*a1 - 4 * a2*a0)) / (2 * a2);
				if (root1 > Izc && root2 > Izc)
				{
					Izr = root1<root2 ? root1 : root2;
				}
				else if (root1 > Izc)
				{
					Izr = root1;
				}
				else
				{
					Izr = root2;
				}

				Iend31 = (Is + (Izr - Izs)* sigma);
				std::cout << "Iend31: " << Iend31 << std::endl << std::endl;
			}
			else
			{
				Izc = Chatac; Ec = Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
				d2 = -1 / (2 * mdelta); d1 = -initialVnormaln + (nhat.transpose()*S*(Il - Izl*delta));
				d0 = e*e*Ec - Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
				root5 = (-d1 - sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);
				root6 = (-d1 + sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);

				if (root5 > Chatac && root6 > Chatac)
				{
					Chatae = root5<root6 ? root5 : root6;

				}
				else if (root5 > Chatac)
				{
					Chatae = root5;
				}
				else
				{
					Chatae = root6;
				}

				if (Izs<Chatae)
				{
					Es = e*e *Ec + Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izs) - Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izc);
					Is = Il + (Izs - Izl)*delta;
					a2 = -1 / (2 * msigma);
					a1 = -initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
					b0 = Es - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
					root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
					root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
					if (root3 > Izs && root4 > Izs)
					{
						Izr = root3<root4 ? root3 : root4;
					}
					else if (root3 > Izs)
					{
						Izr = root3;
					}
					else
					{
						Izr = root4;
					}
					Iend32a = (Is + (Izr - Izs)*sigma);
				}
				else
				{
					Izr = Chatae;
					Iend32b = Il + (Izr - Izl)*delta;
					std::cout << "Iend32b: " << Iend32b << std::endl << std::endl;
				}

			}

		}
		if (compress == 0)
		{
			double a0, a1, a2, root1, root2, d0, d1, d2, root5, root6, b0, root3, root4;
			d2 = -1 / (2 * mdelta);
			d1 = -initialVnormaln + (nhat.transpose()*S*(Il - Izl* delta));
			d0 = El - Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izl);
			root5 = (-d1 - sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2); root6 = (-d1 + sqrt(d1*d1 - 4 * d2*d0)) / (2 * d2);
			if (root5 > Chatac && root6 > Chatac)
			{
				Chatae = root5<root6 ? root5 : root6;
			}
			else if (root5 > Chatac)
			{
				Chatae = root5;
			}
			else
			{
				Chatae = root6;
			}

			if (Izs < Chatae)
			{
				Es = El + Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izs) - Phi2(initialVnormaln, nhat, Il, Izl, S, delta, mdelta, Izl);
				Is = Il + (Izs - Izl)*delta;
				a2 = -1 / (2 * msigma);
				a1 = -initialVnormaln + (nhat.transpose()*S*(Is - Izs* sigma));
				b0 = Es - Phi1(initialVnormaln, nhat, Is, Izs, S, sigma, msigma, Izs);
				root3 = (-a1 - sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
				root4 = (-a1 + sqrt(a1*a1 - 4 * a2*b0)) / (2 * a2);
				if (root3 > Izs && root4 > Izs)
				{
					Izr = root3<root4 ? root3 : root4;
				}
				else if (root3 > Izs)
				{
					Izr = root3;
				}
				else
				{
					Izr = root4;
				}
				Iend33 = (Is + (Izr - Izs)* sigma);
			}
			else
			{
				Izr = Chatae;
				Iend34 = Il + (Izr - Izl)* delta;
			}


		}



	}


	if (countSlide == 0) countSlide = finish;
	std::cout << "at the end sliding1/sticking0?: " << sliding << std::endl;
	std::cout << "finish: " << finish << std::endl;
	std::cout << "countCom: " << countCom << std::endl;
	std::cout << "countSlide: " << countSlide << std::endl << std::endl;

	//compute the post velocity and the post angular velocity:
	if (Iend1.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.2Case1,Iend1" << std::endl << std::endl;
		Iend = Iend1;
	}
	else if (Iend21.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.2Case21,Iend21" << std::endl << std::endl;
		Iend = Iend21;
	}
	else if (Iend22.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.2Case22,Iend22" << std::endl << std::endl;
		Iend = Iend22;
	}
	else if (Iend31.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.3Case1,Iend31" << std::endl << std::endl;
		Iend = Iend31;
	}
	else if (Iend32a.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.3Case2a,Iend32a" << std::endl << std::endl;
		Iend = Iend32a;
	}
	else if (Iend32b.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.3Case2b,Iend32b" << std::endl << std::endl;
		Iend = Iend32b;
	}
	else if (Iend33.norm()>DEFAULT_ACCURACY)
	{
		std::cout << "Sc6.3Case3,Iend33" << std::endl << std::endl;
		Iend = Iend33;
	}
	else
	{
		std::cout << "Sc6.3Case4,Iend34" << std::endl << std::endl;
		Iend = Iend34;
	}
	// Vector3d IendTmp; It's already been in the world frame.
	//IendTmp=Iend;
	//Iend=RTAN*IendTmp;//because the coordinate system is always x-w, y-n, z-u

	v1end = v1 + (1 / m1)*Iend;
	v2end = v2 - (1 / m2)*Iend;

	w1end = w1 + Q1.inverse()*r1.cross(R1.inverse()*Iend);
	w2end = w2 - Q2.inverse()*r2.cross(R2.inverse()*Iend);

	ImpactClosedFormOutput output = { MathUtils::eigenToGlmVector(v1end), MathUtils::eigenToGlmVector(v2end),
		MathUtils::eigenToGlmVector(w1end), MathUtils::eigenToGlmVector(w2end), MathUtils::eigenToGlmVector(Iend) };


	return output;

}