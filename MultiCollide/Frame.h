#pragma once

#include <vector>

#include "ShapeUtils.h"
#include "Arrow.h"


//Represents a right-handed set of axes that can be oriented to render as an object's principal frame
class Frame {

	static const glm::vec3 xColor;
	static const glm::vec3 yColor;
	static const glm::vec3 zColor;

public:

	//Draws an x-y-z frame scaled according to scaling (1 = no scaling), rotated according to rotation, 
	// and translated to the point at translation
	static void Draw(Shader &shader, glm::vec3 &translation, glm::mat4 &rotation, float scaling);

};