#include "shader.h"
#ifdef GLES
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <Shader.h>

shader::shader(char* vertShader, char* fragShader){
	_shader = new Shader(vertShader, fragShader);
}

shader::~shader(){
	delete _shader;
}

GLuint shader::getShader(){
	return _shader->getShader();
}
void shader::use(){
	_shader->use();
}
