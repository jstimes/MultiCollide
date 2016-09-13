#include "Impact.h"

Eigen::Matrix3d  Impact::getPmatrix(Eigen::Vector3d r)
{
	Eigen::Matrix3d P;
	P << 0, -r[2], r[1],
		r[2], 0, -r[0],
		-r[1], r[0], 0;
	return P;
}