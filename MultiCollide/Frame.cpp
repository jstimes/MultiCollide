#include "Frame.h"
#include "ShapeShader.h"

const glm::vec3 Frame::xColor = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Frame::yColor = glm::vec3(0.2f, .5f, 0.2f);
const glm::vec3 Frame::zColor = glm::vec3(0.0f, 0.0f, 1.0f);

//Draws an x-y-z frame scaled according to scaling (1 = no scaling), rotated according to rotation, 
// and translated to the point at translation
void Frame::Draw(glm::vec3 &translation, const glm::mat4 &rotation, float scaling, bool use2D) {

	Shader shader = ShapeShader::getInstance().shader;
	shader.Use();

	GLuint objectColorLoc = ShapeShader::getInstance().objectColorLoc;

	glUniform4f(objectColorLoc, xColor[0], xColor[1], xColor[2], 1.0f);
	Arrow::Draw(shader, scaling, 1.0f, glm::vec3(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)), translation);

	glUniform4f(objectColorLoc, yColor[0], yColor[1], yColor[2], 1.0f);
	Arrow::Draw(shader, scaling, 1.0f, glm::vec3(rotation * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)), translation);

	if (!use2D) {
		glUniform4f(objectColorLoc, zColor[0], zColor[1], zColor[2], 1.0f);
		Arrow::Draw(shader, scaling, 1.0f, glm::vec3(rotation * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)), translation);
	}
}