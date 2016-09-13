
class Impact {

	int iterationSteps;
	float stepSize; 
	// ... other parameters

	struct Output {
		vector3 afterVelocity;

	};



	Output computeImpact(...) {

		Output output;
		output.afterVelocity = ...;


		return output;
	}


};