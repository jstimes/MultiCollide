#include "ColorShapeShader.h"


ColorShapeShader ColorShapeShader::getInstance() {
	static ColorShapeShader shapeShader;

	return shapeShader;
}