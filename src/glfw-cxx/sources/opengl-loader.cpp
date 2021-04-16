#include "../opengl-loader.h"

// TODO : different headers
#include <GL/glew.h>

namespace glfw
{
	bool load_opengl_functions()
	{
		return glewInit() == GLEW_OK;
	}
}