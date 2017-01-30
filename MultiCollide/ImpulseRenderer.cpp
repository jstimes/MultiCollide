#include "ImpulseRenderer.h"

ImpulseRenderer ImpulseRenderer::getInstance() {
	static ImpulseRenderer renderer;
	return renderer;
}