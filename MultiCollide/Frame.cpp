#include "Frame.h"

const glm::vec3 Frame::xColor = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Frame::yColor = glm::vec3(0.2f, .5f, 0.2f);
const glm::vec3 Frame::zColor = glm::vec3(0.0f, 0.0f, 1.0f);

//Draws an x-y-z frame scaled according to scaling (1 = no scaling), rotated according to rotation, 
// and translated to the point at translation
void Frame::Draw(Shader &shader, glm::vec3 &translation, glm::mat4 &rotation, float scaling) {

	shader.Use();
	GLuint objectColorLoc = shader.getUniform("objectColor");

	glUniform3f(objectColorLoc, xColor[0], xColor[1], xColor[2]);
	Arrow::Draw(shader, scaling, .5f, glm::vec3(1.0f, 0.0f, 0.0f), translation);

	glUniform3f(objectColorLoc, yColor[0], yColor[1], yColor[2]);
	Arrow::Draw(shader, scaling, .5f, glm::vec3(0.0f, 1.0f, 0.0f), translation);

	glUniform3f(objectColorLoc, zColor[0], zColor[1], zColor[2]);
	Arrow::Draw(shader, scaling, .5f, glm::vec3(0.0f, 0.0f, 1.0f), translation);
}