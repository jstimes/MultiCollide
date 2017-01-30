#include "ShapeShader.h"


ShapeShader ShapeShader::getInstance() {
	static ShapeShader shapeShader;

	return shapeShader;
}