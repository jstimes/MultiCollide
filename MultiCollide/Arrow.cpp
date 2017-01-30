#include "Arrow.h"

const float Arrow::defaultArrowHeadScaling = .1f;

//ArrowObject Arrow::arrowObject = ArrowObject();

///Draws an arrow based on the parameters:
/// arrowLength - distance from arrow start to arrow head
/// headScaling - scales the size of the arrow head by this fraction (1.0 is default, .5 is half that size, 2.0 is double default)
/// ptToRotateTo - the point to which the arrow should point in its local space
/// translation - gives the position of where the arrow should start at
void Arrow::Draw(Shader &shader, float arrowLength, float headScaling, glm::vec3 ptToRotateArrowTo, glm::vec3 translation) {
	static ArrowObject arrowObject;

	if (arrowLength <= 0.0f) {
		return; // don't draw arrows of length 0 (or invalid lengths)
	}

	glm::mat4 lineScaling = glm::scale(MathUtils::I4, glm::vec3(arrowLength, 0.0f, 0.0f));
	glm::mat4 transAndRotMat = glm::translate(MathUtils::I4, translation) * MathUtils::rotationFromAtoB(glm::vec3(1.0f, 0.0f, 0.0f), ptToRotateArrowTo);
	glm::mat4 lineModel = transAndRotMat * lineScaling;

	glm::mat4 headModel = transAndRotMat * glm::translate(MathUtils::I4, glm::vec3(arrowLength, 0.0f, 0.0f)) * glm::scale(MathUtils::I4, glm::vec3(headScaling * defaultArrowHeadScaling));
	//glm::mat4 headModel = transAndRotMat * glm::translate(MathUtils::I4, glm::vec3(arrowLength, 0.0f, 0.0f)) * glm::scale(MathUtils::I4, glm::vec3(defaultArrowHeadScaling));

	arrowObject.Draw(shader, headModel, lineModel);
}