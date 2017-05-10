#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


namespace MultiCollide {

	class Quaternion {

	public:

		//float q0;
		//glm::vec3 axis;

		glm::quat quaternion;

		Quaternion() {}

		Quaternion(float angle, glm::vec3 vec) {
			//q0 = cosf(angle / 2.0f);
			//axis = glm::normalize(vec) * sinf(angle/2.0f);
			quaternion = glm::angleAxis(angle, vec);
		}

		Quaternion(glm::quat quat) :quaternion(quat) {}

		// Converts a 4x4 rotation matrix to quaternion
		//From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
		Quaternion(glm::mat4 &rotationMatrix) {

			quaternion = glm::quat(rotationMatrix);
			
			//float trace = rotationMatrix[0][0] + rotationMatrix[1][1] + rotationMatrix[2][2]; // I removed + 1.0f; see discussion with Ethan
			//if (trace > 0) {// I changed M_EPSILON to 0
			//	float s = 0.5f / sqrtf(trace + 1.0f);
			//	this->q0 = 0.25f / s;
			//	this->axis.x = (rotationMatrix[2][1] - rotationMatrix[1][2]) * s;
			//	this->axis.y = (rotationMatrix[0][2] - rotationMatrix[2][0]) * s;
			//	this->axis.z = (rotationMatrix[1][0] - rotationMatrix[0][1]) * s;
			//}
			//else {
			//	if (rotationMatrix[0][0] > rotationMatrix[1][1] && rotationMatrix[0][0] > rotationMatrix[2][2]) {
			//		float s = 2.0f * sqrtf(1.0f + rotationMatrix[0][0] - rotationMatrix[1][1] - rotationMatrix[2][2]);
			//		this->q0 = (rotationMatrix[2][1] - rotationMatrix[1][2]) / s;
			//		this->axis.x = 0.25f * s;
			//		this->axis.y = (rotationMatrix[0][1] + rotationMatrix[1][0]) / s;
			//		this->axis.z = (rotationMatrix[0][2] + rotationMatrix[2][0]) / s;
			//	}
			//	else if (rotationMatrix[1][1] > rotationMatrix[2][2]) {
			//		float s = 2.0f * sqrtf(1.0f + rotationMatrix[1][1] - rotationMatrix[0][0] - rotationMatrix[2][2]);
			//		this->q0 = (rotationMatrix[0][2] - rotationMatrix[2][0]) / s;
			//		this->axis.x = (rotationMatrix[0][1] + rotationMatrix[1][0]) / s;
			//		this->axis.y = 0.25f * s;
			//		this->axis.z = (rotationMatrix[1][2] + rotationMatrix[2][1]) / s;
			//	}
			//	else {
			//		float s = 2.0f * sqrtf(1.0f + rotationMatrix[2][2] - rotationMatrix[0][0] - rotationMatrix[1][1]);
			//		this->q0 = (rotationMatrix[1][0] - rotationMatrix[0][1]) / s;
			//		this->axis.x = (rotationMatrix[0][2] + rotationMatrix[2][0]) / s;
			//		this->axis.y = (rotationMatrix[1][2] + rotationMatrix[2][1]) / s;
			//		this->axis.z = 0.25f * s;
			//	}
			//}
		}

		Quaternion Conjugate() {
			/*Quaternion conj;
			conj.q0 = this->q0;
			conj.axis = -this->axis;*/
			glm::quat conj = glm::conjugate(this->quaternion);
			return conj;
		}

		Quaternion Multiply(Quaternion &right) {
			/*float scalar = this->q0 * right.q0 - glm::dot(this->axis, right.axis);
			glm::vec3 axis = this->q0 * right.axis + right.q0 * this->axis + glm::cross(this->axis, right.axis);

			return Quaternion(scalar, axis);*/
			return Quaternion(this->quaternion * right.quaternion);
		}

		float Norm() {
			//return sqrt(this->q0 * this->q0 + glm::dot(this->axis, this->axis));
			return this->quaternion.length();
		}

		void Normalize() {
			/*float norm = this->Norm();
			this->q0 = this->q0 / norm;
			this->axis = this->axis / norm;*/
			glm::normalize(this->quaternion);
		}

		float Angle() {
			//return acosf(this->q0 / this->Norm()) * 2.0f;
			return glm::angle(this->quaternion);
		}

		glm::vec3 Axis() {
			//return this->axis / this->Norm();
			return glm::axis(this->quaternion);
		}

		glm::vec3 Rotate(glm::vec3 vec) {
			//this->axis = glm::normalize(this->axis);

			/*glm::vec3 a, b, c;

			a = (this->q0 * this->q0 - glm::dot(this->axis, this->axis)) * vec;
			b = 2.0f * glm::dot(this->axis, vec) * this->axis;
			c = 2.0f * this->q0 * glm::cross(this->axis, vec);

			return a + b + c;*/
			return glm::vec3(glm::rotate(this->quaternion, glm::vec4(vec.x, vec.y, vec.z, 1.0f)));

			/*Quaternion result = this->Multiply(Quaternion(0.0f, vec).Multiply(this->Conjugate()));
			return result.Axis();*/
		}

	};

}